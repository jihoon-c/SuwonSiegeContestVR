#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Ongseong/ChongtongInteractionTypes.h"
#include "Gameplay/Combat/CombatTypes.h"
#include "OngseongNarrationComponent.generated.h"

class AChongtongCannonActor;
class AOngseongEnemyWaveManager;
class UDataTable;
class UHealthComponent;
class UNarrationSequenceComponent;

/** A named gameplay event. Designers can subscribe without depending on the narration mapping. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnOngseongScenarioEvent, FName, EventName, AActor*, SourceActor);

USTRUCT(BlueprintType)
struct GF_ONGSEONGCROSSBOW_API FOngseongNarrationEventBinding
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Narration")
	FName EventName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Narration")
	FName NarrationRow;

	/** Suppresses repeated warnings such as the first gate-hit callout. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Narration")
	bool bPlayOnce = true;
};

/**
 * Feature-local adapter from extensible gameplay delegates to the shared narration player.
 * Events are queued so situational callouts never cut off an instructor line already playing.
 */
UCLASS(ClassGroup=(Ongseong), meta=(BlueprintSpawnableComponent))
class GF_ONGSEONGCROSSBOW_API UOngseongNarrationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOngseongNarrationComponent();

	UFUNCTION(BlueprintCallable, Category="Ongseong|Narration")
	void ReportScenarioEvent(FName EventName, AActor* SourceActor = nullptr);

	UFUNCTION(BlueprintCallable, Category="Ongseong|Narration")
	bool InitializeNarrationBindings();

	UPROPERTY(BlueprintAssignable, Category="Ongseong|Narration")
	FOnOngseongScenarioEvent OnScenarioEvent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Narration")
	TSoftObjectPtr<UDataTable> NarrationTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Narration")
	TArray<FOngseongNarrationEventBinding> EventBindings;

	/** Optional ally or gate actors whose health events should produce callouts. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Ongseong|Narration")
	TArray<TObjectPtr<AActor>> AlliedDefenseActors;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Ongseong|Narration")
	TObjectPtr<AActor> GateActor;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Ongseong|Narration")
	TObjectPtr<AOngseongEnemyWaveManager> WaveManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Narration")
	bool bPlayIntroduction = true;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void QueueNarration(FName RowName);
	void TryPlayNextNarration();
	void BindHealthActor(AActor* Actor, bool bIsGate);
	void UnbindSources();
	const FOngseongNarrationEventBinding* FindEventBinding(FName EventName) const;

	UFUNCTION()
	void HandleNarrationFinished();
	UFUNCTION()
	void HandleLoadingStateChanged(EChongtongLoadingState NewState, int32 CompletedShots);
	UFUNCTION()
	void HandleWaveStarted(int32 TotalEnemies);
	UFUNCTION()
	void HandleEnemySpawned(class AEnemyCombatCharacter* Enemy, int32 SpawnedEnemies, int32 TotalEnemies);
	UFUNCTION()
	void HandleAllEnemiesDefeated(int32 TotalEnemies);
	UFUNCTION()
	void HandleGateDamaged(UHealthComponent* HealthComponent, const FCombatDamageSpec& DamageSpec);
	UFUNCTION()
	void HandleGateDestroyed(UHealthComponent* HealthComponent, const FCombatDamageSpec& KillingDamage);
	UFUNCTION()
	void HandleAllyDamaged(UHealthComponent* HealthComponent, const FCombatDamageSpec& DamageSpec);

	UPROPERTY(Transient)
	TObjectPtr<AChongtongCannonActor> Cannon;
	UPROPERTY(Transient)
	TObjectPtr<UNarrationSequenceComponent> NarrationSequence;
	UPROPERTY(Transient)
	TArray<TObjectPtr<UHealthComponent>> BoundHealthComponents;

	TArray<FName> PendingRows;
	TSet<FName> PlayedOnceEvents;
	bool bOwnsCurrentNarration = false;
};
