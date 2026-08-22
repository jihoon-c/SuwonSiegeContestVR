#include "Ongseong/OngseongDefenseScenarioManager.h"

#include "Core/Experience/ExperienceSubsystem.h"
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
	if (WaveManager) WaveManager->OnAllEnemiesRetreated.AddUniqueDynamic(this, &AOngseongDefenseScenarioManager::HandleAllEnemiesRetreated);
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
	Super::EndPlay(EndPlayReason);
}

bool AOngseongDefenseScenarioManager::StartDefense()
{
	if (DefenseState == EOngseongDefenseState::Defending || !IsValid(GateActor) || !IsValid(WaveManager)) return false;
	RemainingDefenseTime = FMath::Max(1.0f, DefenseDuration);
	if (!SpawnAndActivateRam()) return false;
	SetDefenseState(EOngseongDefenseState::Defending);
	WaveManager->SetObjectiveTarget(GateActor);
	WaveManager->StartSpawning();
	GetWorldTimerManager().SetTimer(DefenseTimerHandle, this, &AOngseongDefenseScenarioManager::TickDefenseTimer, 1.0f, true);
	Narration->ReportScenarioEvent(TEXT("ScenarioStarted"), this);
	UpdateHUDTime();
	return true;
}

void AOngseongDefenseScenarioManager::RetryDefense()
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
	if (ActiveRam) { ActiveRam->Destroy(); ActiveRam = nullptr; }
	if (WaveManager) WaveManager->ResetWave();
	if (GateActor) GateActor->ResetGate();
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
	ActiveRam->ActivateRam(GateActor);
	return true;
}

void AOngseongDefenseScenarioManager::TickDefenseTimer()
{
	if (DefenseState != EOngseongDefenseState::Defending) return;
	RemainingDefenseTime = FMath::Max(0.0f, RemainingDefenseTime - 1.0f);
	OnDefenseTimeChanged.Broadcast(FMath::CeilToInt(RemainingDefenseTime));
	UpdateHUDTime();
	if (RemainingDefenseTime <= 0.0f) SucceedDefense();
}

void AOngseongDefenseScenarioManager::SucceedDefense()
{
	if (DefenseState != EOngseongDefenseState::Defending) return;
	GetWorldTimerManager().ClearTimer(DefenseTimerHandle);
	if (ActiveRam) ActiveRam->StopRam();
	SetDefenseState(EOngseongDefenseState::Succeeded);
	Narration->ReportScenarioEvent(TEXT("DefenseSucceeded"), this);
	if (VRHUD) VRHUD->ShowNotification(FText::FromString(TEXT("옹성 방어 성공")), EVRHUDNotificationType::Success, 5.0f);
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
	if (ActiveRam) ActiveRam->StopRam();
	if (WaveManager) WaveManager->ReleaseAllEnemies();
	SetDefenseState(EOngseongDefenseState::Failed);
	Narration->ReportScenarioEvent(TEXT("GateDestroyed"), GateActor);
	if (VRHUD)
	{
		VRHUD->SetObjective(FText::FromString(TEXT("옹성 방어 실패")), FText::FromString(TEXT("다시 시도할 수 있습니다")));
		VRHUD->ShowNotification(FText::FromString(TEXT("성문이 파괴되었습니다")), EVRHUDNotificationType::Error, 5.0f);
	}
}

void AOngseongDefenseScenarioManager::HandleAllEnemiesRetreated()
{
	if (DefenseState == EOngseongDefenseState::Succeeded) FinishSuccessfulRetreat();
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
	if (VRHUD) VRHUD->SetObjective(FText::FromString(TEXT("옹성을 방어하십시오")), FText::Format(FText::FromString(TEXT("남은 시간 {0}초")), FText::AsNumber(Seconds)));
}
