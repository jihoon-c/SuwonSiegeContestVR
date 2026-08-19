#include "Gameplay/AI/EnemySimpleMovementComponent.h"

#include "GameFramework/Actor.h"

UEnemySimpleMovementComponent::UEnemySimpleMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = UpdateInterval;
	SetComponentTickEnabled(false);
}

void UEnemySimpleMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	AActor* Owner = GetOwner();
	if (!bSimpleMovementEnabled || !IsValid(Owner))
	{
		return;
	}

	const FVector Destination = IsValid(TargetActor) ? TargetActor->GetActorLocation() : TargetLocation;
	if (!IsValid(TargetActor) && !bHasLocationTarget)
	{
		return;
	}

	FVector ToDestination = Destination - Owner->GetActorLocation();
	ToDestination.Z = 0.0f;
	if (ToDestination.SizeSquared() <= FMath::Square(AcceptanceRadius))
	{
		if (!bTargetReached)
		{
			bTargetReached = true;
			OnTargetReached.Broadcast(Owner);
		}
		return;
	}

	bTargetReached = false;
	const FVector MoveDelta = ToDestination.GetSafeNormal() * MoveSpeed * DeltaTime;
	Owner->AddActorWorldOffset(MoveDelta, bSweepMovement);
}

void UEnemySimpleMovementComponent::SetSimpleMovementEnabled(const bool bEnabled)
{
	bSimpleMovementEnabled = bEnabled;
	PrimaryComponentTick.TickInterval = UpdateInterval;
	SetComponentTickEnabled(bEnabled);
}

void UEnemySimpleMovementComponent::SetMoveTargetActor(AActor* NewTargetActor)
{
	TargetActor = NewTargetActor;
	bHasLocationTarget = false;
	bTargetReached = false;
}

void UEnemySimpleMovementComponent::SetMoveTargetLocation(const FVector NewTargetLocation)
{
	TargetActor = nullptr;
	TargetLocation = NewTargetLocation;
	bHasLocationTarget = true;
	bTargetReached = false;
}

void UEnemySimpleMovementComponent::ClearMoveTarget()
{
	TargetActor = nullptr;
	bHasLocationTarget = false;
	bTargetReached = false;
}
