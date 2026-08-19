#include "Gameplay/Characters/EnemyCombatCharacter.h"

#include "Gameplay/AI/EnemyAIController.h"
#include "Gameplay/AI/EnemyAILODComponent.h"
#include "Gameplay/AI/EnemyBehaviorStateComponent.h"
#include "Gameplay/AI/EnemySimpleMovementComponent.h"
#include "Gameplay/Combat/CombatAttackComponent.h"
#include "Gameplay/Combat/CombatFactionComponent.h"
#include "Gameplay/Combat/HealthComponent.h"

AEnemyCombatCharacter::AEnemyCombatCharacter()
{
	FactionComponent->SetFaction(ECombatFaction::Enemy);
	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AILODComponent = CreateDefaultSubobject<UEnemyAILODComponent>(TEXT("AILODComponent"));
	SimpleMovementComponent = CreateDefaultSubobject<UEnemySimpleMovementComponent>(TEXT("SimpleMovementComponent"));
	BehaviorStateComponent = CreateDefaultSubobject<UEnemyBehaviorStateComponent>(TEXT("BehaviorStateComponent"));
	AttackComponent = CreateDefaultSubobject<UCombatAttackComponent>(TEXT("AttackComponent"));
}

void AEnemyCombatCharacter::BeginPlay()
{
	Super::BeginPlay();
	SimpleMovementComponent->OnTargetReached.AddDynamic(this, &AEnemyCombatCharacter::HandleObjectiveReached);
}

void AEnemyCombatCharacter::SetObjectiveTarget(AActor* NewObjectiveTarget)
{
	ObjectiveTarget = NewObjectiveTarget;
	SimpleMovementComponent->SetMoveTargetActor(NewObjectiveTarget);
	AttackComponent->SetAttackTarget(NewObjectiveTarget);
	AttackComponent->SetAttackEnabled(false);
	BehaviorStateComponent->SetBehaviorState(TEXT("Advance"));
}

void AEnemyCombatCharacter::SetRetreatTargetLocation(const FVector RetreatLocation)
{
	ObjectiveTarget = nullptr;
	SimpleMovementComponent->SetMoveTargetLocation(RetreatLocation);
	AttackComponent->SetAttackEnabled(false);
	BehaviorStateComponent->SetBehaviorState(TEXT("Retreat"));
}

void AEnemyCombatCharacter::HandleObjectiveReached(AActor* ReachedEnemy)
{
	if (ObjectiveTarget && ReachedEnemy == this)
	{
		BehaviorStateComponent->SetBehaviorState(TEXT("Assault"));
		AttackComponent->SetAttackEnabled(true);
	}
}

void AEnemyCombatCharacter::OnAcquiredFromPool_Implementation()
{
	HealthComponent->SetCurrentHealth(HealthComponent->GetMaxHealth());
	AILODComponent->SetComponentTickEnabled(true);
	AILODComponent->RefreshLOD();
}

void AEnemyCombatCharacter::OnReleasedToPool_Implementation()
{
	SimpleMovementComponent->SetSimpleMovementEnabled(false);
	AILODComponent->SetComponentTickEnabled(false);
}
