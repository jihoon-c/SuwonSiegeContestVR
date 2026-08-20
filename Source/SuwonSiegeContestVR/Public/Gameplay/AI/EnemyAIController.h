#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

class UBehaviorTree;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyHighDetailAIChanged, bool, bEnabled);

/** Runs an optional Behavior Tree only for nearby enemies; Blueprints can use the event to start a StateTree instead. */
UCLASS(Blueprintable)
class SUWONSIEGECONTESTVR_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

	UFUNCTION(BlueprintCallable, Category = "Gameplay|AI")
	void SetHighDetailAIEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Gameplay|AI")
	bool IsHighDetailAIEnabled() const { return bHighDetailAIEnabled; }

	/** True when a Behavior Tree is assigned or a Blueprint has bound StateTree startup/shutdown to the LOD event. */
	UFUNCTION(BlueprintPure, Category = "Gameplay|AI")
	bool HasHighDetailAIImplementation() const;

	UPROPERTY(BlueprintAssignable, Category = "Gameplay|AI")
	FOnEnemyHighDetailAIChanged OnHighDetailAIChanged;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|AI")
	TObjectPtr<UBehaviorTree> HighDetailBehaviorTree;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gameplay|AI")
	bool bHighDetailAIEnabled = false;
};
