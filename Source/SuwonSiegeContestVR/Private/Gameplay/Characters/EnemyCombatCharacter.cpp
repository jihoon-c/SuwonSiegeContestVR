#include "Gameplay/Characters/EnemyCombatCharacter.h"

#include "Gameplay/AI/CombatAIController.h"
#include "Gameplay/Combat/CombatAttackComponent.h"
#include "Gameplay/Combat/CombatFactionComponent.h"
#include "Gameplay/Combat/HealthComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimTypes.h"
#include "Animation/AnimSequenceBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

AEnemyCombatCharacter::AEnemyCombatCharacter()
{
	FactionComponent->SetFaction(ECombatFaction::Enemy);
	AIControllerClass = ACombatAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AttackComponent = CreateDefaultSubobject<UCombatAttackComponent>(TEXT("AttackComponent"));
}

void AEnemyCombatCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddUniqueDynamic(this, &AEnemyCombatCharacter::HandleHealthDeath);
	}
	if (bIgnoreLocomotionRootMotion)
	{
		if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
		{
			AnimInstance->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
		}
	}
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
	GetWorldTimerManager().ClearTimer(DeathPresentationTimerHandle);
	bDeathPresentationActive = false;
	SetActorEnableCollision(true);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Walking);
	}
	if (bIgnoreLocomotionRootMotion)
	{
		if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
		{
			AnimInstance->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
		}
	}
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
	GetWorldTimerManager().ClearTimer(DeathPresentationTimerHandle);
	bDeathPresentationActive = false;
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

void AEnemyCombatCharacter::HandleHealthDeath(UHealthComponent* DeadHealthComponent, const FCombatDamageSpec& KillingDamage)
{
	if (bDeathPresentationActive)
	{
		return;
	}
	bDeathPresentationActive = true;
	AttackComponent->SetAttackEnabled(false);
	SetAttacking(false);
	if (ACombatAIController* CombatController = Cast<ACombatAIController>(GetController()))
	{
		CombatController->StopCombatMovement();
		CombatController->SetCombatTarget(nullptr);
	}
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}
	SetActorEnableCollision(false);
	if (GetMesh() && DeathAnimation)
	{
		// Single-node playback does not depend on a Slot node being authored in either enemy AnimBP.
		GetMesh()->PlayAnimation(DeathAnimation, false);
	}
	GetWorldTimerManager().SetTimer(DeathPresentationTimerHandle, this,
		&AEnemyCombatCharacter::FinishDeathPresentation, FMath::Max(0.0f, DeathRemovalDelay), false);
}

void AEnemyCombatCharacter::FinishDeathPresentation()
{
	OnDeathPresentationFinished.Broadcast(this);
}
