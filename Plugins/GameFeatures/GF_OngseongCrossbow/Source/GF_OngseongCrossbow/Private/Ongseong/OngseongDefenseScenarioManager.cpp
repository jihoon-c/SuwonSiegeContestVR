#include "Ongseong/OngseongDefenseScenarioManager.h"

#include "GF_OngseongCrossbow.h"

#include "Components/SceneComponent.h"
#include "Core/Experience/ExperienceSubsystem.h"
#include "Core/VR/VRPlayerPawn.h"
#include "Gameplay/Combat/HealthComponent.h"
#include "Gameplay/Pooling/ActorPool.h"
#include "Gameplay/UI/VRHUDComponent.h"
#include "Gameplay/UI/VRHUDTypes.h"
#include "Ongseong/OngseongEnemyWaveManager.h"
#include "Ongseong/OngseongGateActor.h"
#include "Ongseong/OngseongNarrationComponent.h"
#include "Ongseong/OngseongRamActor.h"
#include "Ongseong/OngseongSpawnPointActor.h"
#include "EngineUtils.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "OngseongDefense"

AOngseongDefenseScenarioManager::AOngseongDefenseScenarioManager()
{
	PrimaryActorTick.bCanEverTick = false;
	// Placeable in the editor: the retreat fallback and ram staging read this actor's transform.
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Narration = CreateDefaultSubobject<UOngseongNarrationComponent>(TEXT("OngseongNarration"));
	RamClass = AOngseongRamActor::StaticClass();
}

void AOngseongDefenseScenarioManager::BeginPlay()
{
	Super::BeginPlay();
	if (!IsValid(RamSpawnPoint))
	{
		for (TActorIterator<AOngseongSpawnPointActor> It(GetWorld()); It; ++It)
		{
			if (It->MatchesRole(EOngseongSpawnPointRole::Ram))
			{
				RamSpawnPoint = *It;
				break;
			}
		}
	}
	if (GateActor) GateActor->OnGateDestroyed.AddUniqueDynamic(this, &AOngseongDefenseScenarioManager::HandleGateDestroyed);
	if (WaveManager)
	{
		WaveManager->OnAllEnemiesRetreated.AddUniqueDynamic(this, &AOngseongDefenseScenarioManager::HandleAllEnemiesRetreated);
	}
	Narration->GateActor = GateActor;
	Narration->WaveManager = WaveManager;
	Narration->InitializeNarrationBindings();
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (APawn* Pawn = PC->GetPawn()) VRHUD = Pawn->FindComponentByClass<UVRHUDComponent>();
	}
	ApplyPlayerLocomotionPolicy();
	if (bAutoStart) StartDefense();
}

void AOngseongDefenseScenarioManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
	if (ActiveRam) ActiveRam->StopRam();
	if (WaveManager)
	{
		WaveManager->OnAllEnemiesRetreated.RemoveDynamic(this, &AOngseongDefenseScenarioManager::HandleAllEnemiesRetreated);
	}
	if (GateActor) GateActor->OnGateDestroyed.RemoveDynamic(this, &AOngseongDefenseScenarioManager::HandleGateDestroyed);
	Super::EndPlay(EndPlayReason);
}

void AOngseongDefenseScenarioManager::ApplyPlayerLocomotionPolicy()
{
	if (!bLockPlayerToBattlement || !GetWorld()) return;
	const APlayerController* PC = GetWorld()->GetFirstPlayerController();
	// The non-VR combat test pawn is not a VR pawn, so this simply does nothing there.
	if (AVRPlayerPawn* VRPawn = PC ? Cast<AVRPlayerPawn>(PC->GetPawn()) : nullptr)
	{
		VRPawn->SetLocomotionEnabled(false, false);
		UE_LOG(LogOngseong, Display, TEXT("Player locomotion locked to the battlement post."));
	}
}

bool AOngseongDefenseScenarioManager::StartDefense()
{
	if (DefenseState == EOngseongDefenseState::Defending || !IsValid(GateActor) || !IsValid(WaveManager)) return false;
	RemainingDefenseTime = bUseDefenseTimeLimit ? FMath::Max(1.0f, DefenseDuration) : 0.0f;
	bRamDestroyed = false;
	if (!SpawnAndActivateRam()) return false;
	WaveManager->ResetWave();
	SetDefenseState(EOngseongDefenseState::Defending);
	WaveManager->SetObjectiveTarget(GateActor);
	WaveManager->StartSpawning();
	if (bUseDefenseTimeLimit)
	{
		GetWorldTimerManager().SetTimer(DefenseTimerHandle, this, &AOngseongDefenseScenarioManager::TickDefenseTimer, 1.0f, true);
	}
	Narration->ReportScenarioEvent(TEXT("ScenarioStarted"), this);
	UpdateHUDTime();
	UE_LOG(LogOngseong, Display, TEXT("Defense started. Ram=%s, enemy slots=%d, time limit=%s"),
		*GetNameSafe(ActiveRam),
		WaveManager->GetMaxConcurrentEnemies(),
		bUseDefenseTimeLimit ? *FString::Printf(TEXT("%.0fs"), DefenseDuration) : TEXT("off"));
	return true;
}

int32 AOngseongDefenseScenarioManager::GetTotalDefeatedEnemies() const
{
	return IsValid(WaveManager) ? WaveManager->GetTotalDefeatedEnemies() : 0;
}

void AOngseongDefenseScenarioManager::RetryDefense()
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
	ReleaseActiveRam();
	if (WaveManager) WaveManager->ResetWave();
	if (GateActor) GateActor->ResetGate();
	bRamDestroyed = false;
	SetDefenseState(EOngseongDefenseState::Idle);
	StartDefense();
}

bool AOngseongDefenseScenarioManager::SpawnAndActivateRam()
{
	if (!IsValid(GateActor) || !IsValid(WaveManager) || ActiveRam) return false;
	const FTransform SpawnTransform = IsValid(RamSpawnPoint) ? RamSpawnPoint->GetActorTransform() : WaveManager->GetActorTransform();

	if (IsValid(RamPool))
	{
		ActiveRam = Cast<AOngseongRamActor>(RamPool->AcquireActor(SpawnTransform));
	}
	else if (RamClass)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		ActiveRam = GetWorld()->SpawnActor<AOngseongRamActor>(RamClass, SpawnTransform, Params);
	}

	if (!ActiveRam)
	{
		UE_LOG(LogOngseong, Warning, TEXT("Could not send in the ram. Check the ram pool size and RamClass."));
		return false;
	}
	if (UHealthComponent* RamHealth = ActiveRam->FindComponentByClass<UHealthComponent>())
	{
		RamHealth->OnDeath.AddUniqueDynamic(this, &AOngseongDefenseScenarioManager::HandleRamDefeated);
	}
	ActiveRam->ActivateRam(GateActor);
	// Archers that cannot get an attack slot on a cannon rally on their own siege engine.
	WaveManager->SetArcherEscortTarget(ActiveRam);
	return true;
}

void AOngseongDefenseScenarioManager::ReleaseActiveRam()
{
	if (!ActiveRam) return;
	AOngseongRamActor* RamToRelease = ActiveRam;
	ActiveRam = nullptr;
	if (IsValid(WaveManager)) WaveManager->SetArcherEscortTarget(nullptr);
	RamToRelease->StopRam();
	if (UHealthComponent* RamHealth = RamToRelease->FindComponentByClass<UHealthComponent>())
	{
		RamHealth->OnDeath.RemoveDynamic(this, &AOngseongDefenseScenarioManager::HandleRamDefeated);
	}
	if (IsValid(RamPool) && RamPool->ReleaseActor(RamToRelease))
	{
		return;
	}
	RamToRelease->Destroy();
}

void AOngseongDefenseScenarioManager::TickDefenseTimer()
{
	if (DefenseState != EOngseongDefenseState::Defending || !bUseDefenseTimeLimit) return;
	RemainingDefenseTime = FMath::Max(0.0f, RemainingDefenseTime - 1.0f);
	UpdateHUDTime();
	if (RemainingDefenseTime <= 0.0f)
	{
		FailDefense(
			TEXT("DefenseTimedOut"),
			LOCTEXT("TimeoutHeadline", "옹성 방어 실패"),
			LOCTEXT("TimeoutDetail", "제한 시간 안에 적 충차를 파괴하지 못했습니다"),
			LOCTEXT("TimeoutNotification", "시간이 초과되었습니다"));
	}
}

void AOngseongDefenseScenarioManager::SucceedDefense()
{
	if (DefenseState != EOngseongDefenseState::Defending) return;
	GetWorldTimerManager().ClearTimer(DefenseTimerHandle);
	SetDefenseState(EOngseongDefenseState::Succeeded);
	UE_LOG(LogOngseong, Display, TEXT("Defense succeeded: the ram was destroyed after %d enemies were defeated."), GetTotalDefeatedEnemies());
	Narration->ReportScenarioEvent(TEXT("DefenseSucceeded"), this);
	if (VRHUD)
	{
		VRHUD->SetObjective(LOCTEXT("SuccessHeadline", "옹성 방어 성공"), LOCTEXT("SuccessDetail", "적이 퇴각하고 있습니다"));
		VRHUD->ShowNotification(LOCTEXT("SuccessNotification", "적 충차를 파괴했습니다"), EVRHUDNotificationType::Success, 5.0f);
	}
	if (WaveManager)
	{
		const FVector RetreatLocation = RetreatPoint ? RetreatPoint->GetActorLocation() : GetActorLocation();
		WaveManager->RetreatAllEnemies(RetreatLocation);
	}
	else FinishSuccessfulRetreat();
}

void AOngseongDefenseScenarioManager::FailDefense(const FName NarrationEvent, const FText& Headline, const FText& Detail, const FText& Notification)
{
	if (DefenseState != EOngseongDefenseState::Defending) return;
	GetWorldTimerManager().ClearTimer(DefenseTimerHandle);
	if (ActiveRam) ActiveRam->StopRam();
	if (WaveManager) WaveManager->ReleaseAllEnemies();
	SetDefenseState(EOngseongDefenseState::Failed);
	UE_LOG(LogOngseong, Warning, TEXT("Defense failed (%s)."), *NarrationEvent.ToString());
	Narration->ReportScenarioEvent(NarrationEvent, GateActor);
	if (VRHUD)
	{
		const FText RetryDetail = bAutoRetryOnFailure
			? FText::Format(LOCTEXT("AutoRetryDetail", "{0} · {1}초 후 자동으로 다시 시작합니다"), Detail, FText::AsNumber(FMath::CeilToInt(AutoRetryDelay)))
			: Detail;
		VRHUD->SetObjective(Headline, RetryDetail);
		VRHUD->ShowNotification(Notification, EVRHUDNotificationType::Error, 5.0f);
	}
	if (bAutoRetryOnFailure)
	{
		GetWorldTimerManager().SetTimer(AutoRetryTimerHandle, this, &AOngseongDefenseScenarioManager::HandleAutoRetry, FMath::Max(1.0f, AutoRetryDelay), false);
	}
}

void AOngseongDefenseScenarioManager::FinishSuccessfulRetreat()
{
	if (!bReturnToMainOnSuccess) return;
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UExperienceSubsystem* Experience = GameInstance->GetSubsystem<UExperienceSubsystem>())
		{
			Experience->CompleteCurrentExperience(true);
		}
	}
}

void AOngseongDefenseScenarioManager::HandleGateDestroyed()
{
	FailDefense(
		TEXT("GateDestroyed"),
		LOCTEXT("GateLostHeadline", "옹성 방어 실패"),
		LOCTEXT("GateLostDetail", "성문이 파괴되었습니다"),
		LOCTEXT("GateLostNotification", "성문이 파괴되었습니다"));
}

void AOngseongDefenseScenarioManager::HandleAllEnemiesRetreated()
{
	if (DefenseState == EOngseongDefenseState::Succeeded) FinishSuccessfulRetreat();
}

void AOngseongDefenseScenarioManager::HandleRamDefeated(UHealthComponent* DeadHealth, const FCombatDamageSpec&)
{
	if (!IsValid(DeadHealth) || DeadHealth->GetOwner() != ActiveRam)
	{
		return;
	}

	// Destroying the ram is the clear condition. It is never replaced.
	bRamDestroyed = true;
	ReleaseActiveRam();
	SucceedDefense();
}

void AOngseongDefenseScenarioManager::SetDefenseState(const EOngseongDefenseState NewState)
{
	if (DefenseState == NewState) return;
	DefenseState = NewState;
	OnDefenseStateChanged.Broadcast(NewState);
}

void AOngseongDefenseScenarioManager::UpdateHUDTime()
{
	const int32 Seconds = FMath::CeilToInt(RemainingDefenseTime);
	OnDefenseTimeChanged.Broadcast(Seconds);
	if (!VRHUD)
	{
		return;
	}

	const FText Objective = LOCTEXT("DefenseObjective", "적 충차를 파괴하십시오");
	if (bUseDefenseTimeLimit)
	{
		const FString TimeText = FString::Printf(TEXT("%02d:%02d"), Seconds / 60, Seconds % 60);
		VRHUD->SetObjective(Objective, FText::Format(LOCTEXT("DefenseRemaining", "남은 시간 {0}"), FText::FromString(TimeText)));
	}
	else
	{
		VRHUD->SetObjective(Objective, LOCTEXT("DefenseDetail", "충차가 성문에 닿기 전에 총통으로 파괴하십시오"));
	}
}

void AOngseongDefenseScenarioManager::HandleAutoRetry()
{
	if (DefenseState == EOngseongDefenseState::Failed) RetryDefense();
}

#undef LOCTEXT_NAMESPACE
