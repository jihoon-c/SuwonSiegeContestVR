#include "Gameplay/AI/EnemyAILODComponent.h"

#include "Components/MeshComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Gameplay/AI/EnemyAIController.h"
#include "Gameplay/AI/EnemySimpleMovementComponent.h"
#include "Kismet/GameplayStatics.h"

UEnemyAILODComponent::UEnemyAILODComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = EvaluationInterval;
}

void UEnemyAILODComponent::BeginPlay()
{
	Super::BeginPlay();
	PrimaryComponentTick.TickInterval = EvaluationInterval;
	RefreshLOD();
	ApplyLODLevel(CurrentLODLevel, true);
}

void UEnemyAILODComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	RefreshLOD();
}

void UEnemyAILODComponent::SetLODReferenceActor(AActor* NewReferenceActor)
{
	LODReferenceActor = NewReferenceActor;
	RefreshLOD();
}

void UEnemyAILODComponent::RefreshLOD()
{
	AActor* Owner = GetOwner();
	AActor* Reference = ResolveReferenceActor();
	if (!IsValid(Owner) || !IsValid(Reference))
	{
		ApplyLODLevel(EEnemyAILODLevel::Far);
		return;
	}

	const float DistanceSquared = FVector::DistSquared2D(Owner->GetActorLocation(), Reference->GetActorLocation());
	const float Threshold = CurrentLODLevel == EEnemyAILODLevel::Near ? ExitNearDistance : EnterNearDistance;
	ApplyLODLevel(DistanceSquared <= FMath::Square(Threshold) ? EEnemyAILODLevel::Near : EEnemyAILODLevel::Far);
}

void UEnemyAILODComponent::ApplyLODLevel(const EEnemyAILODLevel NewLevel, const bool bForce)
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return;
	}

	const EEnemyAILODLevel PreviousLevel = CurrentLODLevel;
	if (!bForce && PreviousLevel == NewLevel)
	{
		return;
	}

	CurrentLODLevel = NewLevel;
	const bool bNear = CurrentLODLevel == EEnemyAILODLevel::Near;
	Owner->SetActorTickInterval(bNear ? NearActorTickInterval : FarActorTickInterval);

	if (bHideVisualsWhenFar)
	{
		TArray<UMeshComponent*> MeshComponents;
		Owner->GetComponents(MeshComponents);
		for (UMeshComponent* MeshComponent : MeshComponents)
		{
			MeshComponent->SetVisibility(bNear, true);
		}
	}

	bool bHasHighDetailAI = false;
	if (const APawn* PawnOwner = Cast<APawn>(Owner))
	{
		if (AEnemyAIController* Controller = Cast<AEnemyAIController>(PawnOwner->GetController()))
		{
			Controller->SetHighDetailAIEnabled(bNear);
			bHasHighDetailAI = Controller->HasHighDetailAIImplementation();
		}
	}
	if (UEnemySimpleMovementComponent* SimpleMovement = Owner->FindComponentByClass<UEnemySimpleMovementComponent>())
	{
		// Preserve basic movement until a Feature actually configures a close-range BT or StateTree.
		SimpleMovement->SetSimpleMovementEnabled(!bNear || !bHasHighDetailAI);
	}

	if (PreviousLevel != CurrentLODLevel)
	{
		OnLODChanged.Broadcast(PreviousLevel, CurrentLODLevel);
	}
}

AActor* UEnemyAILODComponent::ResolveReferenceActor() const
{
	if (IsValid(LODReferenceActor))
	{
		return LODReferenceActor;
	}

	return GetWorld() ? UGameplayStatics::GetPlayerPawn(this, 0) : nullptr;
}
