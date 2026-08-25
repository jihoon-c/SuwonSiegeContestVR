#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CombatAIController.generated.h"

class UBehaviorTree;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatAIMoveTargetReached, APawn*, ControlledPawn);

/** Shared controller for allied and enemy combat pawns. Feature modules supply optional Behavior Trees. */
UCLASS(Blueprintable)
class SUWONSIEGECONTESTVR_API ACombatAIController : public AAIController
{
	GENERATED_BODY()

public:
	ACombatAIController();

	UFUNCTION(BlueprintCallable, Category = "Gameplay|AI")
	bool StartAssignedBehaviorTree();

	UFUNCTION(BlueprintCallable, Category = "Gameplay|AI")
	void SetBehaviorTreeAsset(UBehaviorTree* NewBehaviorTree);

	UFUNCTION(BlueprintCallable, Category = "Gameplay|AI|Targeting")
	void SetCombatTarget(AActor* NewTarget);

	UFUNCTION(BlueprintPure, Category = "Gameplay|AI|Targeting")
	AActor* GetCombatTarget() const;

	/** Returns a random living hostile that is both in range and visible to this controller. */
	UFUNCTION(BlueprintCallable, Category = "Gameplay|AI|Targeting")
	AActor* FindRandomVisibleHostile(float SearchRadius) const;

	UFUNCTION(BlueprintCallable, Category = "Gameplay|AI|Movement")
	bool MoveToCombatActor(AActor* TargetActor, float AcceptanceRadius = 100.0f);

	UFUNCTION(BlueprintCallable, Category = "Gameplay|AI|Movement")
	bool MoveToCombatLocation(FVector TargetLocation, float AcceptanceRadius = 100.0f);

	UFUNCTION(BlueprintCallable, Category = "Gameplay|AI|Movement")
	void StopCombatMovement();

	UFUNCTION(BlueprintCallable, Category = "Gameplay|AI|Animation")
	void SetAttacking(bool bNewAttacking);

	UPROPERTY(BlueprintAssignable, Category = "Gameplay|AI|Movement")
	FOnCombatAIMoveTargetReached OnMoveTargetReached;

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|AI|Blackboard")
	FName TargetActorKey = TEXT("TargetActor");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|AI|Blackboard")
	FName IsAttackingKey = TEXT("IsAttacking");

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gameplay|AI|Targeting")
	TObjectPtr<AActor> CombatTarget;
};
