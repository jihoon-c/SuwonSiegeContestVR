#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ScenarioNarrationBridgeComponent.generated.h"

class UDataTable;
class UNarrationSequenceComponent;
class UScenarioManagerComponent;

/** Optional adapter; reuses NarrationSequenceComponent instead of introducing a second narration manager. */
UCLASS(ClassGroup = (Scenario), meta = (BlueprintSpawnableComponent))
class SUWONSIEGECONTESTVR_API UScenarioNarrationBridgeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UScenarioNarrationBridgeComponent();

	UFUNCTION(BlueprintCallable, Category = "Scenario|Narration")
	bool InitializeBridge();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Narration")
	TObjectPtr<UDataTable> NarrationTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Narration")
	bool bInitializeOnBeginPlay = true;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleNarrationRequested(FName NarrationID, FName InteractionID);

	UFUNCTION()
	void HandleNarrationSequenceFinished();

	bool ResolveNarrationSequence();
	void Unbind();

	UPROPERTY(Transient)
	TObjectPtr<UScenarioManagerComponent> ScenarioManager;

	UPROPERTY(Transient)
	TObjectPtr<UNarrationSequenceComponent> NarrationSequence;

	FName PendingInteractionID;
};
