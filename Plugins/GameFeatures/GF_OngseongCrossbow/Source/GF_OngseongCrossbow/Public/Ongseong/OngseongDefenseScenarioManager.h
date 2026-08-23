#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Combat/CombatTypes.h"
#include "OngseongDefenseScenarioManager.generated.h"

class AOngseongEnemyWaveManager;
class AOngseongGateActor;
class AOngseongRamActor;
class UHealthComponent;
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
	UFUNCTION(BlueprintPure, Category="Ongseong|Scenario|Wave")
	int32 GetCurrentWaveNumber() const { return CurrentWaveNumber; }
	UFUNCTION(BlueprintPure, Category="Ongseong|Scenario|Wave")
	bool ShouldRepeatWavesDuringDefense() const { return bRepeatWavesDuringDefense; }
	UFUNCTION(BlueprintPure, Category="Ongseong|Scenario|Wave")
	float GetInterWaveDelay() const { return InterWaveDelay; }
	UFUNCTION(BlueprintPure, Category="Ongseong|Scenario|Wave")
	bool IsNextWavePending() const;
	UFUNCTION(BlueprintPure, Category="Ongseong|Scenario|Wave")
	AOngseongRamActor* GetActiveRam() const { return ActiveRam; }

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
	void TryScheduleNextWave();
	UFUNCTION()
	void StartNextWave();

	UFUNCTION()
	void HandleGateDestroyed();
	UFUNCTION()
	void HandleAllEnemiesRetreated();
	UFUNCTION()
	void HandleWaveDefeated(int32 DefeatedEnemies);
	UFUNCTION()
	void HandleRamDefeated(UHealthComponent* DeadHealth, const FCombatDamageSpec& KillingDamage);

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
	/** Repeats the configured swordsman/archer group until the defense timer ends. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Scenario|Wave")
	bool bRepeatWavesDuringDefense = true;
	/** Breathing room between clearing one Wave and spawning the next one. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Scenario|Wave", meta=(ClampMin="0.1"))
	float InterWaveDelay = 3.0f;
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
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Ongseong|Scenario|Wave")
	int32 CurrentWaveNumber = 0;
	bool bCurrentInfantryWaveDefeated = false;
	bool bCurrentRamDefeated = false;
	float RemainingDefenseTime = 0.0f;
	FTimerHandle DefenseTimerHandle;
	FTimerHandle AutoRetryTimerHandle;
	FTimerHandle NextWaveTimerHandle;
};
