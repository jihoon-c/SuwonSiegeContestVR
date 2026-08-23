#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OngseongDefenseScenarioManager.generated.h"

class AOngseongEnemyWaveManager;
class AOngseongGateActor;
class AOngseongRamActor;
class UOngseongNarrationComponent;
class UVRHUDComponent;

UENUM(BlueprintType)
enum class EOngseongDefenseState : uint8
{
	Idle,
	Defending,
	Succeeded,
	Failed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOngseongDefenseStateChanged, EOngseongDefenseState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOngseongDefenseTimeChanged, int32, RemainingSeconds);

/** Owns the defense timer, completed ram assault, terminal states, retreat and Experience handoff. */
UCLASS(Blueprintable)
class GF_ONGSEONGCROSSBOW_API AOngseongDefenseScenarioManager : public AActor
{
	GENERATED_BODY()

public:
	AOngseongDefenseScenarioManager();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category="Ongseong|Scenario")
	bool StartDefense();
	UFUNCTION(BlueprintCallable, Category="Ongseong|Scenario")
	void RetryDefense();

	UFUNCTION(BlueprintCallable, Category="Ongseong|Scenario")
	void SetGateActor(AOngseongGateActor* NewGateActor) { GateActor = NewGateActor; }
	UFUNCTION(BlueprintCallable, Category="Ongseong|Scenario")
	void SetWaveManager(AOngseongEnemyWaveManager* NewWaveManager) { WaveManager = NewWaveManager; }

	UFUNCTION(BlueprintPure, Category="Ongseong|Scenario")
	EOngseongDefenseState GetDefenseState() const { return DefenseState; }
	UFUNCTION(BlueprintPure, Category="Ongseong|Scenario")
	float GetRemainingDefenseTime() const { return RemainingDefenseTime; }

	UPROPERTY(BlueprintAssignable, Category="Ongseong|Scenario")
	FOnOngseongDefenseStateChanged OnDefenseStateChanged;
	UPROPERTY(BlueprintAssignable, Category="Ongseong|Scenario")
	FOnOngseongDefenseTimeChanged OnDefenseTimeChanged;
protected:
	void TickDefenseTimer();
	bool SpawnAndActivateRam();
	void SucceedDefense();
	void FinishSuccessfulRetreat();
	void SetDefenseState(EOngseongDefenseState NewState);
	void UpdateHUDTime();
	void HandleAutoRetry();

	UFUNCTION()
	void HandleGateDestroyed();
	UFUNCTION()
	void HandleAllEnemiesRetreated();

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Ongseong|Scenario")
	TObjectPtr<AOngseongGateActor> GateActor;
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Ongseong|Scenario")
	TObjectPtr<AOngseongEnemyWaveManager> WaveManager;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Scenario")
	TSubclassOf<AOngseongRamActor> RamClass;
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Ongseong|Scenario")
	TObjectPtr<AActor> RamSpawnPoint;
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Ongseong|Scenario")
	TObjectPtr<AActor> RetreatPoint;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Scenario", meta=(ClampMin="1.0"))
	float DefenseDuration = 180.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Scenario")
	bool bAutoStart = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Scenario")
	bool bReturnToMainOnSuccess = true;
	/** Educational fallback: restart cleanly after showing the failure reason. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Scenario")
	bool bAutoRetryOnFailure = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Scenario", meta=(ClampMin="1.0"))
	float AutoRetryDelay = 6.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ongseong|Scenario")
	TObjectPtr<UOngseongNarrationComponent> Narration;

	UPROPERTY(Transient)
	TObjectPtr<UVRHUDComponent> VRHUD;
	UPROPERTY(Transient)
	TObjectPtr<AOngseongRamActor> ActiveRam;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Ongseong|Scenario")
	EOngseongDefenseState DefenseState = EOngseongDefenseState::Idle;
	float RemainingDefenseTime = 0.0f;
	FTimerHandle DefenseTimerHandle;
	FTimerHandle AutoRetryTimerHandle;
};
