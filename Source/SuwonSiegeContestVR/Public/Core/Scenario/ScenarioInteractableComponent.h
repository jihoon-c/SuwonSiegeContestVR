#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/Scenario/ScenarioTypes.h"
#include "ScenarioInteractableComponent.generated.h"

class UScenarioManagerComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScenarioTargetEvent, FName, TargetID, EScenarioInteractionType, InteractionType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnScenarioTargetProgress, FName, TargetID, EScenarioInteractionType, InteractionType, float, Progress);

/** Adds logical scenario identity and event-only reporting to a level Actor. */
UCLASS(ClassGroup = (Scenario), meta = (BlueprintSpawnableComponent))
class SUWONSIEGECONTESTVR_API UScenarioInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UScenarioInteractableComponent();

	UFUNCTION(BlueprintCallable, Category = "Scenario|Interaction")
	void SetInteractionEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Scenario|Interaction")
	bool IsInteractionEnabled() const { return bInteractionEnabled; }

	UFUNCTION(BlueprintPure, Category = "Scenario|Interaction")
	bool SupportsInteractionType(EScenarioInteractionType InteractionType) const;

	/** Checks the active Scenario without completing or failing it. */
	UFUNCTION(BlueprintPure, Category = "Scenario|Interaction")
	bool CanReportInteraction(EScenarioInteractionType InteractionType) const;

	UFUNCTION(BlueprintCallable, Category = "Scenario|Interaction")
	bool ReportInteractionStarted(EScenarioInteractionType InteractionType);

	UFUNCTION(BlueprintCallable, Category = "Scenario|Interaction")
	bool ReportInteractionProgress(EScenarioInteractionType InteractionType, float Progress);

	UFUNCTION(BlueprintCallable, Category = "Scenario|Interaction")
	bool ReportInteractionCompleted(EScenarioInteractionType InteractionType);

	UFUNCTION(BlueprintCallable, Category = "Scenario|Interaction")
	bool ReportInteractionFailed(EScenarioInteractionType InteractionType);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Interaction")
	FName TargetID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Interaction")
	TArray<EScenarioInteractionType> SupportedInteractionTypes;

	/** Reports completion/failure to the first active ScenarioManager in this world. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Interaction")
	bool bAutoReportToScenarioManager = true;

	UPROPERTY(BlueprintAssignable, Category = "Scenario|Interaction")
	FOnScenarioTargetEvent OnInteractionStarted;

	UPROPERTY(BlueprintAssignable, Category = "Scenario|Interaction")
	FOnScenarioTargetProgress OnInteractionProgress;

	UPROPERTY(BlueprintAssignable, Category = "Scenario|Interaction")
	FOnScenarioTargetEvent OnInteractionCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Scenario|Interaction")
	FOnScenarioTargetEvent OnInteractionFailed;

protected:
	UScenarioManagerComponent* FindScenarioManager() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Interaction")
	bool bInteractionEnabled = true;
};
