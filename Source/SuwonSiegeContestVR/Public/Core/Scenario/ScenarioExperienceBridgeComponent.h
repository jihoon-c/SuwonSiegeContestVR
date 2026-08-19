#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ScenarioExperienceBridgeComponent.generated.h"

class UExperienceDefinition;
class UScenarioManagerComponent;

/** Connects a level-local Scenario completion event to the cross-level ExperienceSubsystem. */
UCLASS(ClassGroup = (Scenario), meta = (BlueprintSpawnableComponent))
class SUWONSIEGECONTESTVR_API UScenarioExperienceBridgeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UScenarioExperienceBridgeComponent();

	UFUNCTION(BlueprintCallable, Category = "Scenario|Experience")
	bool InitializeBridge();

	/** Restores and consumes the session checkpoint for the configured Scenario. */
	UFUNCTION(BlueprintCallable, Category = "Scenario|Experience")
	bool RestoreScenarioCheckpoint();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Experience")
	TObjectPtr<UExperienceDefinition> ExperienceDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Experience")
	bool bInitializeOnBeginPlay = true;

	/** Supports opening an experience level directly in PIE without traveling through the subsystem first. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Experience")
	bool bActivateExperienceWhenOpenedDirectly = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Experience")
	bool bCompleteExperienceOnScenarioFinished = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Experience")
	bool bRestoreScenarioCheckpoint = true;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleScenarioFinished();

	void Unbind();

	UPROPERTY(Transient)
	TObjectPtr<UScenarioManagerComponent> ScenarioManager;
};
