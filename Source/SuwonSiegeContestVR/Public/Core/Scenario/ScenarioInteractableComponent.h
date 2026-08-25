#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/Scenario/ScenarioTypes.h"
#include "ScenarioInteractableComponent.generated.h"

class UScenarioManagerComponent;
class UArrowComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScenarioTargetEvent, FName, TargetID, EScenarioInteractionType, InteractionType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnScenarioTargetProgress, FName, TargetID, EScenarioInteractionType, InteractionType, float, Progress);

/** Adds logical scenario identity and event-only reporting to a level Actor. */
UCLASS(ClassGroup = (Scenario), meta = (BlueprintSpawnableComponent))
class SUWONSIEGECONTESTVR_API UScenarioInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UScenarioInteractableComponent();

	/** Bounds-top guide anchor used both by the editor preview and runtime Widget. */
	UFUNCTION(BlueprintPure, Category = "Scenario|Guide")
	FVector GetGuideAnchorWorldLocation() const;

	/** Moves the guide pivot in world space and stores the result in GuideAnchorOffset. */
	UFUNCTION(BlueprintCallable, Category = "Scenario|Guide")
	void SetGuideAnchorWorldLocation(const FVector& WorldLocation);

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

	/** World-space adjustment from this Actor's bounds top to the guide pivot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Guide", meta = (Units = "cm"))
	FVector GuideAnchorOffset = FVector(0.0f, 0.0f, 10.0f);

	/** Shows a yellow editor-only arrow where the runtime guide pivot will appear. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Guide")
	bool bShowGuideAnchorInEditor = true;

	UPROPERTY(BlueprintAssignable, Category = "Scenario|Interaction")
	FOnScenarioTargetEvent OnInteractionStarted;

	UPROPERTY(BlueprintAssignable, Category = "Scenario|Interaction")
	FOnScenarioTargetProgress OnInteractionProgress;

	UPROPERTY(BlueprintAssignable, Category = "Scenario|Interaction")
	FOnScenarioTargetEvent OnInteractionCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Scenario|Interaction")
	FOnScenarioTargetEvent OnInteractionFailed;

protected:
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UScenarioManagerComponent* FindScenarioManager() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Interaction")
	bool bInteractionEnabled = true;

private:
#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient, DuplicateTransient, TextExportTransient)
	TObjectPtr<UArrowComponent> EditorGuideAnchor;
#endif

	void RefreshEditorGuideAnchor();
};
