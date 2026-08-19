#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/AI/EnemyAITypes.h"
#include "EnemyAILODComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyAILODChanged, EEnemyAILODLevel, PreviousLevel, EEnemyAILODLevel, NewLevel);

/** Switches an enemy between cheap far movement and close-range high-detail AI/rendering. */
UCLASS(ClassGroup = (Gameplay), meta = (BlueprintSpawnableComponent))
class SUWONSIEGECONTESTVR_API UEnemyAILODComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyAILODComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Gameplay|AI|LOD")
	void SetLODReferenceActor(AActor* NewReferenceActor);

	UFUNCTION(BlueprintCallable, Category = "Gameplay|AI|LOD")
	void RefreshLOD();

	UFUNCTION(BlueprintPure, Category = "Gameplay|AI|LOD")
	EEnemyAILODLevel GetCurrentLODLevel() const { return CurrentLODLevel; }

	UPROPERTY(BlueprintAssignable, Category = "Gameplay|AI|LOD")
	FOnEnemyAILODChanged OnLODChanged;

protected:
	void ApplyLODLevel(EEnemyAILODLevel NewLevel, bool bForce = false);
	AActor* ResolveReferenceActor() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay|AI|LOD")
	TObjectPtr<AActor> LODReferenceActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay|AI|LOD", meta = (ClampMin = "0.0"))
	float EnterNearDistance = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay|AI|LOD", meta = (ClampMin = "0.0"))
	float ExitNearDistance = 2200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay|AI|LOD", meta = (ClampMin = "0.05"))
	float EvaluationInterval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay|AI|LOD")
	bool bHideVisualsWhenFar = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay|AI|LOD", meta = (ClampMin = "0.0"))
	float FarActorTickInterval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay|AI|LOD", meta = (ClampMin = "0.0"))
	float NearActorTickInterval = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gameplay|AI|LOD")
	EEnemyAILODLevel CurrentLODLevel = EEnemyAILODLevel::Far;
};
