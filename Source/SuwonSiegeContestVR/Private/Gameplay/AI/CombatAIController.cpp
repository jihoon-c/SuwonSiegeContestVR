#include "Gameplay/AI/CombatAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EngineUtils.h"
#include "Gameplay/Characters/CombatCharacter.h"
#include "Gameplay/Combat/CombatFactionComponent.h"
#include "Gameplay/Combat/HealthComponent.h"
#include "Navigation/PathFollowingComponent.h"

ACombatAIController::ACombatAIController()
{
	bStartAILogicOnPossess = true;
}

void ACombatAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	StartAssignedBehaviorTree();
}

void ACombatAIController::OnUnPossess()
{
	SetAttacking(false);
	CombatTarget = nullptr;
	Super::OnUnPossess();
}

bool ACombatAIController::StartAssignedBehaviorTree()
{
	return BehaviorTreeAsset && RunBehaviorTree(BehaviorTreeAsset);
}

void ACombatAIController::SetBehaviorTreeAsset(UBehaviorTree* NewBehaviorTree)
{
	BehaviorTreeAsset = NewBehaviorTree;
	if (GetPawn())
	{
		StartAssignedBehaviorTree();
	}
}

void ACombatAIController::SetCombatTarget(AActor* NewTarget)
{
	CombatTarget = IsValid(NewTarget) ? NewTarget : nullptr;
	if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
	{
		BlackboardComp->SetValueAsObject(TargetActorKey, CombatTarget);
	}
}

AActor* ACombatAIController::GetCombatTarget() const
{
	if (const UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
	{
		if (AActor* BlackboardTarget = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey)))
		{
			return BlackboardTarget;
		}
	}
	return CombatTarget;
}

AActor* ACombatAIController::FindRandomVisibleHostile(const float SearchRadius) const
{
	TArray<AActor*> Candidates;
	const APawn* ControlledPawn = GetPawn();
	const UWorld* World = GetWorld();
	const UCombatFactionComponent* PawnFaction = ControlledPawn ? ControlledPawn->FindComponentByClass<UCombatFactionComponent>() : nullptr;
	if (!ControlledPawn || !World || !PawnFaction)
	{
		return nullptr;
	}

	const float RadiusSquared = FMath::Square(FMath::Max(0.0f, SearchRadius));
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Candidate = *It;
		const UCombatFactionComponent* CandidateFaction = IsValid(Candidate) ? Candidate->FindComponentByClass<UCombatFactionComponent>() : nullptr;
		const UHealthComponent* CandidateHealth = IsValid(Candidate) ? Candidate->FindComponentByClass<UHealthComponent>() : nullptr;
		if (Candidate == ControlledPawn || !CandidateFaction || !CandidateHealth || CandidateHealth->IsDead())
		{
			continue;
		}
		if (!UCombatFactionComponent::AreHostile(PawnFaction->GetFaction(), CandidateFaction->GetFaction()))
		{
			continue;
		}
		if (FVector::DistSquared(ControlledPawn->GetActorLocation(), Candidate->GetActorLocation()) > RadiusSquared)
		{
			continue;
		}
		if (LineOfSightTo(Candidate))
		{
			Candidates.Add(Candidate);
		}
	}
	return Candidates.IsEmpty() ? nullptr : Candidates[FMath::RandHelper(Candidates.Num())];
}

bool ACombatAIController::MoveToCombatActor(AActor* TargetActor, const float AcceptanceRadius)
{
	if (!IsValid(TargetActor))
	{
		return false;
	}
	SetCombatTarget(TargetActor);
	return MoveToActor(TargetActor, FMath::Max(0.0f, AcceptanceRadius), true, true, true, nullptr, true) != EPathFollowingRequestResult::Failed;
}

bool ACombatAIController::MoveToCombatLocation(const FVector TargetLocation, const float AcceptanceRadius)
{
	SetCombatTarget(nullptr);
	return MoveToLocation(TargetLocation, FMath::Max(0.0f, AcceptanceRadius), true, true, true, true, nullptr, true) != EPathFollowingRequestResult::Failed;
}

void ACombatAIController::StopCombatMovement()
{
	StopMovement();
}

void ACombatAIController::SetAttacking(const bool bNewAttacking)
{
	if (ACombatCharacter* CombatPawn = Cast<ACombatCharacter>(GetPawn()))
	{
		CombatPawn->SetAttacking(bNewAttacking);
	}
	if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
	{
		BlackboardComp->SetValueAsBool(IsAttackingKey, bNewAttacking);
	}
}

void ACombatAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);
	if (Result.IsSuccess())
	{
		OnMoveTargetReached.Broadcast(GetPawn());
	}
}
