#include "Gameplay/Combat/CombatTargetingComponent.h"

#include "EngineUtils.h"
#include "Engine/World.h"
#include "Gameplay/Combat/CombatFactionComponent.h"
#include "Gameplay/Combat/HealthComponent.h"

UCombatTargetingComponent::UCombatTargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

TArray<AActor*> UCombatTargetingComponent::FindVisibleHostileTargets(const float SearchRadius) const
{
	TArray<AActor*> VisibleTargets;
	for (AActor* Candidate : FindHostileTargets(SearchRadius))
	{
		if (HasLineOfSightTo(Candidate))
		{
			VisibleTargets.Add(Candidate);
		}
	}
	return VisibleTargets;
}

TArray<AActor*> UCombatTargetingComponent::FindHostileTargets(const float SearchRadius) const
{
	TArray<AActor*> Targets;
	const AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !GetWorld())
	{
		return Targets;
	}

	const float SearchRadiusSquared = FMath::Square(FMath::Max(0.0f, SearchRadius));
	for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
	{
		AActor* Candidate = *ActorIterator;
		if (IsValidHostileTarget(Candidate) && FVector::DistSquared(Owner->GetActorLocation(), Candidate->GetActorLocation()) <= SearchRadiusSquared)
		{
			Targets.Add(Candidate);
		}
	}
	return Targets;
}

AActor* UCombatTargetingComponent::SelectClosestTo(const TArray<AActor*>& Candidates, const AActor* ReferenceActor) const
{
	if (!IsValid(ReferenceActor))
	{
		return nullptr;
	}

	AActor* BestTarget = nullptr;
	float BestDistanceSquared = TNumericLimits<float>::Max();
	for (AActor* Candidate : Candidates)
	{
		if (IsValid(Candidate))
		{
			const float DistanceSquared = FVector::DistSquared(Candidate->GetActorLocation(), ReferenceActor->GetActorLocation());
			if (DistanceSquared < BestDistanceSquared)
			{
				BestDistanceSquared = DistanceSquared;
				BestTarget = Candidate;
			}
		}
	}
	return BestTarget;
}

AActor* UCombatTargetingComponent::SelectRandom(const TArray<AActor*>& Candidates) const
{
	TArray<AActor*> ValidCandidates;
	for (AActor* Candidate : Candidates)
	{
		if (IsValid(Candidate))
		{
			ValidCandidates.Add(Candidate);
		}
	}
	return ValidCandidates.IsEmpty() ? nullptr : ValidCandidates[FMath::RandHelper(ValidCandidates.Num())];
}

bool UCombatTargetingComponent::IsValidHostileTarget(const AActor* Candidate) const
{
	const AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !IsValid(Candidate) || Candidate == Owner || Candidate->IsHidden())
	{
		return false;
	}

	const UCombatFactionComponent* OwnerFaction = Owner->FindComponentByClass<UCombatFactionComponent>();
	const UCombatFactionComponent* CandidateFaction = Candidate->FindComponentByClass<UCombatFactionComponent>();
	const UHealthComponent* CandidateHealth = Candidate->FindComponentByClass<UHealthComponent>();
	return OwnerFaction && CandidateFaction && CandidateHealth && !CandidateHealth->IsDead() && UCombatFactionComponent::AreHostile(OwnerFaction->GetFaction(), CandidateFaction->GetFaction());
}

bool UCombatTargetingComponent::HasLineOfSightTo(const AActor* Candidate) const
{
	const AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !IsValid(Candidate) || !GetWorld())
	{
		return false;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	Owner->GetActorEyesViewPoint(ViewLocation, ViewRotation);
	FVector TargetLocation;
	FVector TargetExtent;
	Candidate->GetActorBounds(true, TargetLocation, TargetExtent);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CombatTargetVisibility), true, Owner);
	QueryParams.AddIgnoredActor(Owner);
	FHitResult Hit;
	const bool bBlocked = GetWorld()->LineTraceSingleByChannel(Hit, ViewLocation, TargetLocation, ECC_Visibility, QueryParams);
	return !bBlocked || Hit.GetActor() == Candidate;
}
