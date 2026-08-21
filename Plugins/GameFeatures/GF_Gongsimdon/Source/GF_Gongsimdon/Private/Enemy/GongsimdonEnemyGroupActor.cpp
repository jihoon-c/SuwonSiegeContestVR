#include "Enemy/GongsimdonEnemyGroupActor.h"

#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "Core/Scenario/ScenarioInteractableComponent.h"
#include "Core/Scenario/ScenarioObservationComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Scenario/GongsimdonScenarioDirectorActor.h"
#include "Shared/Characters/EnemySoldierActor.h"
#include "Shared/Combat/LegacyHealthComponent.h"
#include "TimerManager.h"

AGongsimdonEnemyGroupActor::AGongsimdonEnemyGroupActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	ApproachSpline = CreateDefaultSubobject<USplineComponent>(TEXT("ApproachSpline"));
	ApproachSpline->SetupAttachment(SceneRoot);
	ApproachSpline->ClearSplinePoints(false);
	ApproachSpline->AddSplinePoint(FVector::ZeroVector, ESplineCoordinateSpace::Local, false);
	ApproachSpline->AddSplinePoint(FVector(1500.0f, 0.0f, 0.0f), ESplineCoordinateSpace::Local, true);

	RetreatSpline = CreateDefaultSubobject<USplineComponent>(TEXT("RetreatSpline"));
	RetreatSpline->SetupAttachment(SceneRoot);
	RetreatSpline->ClearSplinePoints(false);
	RetreatSpline->AddSplinePoint(FVector(1500.0f, 0.0f, 0.0f), ESplineCoordinateSpace::Local, false);
	RetreatSpline->AddSplinePoint(FVector(3200.0f, 500.0f, 0.0f), ESplineCoordinateSpace::Local, true);

	ObservationAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("ObservationAnchor"));
	ObservationAnchor->SetupAttachment(SceneRoot);

	Observation = CreateDefaultSubobject<UScenarioObservationComponent>(TEXT("Observation"));
	Observation->ObservationTarget = ObservationAnchor;
	Observation->RequiredViewTime = 2.0f;
	Observation->RequiredViewAngle = 16.0f;
	Observation->MaxDistance = 6000.0f;
	Observation->bRequireLineOfSight = false;

	CombatInteraction = CreateDefaultSubobject<UScenarioInteractableComponent>(TEXT("CombatInteraction"));
	CombatInteraction->SupportedInteractionTypes = {EScenarioInteractionType::Combat};

	EnemyClass = AEnemySoldierActor::StaticClass();
}

void AGongsimdonEnemyGroupActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	EnemyCount = FMath::Clamp(EnemyCount, 6, 8);
	FormationColumns = FMath::Max(1, FormationColumns);
	RequiredCombatHits = FMath::Max(1, RequiredCombatHits);
	if (Observation)
	{
		Observation->TargetID = ObservationTargetID;
		Observation->SetInteractionEnabled(false);
	}
	if (CombatInteraction)
	{
		CombatInteraction->TargetID = CombatTargetID;
		CombatInteraction->SetInteractionEnabled(false);
	}
}

void AGongsimdonEnemyGroupActor::BeginPlay()
{
	Super::BeginPlay();
	SpawnEnemies();
	HideGroup();
	GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		BindScenarioDirector();
	}));
}

void AGongsimdonEnemyGroupActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroySpawnedEnemies();
	Super::EndPlay(EndPlayReason);
}

void AGongsimdonEnemyGroupActor::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	USplineComponent* ActiveSpline = nullptr;
	float Speed = 0.0f;
	if (GroupState == EGongsimdonEnemyGroupState::Approaching)
	{
		ActiveSpline = ApproachSpline;
		Speed = ApproachSpeed;
	}
	else if (GroupState == EGongsimdonEnemyGroupState::Retreating)
	{
		ActiveSpline = RetreatSpline;
		Speed = RetreatSpeed;
	}
	if (!ActiveSpline)
	{
		SetActorTickEnabled(false);
		return;
	}

	TravelDistance += FMath::Max(0.0f, DeltaSeconds) * Speed;
	UpdateFormation(ActiveSpline, TravelDistance);
	const float CompletionDistance = ActiveSpline->GetSplineLength() + GetFormationTailLength();
	if (TravelDistance < CompletionDistance)
	{
		return;
	}

	if (GroupState == EGongsimdonEnemyGroupState::Approaching)
	{
		SetGroupState(EGongsimdonEnemyGroupState::Holding);
	}
	else
	{
		// Keep the formation targetable at the path end until the required shot lands.
		SetGroupState(bCombatArmed
			? EGongsimdonEnemyGroupState::Holding
			: EGongsimdonEnemyGroupState::Escaped);
		if (!bCombatArmed)
		{
			SetEnemiesActive(false);
		}
	}
	SetActorTickEnabled(false);
}

bool AGongsimdonEnemyGroupActor::SpawnEnemies()
{
	if (!GetWorld() || !EnemyClass)
	{
		return false;
	}
	DestroySpawnedEnemies();
	const FActorSpawnParameters SpawnParameters;
	for (int32 Index = 0; Index < EnemyCount; ++Index)
	{
		AEnemySoldierActor* Enemy = GetWorld()->SpawnActor<AEnemySoldierActor>(
			EnemyClass, GetActorTransform(), SpawnParameters);
		if (!Enemy)
		{
			DestroySpawnedEnemies();
			return false;
		}
		SpawnedEnemies.Add(Enemy);
		if (ULegacyHealthComponent* Health = Enemy->GetHealthComponent())
		{
			Health->OnHealthChanged.AddUniqueDynamic(this, &ThisClass::HandleEnemyHealthChanged);
		}
	}
	UpdateFormation(ApproachSpline, 0.0f);
	SetEnemiesActive(false);
	return SpawnedEnemies.Num() == EnemyCount;
}

void AGongsimdonEnemyGroupActor::BeginApproach()
{
	if (SpawnedEnemies.Num() != EnemyCount && !SpawnEnemies())
	{
		return;
	}
	TravelDistance = 0.0f;
	SetEnemiesActive(true);
	UpdateFormation(ApproachSpline, TravelDistance);
	SetGroupState(EGongsimdonEnemyGroupState::Approaching);
	SetActorTickEnabled(true);
}

void AGongsimdonEnemyGroupActor::StartRetreat()
{
	if (SpawnedEnemies.IsEmpty())
	{
		return;
	}
	TravelDistance = 0.0f;
	SetEnemiesActive(true);
	UpdateFormation(RetreatSpline, TravelDistance);
	SetGroupState(EGongsimdonEnemyGroupState::Retreating);
	SetActorTickEnabled(true);
}

void AGongsimdonEnemyGroupActor::HideGroup()
{
	TravelDistance = 0.0f;
	SetActorTickEnabled(false);
	SetEnemiesActive(false);
	SetGroupState(EGongsimdonEnemyGroupState::Hidden);
}

void AGongsimdonEnemyGroupActor::ActivateObservation()
{
	if (Observation)
	{
		Observation->SetInteractionEnabled(true);
		Observation->StartObservation();
	}
}

void AGongsimdonEnemyGroupActor::DeactivateObservation(const bool bReportFailure)
{
	if (Observation)
	{
		Observation->StopObservation(bReportFailure);
		Observation->SetInteractionEnabled(false);
	}
}

void AGongsimdonEnemyGroupActor::SetCombatArmed(const bool bArmed)
{
	bCombatArmed = bArmed;
	CombatHits = 0;
	if (CombatInteraction)
	{
		CombatInteraction->SetInteractionEnabled(bArmed);
	}
}

AEnemySoldierActor* AGongsimdonEnemyGroupActor::GetSpawnedEnemy(const int32 Index) const
{
	return SpawnedEnemies.IsValidIndex(Index) ? SpawnedEnemies[Index].Get() : nullptr;
}

void AGongsimdonEnemyGroupActor::HandleCueRequested(FName InteractionID, FName TargetID)
{
	if (TargetID == TEXT("CUE_REVEAL_ENEMY"))
	{
		BeginApproach();
	}
	else if (TargetID == TEXT("CUE_RETREAT_ENEMY"))
	{
		StartRetreat();
	}
}

void AGongsimdonEnemyGroupActor::HandleEnemyHealthChanged(
	AActor* OwnerActor,
	float CurrentHealth,
	float MaxHealth,
	const float HealthDelta)
{
	if (!bCombatArmed || HealthDelta >= 0.0f || !CombatInteraction)
	{
		return;
	}
	++CombatHits;
	CombatInteraction->ReportInteractionProgress(
		EScenarioInteractionType::Combat,
		static_cast<float>(CombatHits) / static_cast<float>(RequiredCombatHits));
	if (CombatHits < RequiredCombatHits)
	{
		return;
	}
	if (CombatInteraction->ReportInteractionCompleted(EScenarioInteractionType::Combat))
	{
		bCombatArmed = false;
		CombatInteraction->SetInteractionEnabled(false);
		SetGroupState(EGongsimdonEnemyGroupState::Escaped);
		SetEnemiesActive(false);
		SetActorTickEnabled(false);
	}
}

void AGongsimdonEnemyGroupActor::BindScenarioDirector()
{
	if (!GetWorld())
	{
		return;
	}
	for (TActorIterator<AGongsimdonScenarioDirectorActor> It(GetWorld()); It; ++It)
	{
		It->OnCueRequested.AddUniqueDynamic(this, &ThisClass::HandleCueRequested);
		break;
	}
}

void AGongsimdonEnemyGroupActor::SetGroupState(const EGongsimdonEnemyGroupState NewState)
{
	if (GroupState == NewState)
	{
		return;
	}
	const EGongsimdonEnemyGroupState OldState = GroupState;
	GroupState = NewState;
	OnGroupStateChanged.Broadcast(OldState, NewState);
}

void AGongsimdonEnemyGroupActor::SetEnemiesActive(const bool bActive)
{
	for (AEnemySoldierActor* Enemy : SpawnedEnemies)
	{
		if (IsValid(Enemy))
		{
			Enemy->SetSoldierActive(bActive);
		}
	}
}

void AGongsimdonEnemyGroupActor::UpdateFormation(
	USplineComponent* Spline,
	const float LeaderDistance)
{
	if (!Spline)
	{
		return;
	}
	const int32 Columns = FMath::Max(1, FormationColumns);
	const float SplineLength = Spline->GetSplineLength();
	FVector Center = FVector::ZeroVector;
	int32 ValidEnemyCount = 0;
	for (int32 Index = 0; Index < SpawnedEnemies.Num(); ++Index)
	{
		AEnemySoldierActor* Enemy = SpawnedEnemies[Index].Get();
		if (!IsValid(Enemy))
		{
			continue;
		}
		const int32 Row = Index / Columns;
		const int32 Column = Index % Columns;
		const float Distance = FMath::Clamp(LeaderDistance - Row * RowSpacing, 0.0f, SplineLength);
		const FRotator Rotation = Spline->GetRotationAtDistanceAlongSpline(
			Distance, ESplineCoordinateSpace::World);
		const FVector Right = Rotation.RotateVector(FVector::RightVector);
		const float LateralIndex = static_cast<float>(Column) -
			(static_cast<float>(Columns - 1) * 0.5f);
		const FVector Location = Spline->GetLocationAtDistanceAlongSpline(
			Distance, ESplineCoordinateSpace::World) + Right * LateralIndex * LateralSpacing;
		Enemy->SetActorLocationAndRotation(Location, Rotation, false, nullptr, ETeleportType::TeleportPhysics);
		Center += Location;
		++ValidEnemyCount;
	}
	if (ObservationAnchor && ValidEnemyCount > 0)
	{
		ObservationAnchor->SetWorldLocation(Center / static_cast<float>(ValidEnemyCount));
	}
}

float AGongsimdonEnemyGroupActor::GetFormationTailLength() const
{
	const int32 Rows = FMath::CeilToInt(
		static_cast<float>(FMath::Max(1, EnemyCount)) /
		static_cast<float>(FMath::Max(1, FormationColumns)));
	return FMath::Max(0, Rows - 1) * RowSpacing;
}

void AGongsimdonEnemyGroupActor::DestroySpawnedEnemies()
{
	for (AEnemySoldierActor* Enemy : SpawnedEnemies)
	{
		if (IsValid(Enemy))
		{
			Enemy->Destroy();
		}
	}
	SpawnedEnemies.Reset();
}
