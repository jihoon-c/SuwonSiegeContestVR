#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyBehaviorStateComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyBehaviorStateChanged, FName, PreviousState, FName, NewState);

/** Feature-neutral state label bridge for simple LOD movement and close-range BT/StateTree logic. */
UCLASS(ClassGroup = (Gameplay), meta = (BlueprintSpawnableComponent))
class SUWONSIEGECONTESTVR_API UEnemyBehaviorStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyBehaviorStateComponent();

	UFUNCTION(BlueprintCallable, Category = "Gameplay|AI|Behavior")
	void SetBehaviorState(FName NewState);

	UFUNCTION(BlueprintPure, Category = "Gameplay|AI|Behavior")
	FName GetBehaviorState() const { return BehaviorState; }

	UPROPERTY(BlueprintAssignable, Category = "Gameplay|AI|Behavior")
	FOnEnemyBehaviorStateChanged OnBehaviorStateChanged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay|AI|Behavior")
	FName BehaviorState = TEXT("Advance");
};
