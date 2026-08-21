#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GongsimdonEnemyGroupActor.generated.h"

class AEnemySoldierActor;
class USceneComponent;
class UScenarioInteractableComponent;
class UScenarioObservationComponent;
class USplineComponent;

UENUM(BlueprintType)
enum class EGongsimdonEnemyGroupState : uint8
{
	Hidden,
	Approaching,
	Holding,
	Retreating,
	Escaped
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnGongsimdonEnemyGroupStateChanged,
	EGongsimdonEnemyGroupState, OldState,
	EGongsimdonEnemyGroupState, NewState);

/** Owns a small authored enemy formation and moves it over approach/retreat splines. */
UCLASS(Blueprintable)
class GF_GONGSIMDON_API AGongsimdonEnemyGroupActor : public AActor
{
	GENERATED_BODY()

public:
	AGongsimdonEnemyGroupActor();
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable, Category = "Gongsimdon|Enemy Group")
	bool SpawnEnemies();

	UFUNCTION(BlueprintCallable, Category = "Gongsimdon|Enemy Group")
	void BeginApproach();

	UFUNCTION(BlueprintCallable, Category = "Gongsimdon|Enemy Group")
	void StartRetreat();

	UFUNCTION(BlueprintCallable, Category = "Gongsimdon|Enemy Group")
	void HideGroup();

	UFUNCTION(BlueprintCallable, Category = "Gongsimdon|Enemy Group")
	void ActivateObservation();

	UFUNCTION(BlueprintCallable, Category = "Gongsimdon|Enemy Group")
	void DeactivateObservation(bool bReportFailure = false);

	UFUNCTION(BlueprintCallable, Category = "Gongsimdon|Enemy Group")
	void SetCombatArmed(bool bArmed);

	UFUNCTION(BlueprintPure, Category = "Gongsimdon|Enemy Group")
	EGongsimdonEnemyGroupState GetGroupState() const { return GroupState; }

	UFUNCTION(BlueprintPure, Category = "Gongsimdon|Enemy Group")
	int32 GetSpawnedEnemyCount() const { return SpawnedEnemies.Num(); }

	UFUNCTION(BlueprintPure, Category = "Gongsimdon|Enemy Group")
	AEnemySoldierActor* GetSpawnedEnemy(int32 Index) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gongsimdon|Enemy Group")
	TSubclassOf<AEnemySoldierActor> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gongsimdon|Enemy Group", meta = (ClampMin = "6", ClampMax = "8"))
	int32 EnemyCount = 7;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gongsimdon|Enemy Group", meta = (ClampMin = "1"))
	int32 FormationColumns = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gongsimdon|Enemy Group", meta = (ClampMin = "0.0"))
	float LateralSpacing = 140.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gongsimdon|Enemy Group", meta = (ClampMin = "0.0"))
	float RowSpacing = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gongsimdon|Enemy Group", meta = (ClampMin = "1.0"))
	float ApproachSpeed = 140.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gongsimdon|Enemy Group", meta = (ClampMin = "1.0"))
	float RetreatSpeed = 260.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gongsimdon|Scenario")
	FName ObservationTargetID = TEXT("OBS_ENEMY_GROUP");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gongsimdon|Scenario")
	FName CombatTargetID = TEXT("COMBAT_RETREATING");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gongsimdon|Scenario", meta = (ClampMin = "1"))
	int32 RequiredCombatHits = 1;

	UPROPERTY(BlueprintAssignable, Category = "Gongsimdon|Enemy Group")
	FOnGongsimdonEnemyGroupStateChanged OnGroupStateChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleCueRequested(FName InteractionID, FName TargetID);

	UFUNCTION()
	void HandleEnemyHealthChanged(
		AActor* OwnerActor,
		float CurrentHealth,
		float MaxHealth,
		float HealthDelta);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gongsimdon|Enemy Group")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gongsimdon|Enemy Group")
	TObjectPtr<USplineComponent> ApproachSpline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gongsimdon|Enemy Group")
	TObjectPtr<USplineComponent> RetreatSpline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gongsimdon|Scenario")
	TObjectPtr<USceneComponent> ObservationAnchor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gongsimdon|Scenario")
	TObjectPtr<UScenarioObservationComponent> Observation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gongsimdon|Scenario")
	TObjectPtr<UScenarioInteractableComponent> CombatInteraction;

private:
	void BindScenarioDirector();
	void SetGroupState(EGongsimdonEnemyGroupState NewState);
	void SetEnemiesActive(bool bActive);
	void UpdateFormation(USplineComponent* Spline, float LeaderDistance);
	float GetFormationTailLength() const;
	void DestroySpawnedEnemies();

	UPROPERTY(Transient)
	TArray<TObjectPtr<AEnemySoldierActor>> SpawnedEnemies;

	EGongsimdonEnemyGroupState GroupState = EGongsimdonEnemyGroupState::Hidden;
	float TravelDistance = 0.0f;
	int32 CombatHits = 0;
	bool bCombatArmed = false;
};
