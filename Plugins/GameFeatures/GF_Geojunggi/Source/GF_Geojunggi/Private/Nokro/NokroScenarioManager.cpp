#include "Nokro/NokroScenarioManager.h"

#include "Core/Experience/ExperienceSubsystem.h"
#include "Core/Scenario/ScenarioInteractableComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Gameplay/UI/VRHUDComponent.h"
#include "Gameplay/UI/VRHUDTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Nokro/NokroCraneActor.h"
#include "Nokro/NokroNarrationComponent.h"
#include "Nokro/NokroRepairTargetActor.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "NokroVRUI"

ANokroScenarioManager::ANokroScenarioManager()
{
	PrimaryActorTick.bCanEverTick = false;
	Narration = CreateDefaultSubobject<UNokroNarrationComponent>(TEXT("NokroNarration"));
	CraneClass = ANokroCraneActor::StaticClass();
	RepairTargetClass = ANokroRepairTargetActor::StaticClass();
	DefaultTargetOffsets = {
		FVector(400.0f, 0.0f, 180.0f), FVector(0.0f, 400.0f, 290.0f),
		FVector(-400.0f, 0.0f, 410.0f), FVector(0.0f, -400.0f, 245.0f) };
}

void ANokroScenarioManager::BeginPlay()
{
	Super::BeginPlay();
	DiscoverOrSpawnLayout();
	BindCrane();
	if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0)) VRHUD = Pawn->FindComponentByClass<UVRHUDComponent>();
	Narration->InitializeNarration();
	if (bAutoStart) StartScenario();
}

void ANokroScenarioManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
	if (Crane)
	{
		Crane->OnPlacementRequested.RemoveDynamic(this, &ThisClass::HandlePlacementRequested);
		Crane->OnHandleGrabbed.RemoveDynamic(this, &ThisClass::HandleHandleGrabbed);
		Crane->OnHeightAdjusted.RemoveDynamic(this, &ThisClass::HandleHeightAdjusted);
		Crane->OnDirectionAdjusted.RemoveDynamic(this, &ThisClass::HandleDirectionAdjusted);
	}
	if (VRHUD) VRHUD->ClearAll();
	Super::EndPlay(EndPlayReason);
}

void ANokroScenarioManager::DiscoverOrSpawnLayout()
{
	if (!Crane)
	{
		for (TActorIterator<ANokroCraneActor> It(GetWorld()); It; ++It)
		{
			Crane = *It;
			break;
		}
	}
	if (!Crane && bSpawnDefaultLayout && CraneClass)
	{
		Crane = GetWorld()->SpawnActor<ANokroCraneActor>(CraneClass, GetActorTransform());
	}

	if (RepairTargets.IsEmpty())
	{
		for (TActorIterator<ANokroRepairTargetActor> It(GetWorld()); It; ++It) RepairTargets.Add(*It);
	}
	if (RepairTargets.IsEmpty() && bSpawnDefaultLayout && RepairTargetClass)
	{
		for (const FVector& Offset : DefaultTargetOffsets)
		{
			FTransform Transform = GetActorTransform();
			Transform.SetLocation(GetActorTransform().TransformPosition(Offset));
			const FVector FlatOffset(Offset.X, Offset.Y, 0.0f);
			Transform.SetRotation(FRotator(0.0f, FlatOffset.Rotation().Yaw, 0.0f).Quaternion());
			if (ANokroRepairTargetActor* Target = GetWorld()->SpawnActor<ANokroRepairTargetActor>(RepairTargetClass, Transform))
			{
				RepairTargets.Add(Target);
			}
		}
	}
}

void ANokroScenarioManager::BindCrane()
{
	if (!Crane) return;
	Crane->OnPlacementRequested.AddUniqueDynamic(this, &ThisClass::HandlePlacementRequested);
	Crane->OnHandleGrabbed.AddUniqueDynamic(this, &ThisClass::HandleHandleGrabbed);
	Crane->OnHeightAdjusted.AddUniqueDynamic(this, &ThisClass::HandleHeightAdjusted);
	Crane->OnDirectionAdjusted.AddUniqueDynamic(this, &ThisClass::HandleDirectionAdjusted);
}

bool ANokroScenarioManager::StartScenario()
{
	if (!IsValid(Crane) || RepairTargets.IsEmpty()) return false;
	GetWorldTimerManager().ClearTimer(ExperienceCompletionTimer);
	for (ANokroRepairTargetActor* Target : RepairTargets) if (Target) Target->ResetRepair();
	Crane->ResetCarriedStone(true);
	ScenarioState = ENokroScenarioState::Repairing;
	Narration->ReportScenarioEvent(TEXT("ScenarioStarted"));
	if (RepairTargets.Num() == 1) Narration->ReportScenarioEvent(TEXT("LastStone"));
	UpdateHUD();
	OnRepairProgress.Broadcast(0, RepairTargets.Num());
	return true;
}

void ANokroScenarioManager::ResetScenario()
{
	ScenarioState = ENokroScenarioState::Idle;
	StartScenario();
}

bool ANokroScenarioManager::TryPlaceStoneAtTransform(const FTransform& StoneTransform)
{
	if (ScenarioState != ENokroScenarioState::Repairing) return false;
	ANokroRepairTargetActor* MatchedTarget = nullptr;
	for (ANokroRepairTargetActor* Target : RepairTargets)
	{
		if (IsValid(Target) && Target->IsStoneWithinTolerance(StoneTransform))
		{
			MatchedTarget = Target;
			break;
		}
	}

	if (!MatchedTarget)
	{
		Crane->ScenarioInteraction->ReportInteractionFailed(EScenarioInteractionType::Trigger);
		Crane->ResetCarriedStone(true);
		Narration->ReportScenarioEvent(TEXT("PlacementFailed"));
		if (VRHUD)
		{
			VRHUD->ShowNotification(LOCTEXT("PlacementFailed", "성돌의 위치가 정확하지 않습니다. 처음 위치로 돌아갑니다."), EVRHUDNotificationType::Error, 4.0f);
		}
		OnPlacementResult.Broadcast(false);
		return false;
	}

	MatchedTarget->CompleteRepair();
	Crane->ScenarioInteraction->ReportInteractionCompleted(EScenarioInteractionType::Trigger);
	const int32 Repaired = GetRepairedCount();
	const int32 Total = RepairTargets.Num();
	OnRepairProgress.Broadcast(Repaired, Total);
	OnPlacementResult.Broadcast(true);
	if (VRHUD) VRHUD->ShowNotification(LOCTEXT("PlacementSucceeded", "성돌이 정확한 위치에 배치되었습니다."), EVRHUDNotificationType::Success, 3.0f);

	if (Repaired >= Total)
	{
		Crane->ResetCarriedStone(false);
		CompleteScenario();
	}
	else
	{
		Crane->ResetCarriedStone(true);
		Narration->ReportScenarioEvent(TEXT("PlacementSucceeded"));
		if (Total - Repaired == 1) Narration->ReportScenarioEvent(TEXT("LastStone"));
		else Narration->ReportScenarioEvent(TEXT("Progress"));
		UpdateHUD();
	}
	return true;
}

int32 ANokroScenarioManager::GetRepairedCount() const
{
	int32 RepairedCount = 0;
	for (const ANokroRepairTargetActor* Target : RepairTargets)
	{
		if (IsValid(Target) && Target->IsRepaired()) ++RepairedCount;
	}
	return RepairedCount;
}

void ANokroScenarioManager::HandlePlacementRequested(const FTransform StoneTransform)
{
	TryPlaceStoneAtTransform(StoneTransform);
}

void ANokroScenarioManager::HandleHandleGrabbed()
{
	if (ScenarioState == ENokroScenarioState::Repairing)
	{
		Narration->ReportScenarioEvent(TEXT("HandleGrabbed"));
		if (VRHUD) VRHUD->ShowPrompt(LOCTEXT("CrankPrompt", "손잡이를 돌려 성돌의 높이를 조절하십시오"));
	}
}

void ANokroScenarioManager::HandleHeightAdjusted()
{
	if (ScenarioState == ENokroScenarioState::Repairing)
	{
		Narration->ReportScenarioEvent(TEXT("HeightAdjusted"));
		if (VRHUD) VRHUD->ShowPrompt(LOCTEXT("TurnPrompt", "조이스틱으로 녹로의 방향을 조절하십시오"));
	}
}

void ANokroScenarioManager::HandleDirectionAdjusted()
{
	if (ScenarioState == ENokroScenarioState::Repairing)
	{
		Narration->ReportScenarioEvent(TEXT("DirectionAdjusted"));
		if (VRHUD) VRHUD->ShowPrompt(LOCTEXT("PlacePrompt", "노란 위치에 맞춘 뒤 양손 트리거를 동시에 누르십시오"));
	}
}

void ANokroScenarioManager::UpdateHUD()
{
	if (!VRHUD) return;
	const int32 Repaired = GetRepairedCount();
	const int32 Total = RepairTargets.Num();
	VRHUD->SetObjective(LOCTEXT("RepairObjective", "녹로로 성벽을 보수하십시오"),
		LOCTEXT("RepairDetail", "손잡이: 높이 · 조이스틱: 방향 · 양손 트리거: 배치"));
	VRHUD->SetProgress(LOCTEXT("RepairProgress", "성돌 배치"), Repaired, Total);
	if (Repaired == 0) VRHUD->ShowPrompt(LOCTEXT("InitialPrompt", "녹로 손잡이를 그랩 버튼으로 잡으십시오"));
}

void ANokroScenarioManager::CompleteScenario()
{
	ScenarioState = ENokroScenarioState::Completed;
	Narration->ReportScenarioEvent(TEXT("ScenarioCompleted"));
	if (VRHUD)
	{
		VRHUD->ClearPrompt();
		VRHUD->SetProgress(LOCTEXT("RepairProgress", "성돌 배치"), RepairTargets.Num(), RepairTargets.Num());
		VRHUD->SetObjective(LOCTEXT("RepairComplete", "성벽 보수 완료"), LOCTEXT("RepairCompleteDetail", "모든 파손 부분을 복구했습니다"));
		VRHUD->ShowNotification(LOCTEXT("AllComplete", "파손된 성벽의 보수가 모두 완료되었습니다."), EVRHUDNotificationType::Success, 6.0f);
	}
	if (bCompleteExperienceOnFinish)
	{
		GetWorldTimerManager().SetTimer(ExperienceCompletionTimer, this, &ThisClass::CompleteExperience,
			FMath::Max(0.1f, ExperienceCompletionDelay), false);
	}
}

void ANokroScenarioManager::CompleteExperience()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UExperienceSubsystem* Experience = GameInstance->GetSubsystem<UExperienceSubsystem>())
		{
			Experience->CompleteCurrentExperience(true);
		}
	}
}

#undef LOCTEXT_NAMESPACE
