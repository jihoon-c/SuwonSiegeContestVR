#include "Gameplay/Characters/EnemyCombatCharacter.h"

#include "Gameplay/AI/CombatAIController.h"
#include "Gameplay/Combat/CombatAttackComponent.h"
#include "Gameplay/Combat/CombatFactionComponent.h"
#include "Gameplay/Combat/HealthComponent.h"

AEnemyCombatCharacter::AEnemyCombatCharacter()
{
	FactionComponent->SetFaction(ECombatFaction::Enemy);
	AIControllerClass = ACombatAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AttackComponent = CreateDefaultSubobject<UCombatAttackComponent>(TEXT("AttackComponent"));
}

void AEnemyCombatCharacter::SetObjectiveTarget(AActor* NewObjectiveTarget)
{
	ObjectiveTarget = NewObjectiveTarget;
	AttackComponent->SetAttackTarget(NewObjectiveTarget);
	AttackComponent->SetAttackEnabled(false);
	SetAttacking(false);
	if (ACombatAIController* CombatController = Cast<ACombatAIController>(GetController()))
	{
		CombatController->MoveToCombatActor(NewObjectiveTarget);
	}
}

void AEnemyCombatCharacter::SetRetreatTargetLocation(const FVector RetreatLocation)
{
	ObjectiveTarget = nullptr;
	AttackComponent->SetAttackEnabled(false);
	SetAttacking(false);
	if (ACombatAIController* CombatController = Cast<ACombatAIController>(GetController()))
	{
		CombatController->MoveToCombatLocation(RetreatLocation);
	}
}

void AEnemyCombatCharacter::OnAcquiredFromPool_Implementation()
{
	ObjectiveTarget = nullptr;
	HealthComponent->ResetHealth();
	AttackComponent->SetAttackEnabled(false);
	AttackComponent->SetAttackTarget(nullptr);
	SetAttacking(false);
	if (ACombatAIController* CombatController = Cast<ACombatAIController>(GetController()))
	{
		CombatController->StopCombatMovement();
		CombatController->SetCombatTarget(nullptr);
		CombatController->StartAssignedBehaviorTree();
	}
}

void AEnemyCombatCharacter::OnReleasedToPool_Implementation()
{
	ObjectiveTarget = nullptr;
	AttackComponent->SetAttackEnabled(false);
	AttackComponent->SetAttackTarget(nullptr);
	SetAttacking(false);
	if (ACombatAIController* CombatController = Cast<ACombatAIController>(GetController()))
	{
		CombatController->StopCombatMovement();
		CombatController->SetCombatTarget(nullptr);
	}
}
