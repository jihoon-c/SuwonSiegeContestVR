#include "Ongseong/OngseongDefenseScenarioManager.h"

#include "Core/Experience/ExperienceSubsystem.h"
#include "Gameplay/Combat/HealthComponent.h"
#include "Gameplay/UI/VRHUDComponent.h"
#include "Gameplay/UI/VRHUDTypes.h"
#include "Ongseong/OngseongEnemyWaveManager.h"
#include "Ongseong/OngseongGateActor.h"
#include "Ongseong/OngseongNarrationComponent.h"
#include "Ongseong/OngseongRamActor.h"
#include "TimerManager.h"

AOngseongDefenseScenarioManager::AOngseongDefenseScenarioManager()
{
	PrimaryActorTick.bCanEverTick = false;
	Narration = CreateDefaultSubobject<UOngseongNarrationComponent>(TEXT("OngseongNarration"));
	RamClass = AOngseongRamActor::StaticClass();
}

void AOngseongDefenseScenarioManager::BeginPlay()
{
	Super::BeginPlay();
	if (GateActor) GateActor->OnGateDestroyed.AddUniqueDynamic(this, &AOngseongDefenseScenarioManager::HandleGateDestroyed);
	if (WaveManager)
	{
		WaveManager->OnAllEnemiesDefeated.AddUniqueDynamic(this, &AOngseongDefenseScenarioManager::HandleWaveDefeated);
		WaveManager->OnAllEnemiesRetreated.AddUniqueDynamic(this, &AOngseongDefenseScenarioManager::HandleAllEnemiesRetreated);
	}
	Narration->GateActor = GateActor;
	Narration->WaveManager = WaveManager;
	Narration->InitializeNarrationBindings();
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (APawn* Pawn = PC->GetPawn()) VRHUD = Pawn->FindComponentByClass<UVRHUDComponent>();
	}
	if (bAutoStart) StartDefense();
}

void AOngseongDefenseScenarioManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
	if (ActiveRam) ActiveRam->StopRam();
	if (WaveManager)
	{
		WaveManager->OnAllEnemiesDefeated.RemoveDynamic(this, &AOngseongDefenseScenarioManager::HandleWaveDefeated);
		WaveManager->OnAllEnemiesRetreated.RemoveDynamic(this, &AOngseongDefenseScenarioManager::HandleAllEnemiesRetreated);
	}
	if (GateActor) GateActor->OnGateDestroyed.RemoveDynamic(this, &AOngseongDefenseScenarioManager::HandleGateDestroyed);
	Super::EndPlay(EndPlayReason);
}

bool AOngseongDefenseScenarioManager::StartDefense()
{
	if (DefenseState == EOngseongDefenseState::Defending || !IsValid(GateActor) || !IsValid(WaveManager)) return false;
	RemainingDefenseTime = FMath::Max(1.0f, DefenseDuration);
	bCurrentInfantryWaveDefeated = false;
	bCurrentRamDefeated = false;
	if (!SpawnAndActivateRam()) return false;
	WaveManager->ResetWave();
	SetDefenseState(EOngseongDefenseState::Defending);
	WaveManager->SetObjectiveTarget(GateActor);
	CurrentWaveNumber = 1;
	WaveManager->StartSpawning();
	GetWorldTimerManager().SetTimer(DefenseTimerHandle, this, &AOngseongDefenseScenarioManager::TickDefenseTimer, 1.0f, true);
	Narration->ReportScenarioEvent(TEXT("ScenarioStarted"), this);
	UpdateHUDTime();
	return true;
}

bool AOngseongDefenseScenarioManager::IsNextWavePending() const
{
	return GetWorld() && GetWorldTimerManager().IsTimerActive(NextWaveTimerHandle);
}

void AOngseongDefenseScenarioManager::RetryDefense()
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
	if (ActiveRam) { ActiveRam->Destroy(); ActiveRam = nullptr; }
	if (WaveManager) WaveManager->ResetWave();
	if (GateActor) GateActor->ResetGate();
	CurrentWaveNumber = 0;
	SetDefenseState(EOngseongDefenseState::Idle);
	StartDefense();
}

bool AOngseongDefenseScenarioManager::SpawnAndActivateRam()
{
	if (!RamClass || !IsValid(GateActor) || !IsValid(WaveManager) || ActiveRam) return false;
	const FTransform SpawnTransform = IsValid(RamSpawnPoint) ? RamSpawnPoint->GetActorTransform() : WaveManager->GetActorTransform();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	ActiveRam = GetWorld()->SpawnActor<AOngseongRamActor>(RamClass, SpawnTransform, Params);
	if (!ActiveRam) return false;
	if (UHealthComponent* RamHealth = ActiveRam->FindComponentByClass<UHealthComponent>())
	{
		RamHealth->OnDeath.AddUniqueDynamic(this, &AOngseongDefenseScenarioManager::HandleRamDefeated);
	}
	ActiveRam->ActivateRam(GateActor);
	return true;
}

void AOngseongDefenseScenarioManager::TickDefenseTimer()
{
	if (DefenseState != EOngseongDefenseState::Defending) return;
	RemainingDefenseTime = FMath::Max(0.0f, RemainingDefenseTime - 1.0f);
	UpdateHUDTime();
	if (RemainingDefenseTime <= 0.0f) SucceedDefense();
}

void AOngseongDefenseScenarioManager::SucceedDefense()
{
	if (DefenseState != EOngseongDefenseState::Defending) return;
	GetWorldTimerManager().ClearTimer(DefenseTimerHandle);
	GetWorldTimerManager().ClearTimer(NextWaveTimerHandle);
	if (ActiveRam) ActiveRam->StopRam();
	SetDefenseState(EOngseongDefenseState::Succeeded);
	Narration->ReportScenarioEvent(TEXT("DefenseSucceeded"), this);
	if (VRHUD)
	{
		VRHUD->SetObjective(FText::FromString(TEXT("옹성 방어 성공")), FText::FromString(TEXT("적이 퇴각하고 있습니다")));
		VRHUD->ShowNotification(FText::FromString(TEXT("성문을 지켜냈습니다")), EVRHUDNotificationType::Success, 5.0f);
	}
	if (WaveManager)
	{
		const FVector RetreatLocation = RetreatPoint ? RetreatPoint->GetActorLocation() : GetActorLocation();
		WaveManager->RetreatAllEnemies(RetreatLocation);
	}
	else FinishSuccessfulRetreat();
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
	if (DefenseState != EOngseongDefenseState::Defending) return;
	GetWorldTimerManager().ClearTimer(DefenseTimerHandle);
	GetWorldTimerManager().ClearTimer(NextWaveTimerHandle);
	if (ActiveRam) ActiveRam->StopRam();
	if (WaveManager) WaveManager->ReleaseAllEnemies();
	SetDefenseState(EOngseongDefenseState::Failed);
	Narration->ReportScenarioEvent(TEXT("GateDestroyed"), GateActor);
	if (VRHUD)
	{
		const FText RetryDetail = bAutoRetryOnFailure
			? FText::Format(FText::FromString(TEXT("{0}초 후 자동으로 다시 시작합니다")), FText::AsNumber(FMath::CeilToInt(AutoRetryDelay)))
			: FText::FromString(TEXT("재시도 동작으로 다시 시작할 수 있습니다"));
		VRHUD->SetObjective(FText::FromString(TEXT("옹성 방어 실패")), RetryDetail);
		VRHUD->ShowNotification(FText::FromString(TEXT("성문이 파괴되었습니다")), EVRHUDNotificationType::Error, 5.0f);
	}
	if (bAutoRetryOnFailure)
	{
		GetWorldTimerManager().SetTimer(AutoRetryTimerHandle, this, &AOngseongDefenseScenarioManager::HandleAutoRetry, FMath::Max(1.0f, AutoRetryDelay), false);
	}
}

void AOngseongDefenseScenarioManager::HandleAllEnemiesRetreated()
{
	if (DefenseState == EOngseongDefenseState::Succeeded) FinishSuccessfulRetreat();
}

void AOngseongDefenseScenarioManager::HandleWaveDefeated(const int32)
{
	bCurrentInfantryWaveDefeated = true;
	TryScheduleNextWave();
}

void AOngseongDefenseScenarioManager::HandleRamDefeated(UHealthComponent* DeadHealth, const FCombatDamageSpec&)
{
	if (!IsValid(DeadHealth) || DeadHealth->GetOwner() != ActiveRam)
	{
		return;
	}

	AOngseongRamActor* DefeatedRam = ActiveRam;
	ActiveRam = nullptr;
	bCurrentRamDefeated = true;
	DefeatedRam->Destroy();
	TryScheduleNextWave();
}

void AOngseongDefenseScenarioManager::TryScheduleNextWave()
{
	if (DefenseState != EOngseongDefenseState::Defending || !bRepeatWavesDuringDefense ||
		RemainingDefenseTime <= 0.0f || !bCurrentInfantryWaveDefeated || !bCurrentRamDefeated)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(NextWaveTimerHandle);
	GetWorldTimerManager().SetTimer(
		NextWaveTimerHandle,
		this,
		&AOngseongDefenseScenarioManager::StartNextWave,
		FMath::Max(0.1f, InterWaveDelay),
		false);
}

void AOngseongDefenseScenarioManager::StartNextWave()
{
	if (DefenseState != EOngseongDefenseState::Defending || !bRepeatWavesDuringDefense ||
		RemainingDefenseTime <= 0.0f || !IsValid(WaveManager) || !IsValid(GateActor))
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(NextWaveTimerHandle);
	WaveManager->ResetWave();
	WaveManager->SetObjectiveTarget(GateActor);
	bCurrentInfantryWaveDefeated = false;
	bCurrentRamDefeated = false;
	if (!SpawnAndActivateRam())
	{
		return;
	}
	++CurrentWaveNumber;
	WaveManager->StartSpawning();
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
	if (VRHUD)
	{
		const FString TimeText = FString::Printf(TEXT("%02d:%02d"), Seconds / 60, Seconds % 60);
		VRHUD->SetObjective(FText::FromString(TEXT("옹성을 방어하십시오")), FText::Format(FText::FromString(TEXT("남은 시간 {0}")), FText::FromString(TimeText)));
	}
}

void AOngseongDefenseScenarioManager::HandleAutoRetry()
{
	if (DefenseState == EOngseongDefenseState::Failed) RetryDefense();
}
