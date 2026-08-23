#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Combat/CombatTypes.h"
#include "OngseongDefenseScenarioManager.generated.h"

class AActorPool;
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

/**
 * Owns the ram assault, terminal states, retreat and Experience handoff.
 * The experience is cleared by destroying the enemy ram; losing the gate fails it.
 */
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
	UFUNCTION(BlueprintCallable, Category="Ongseong|Scenario")
	void SetRamPool(AActorPool* NewRamPool) { RamPool = NewRamPool; }

	UFUNCTION(BlueprintPure, Category="Ongseong|Scenario")
	EOngseongDefenseState GetDefenseState() const { return DefenseState; }
	UFUNCTION(BlueprintPure, Category="Ongseong|Scenario")
	float GetRemainingDefenseTime() const { return RemainingDefenseTime; }
	UFUNCTION(BlueprintPure, Category="Ongseong|Scenario|Ram")
	AOngseongRamActor* GetActiveRam() const { return ActiveRam; }
	/** The ram is a single objective target. It is never replaced once destroyed. */
	UFUNCTION(BlueprintPure, Category="Ongseong|Scenario|Ram")
	bool IsRamDestroyed() const { return bRamDestroyed; }
	UFUNCTION(BlueprintPure, Category="Ongseong|Scenario")
	bool IsDefenseTimeLimited() const { return bUseDefenseTimeLimit; }
	/** Cumulative enemies defeated during the current defense attempt. */
	UFUNCTION(BlueprintPure, Category="Ongseong|Scenario")
	int32 GetTotalDefeatedEnemies() const;

	UPROPERTY(BlueprintAssignable, Category="Ongseong|Scenario")
	FOnOngseongDefenseStateChanged OnDefenseStateChanged;
	UPROPERTY(BlueprintAssignable, Category="Ongseong|Scenario")
	FOnOngseongDefenseTimeChanged OnDefenseTimeChanged;
protected:
	void TickDefenseTimer();
	bool SpawnAndActivateRam();
	void ReleaseActiveRam();
	void SucceedDefense();
	void FailDefense(FName NarrationEvent, const FText& Headline, const FText& Detail, const FText& Notification);
	void FinishSuccessfulRetreat();
	void SetDefenseState(EOngseongDefenseState NewState);
	void UpdateHUDTime();
	void HandleAutoRetry();

	UFUNCTION()
	void HandleGateDestroyed();
	UFUNCTION()
	void HandleAllEnemiesRetreated();
	UFUNCTION()
	void HandleRamDefeated(UHealthComponent* DeadHealth, const FCombatDamageSpec& KillingDamage);

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Ongseong|Scenario")
	TObjectPtr<AOngseongGateActor> GateActor;
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Ongseong|Scenario")
	TObjectPtr<AOngseongEnemyWaveManager> WaveManager;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Scenario|Ram")
	TSubclassOf<AOngseongRamActor> RamClass;
	/** Optional. When set, rams are acquired and released instead of spawned and destroyed. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Ongseong|Scenario|Ram")
	TObjectPtr<AActorPool> RamPool;
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Ongseong|Scenario")
	TObjectPtr<AActor> RamSpawnPoint;
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Ongseong|Scenario")
	TObjectPtr<AActor> RetreatPoint;
	/** Optional time limit. The experience is normally cleared by destroying the ram, not by surviving. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Scenario")
	bool bUseDefenseTimeLimit = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Scenario", meta=(ClampMin="1.0", EditCondition="bUseDefenseTimeLimit"))
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
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Ongseong|Scenario|Ram")
	bool bRamDestroyed = false;
	float RemainingDefenseTime = 0.0f;
	FTimerHandle DefenseTimerHandle;
	FTimerHandle AutoRetryTimerHandle;
};
