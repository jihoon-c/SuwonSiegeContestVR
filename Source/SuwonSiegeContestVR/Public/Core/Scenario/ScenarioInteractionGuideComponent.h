#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/Scenario/ScenarioTypes.h"
#include "ScenarioInteractionGuideComponent.generated.h"

class AActor;
class UScenarioManagerComponent;
class UScenarioInteractableComponent;
class UUserWidget;
class UWidgetComponent;

/** Resolves the current TargetID and presents one input guide above that Actor. */
UCLASS(ClassGroup = (Scenario), meta = (BlueprintSpawnableComponent))
class SUWONSIEGECONTESTVR_API UScenarioInteractionGuideComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UScenarioInteractionGuideComponent();

	UFUNCTION(BlueprintCallable, Category = "Scenario|Guide")
	bool InitializeGuide();

	UFUNCTION(BlueprintCallable, Category = "Scenario|Guide")
	void HideGuide();

	/** Immediately retries target resolution and updates the active guide transform. */
	UFUNCTION(BlueprintCallable, Category = "Scenario|Guide")
	void RefreshGuide();

	UFUNCTION(BlueprintPure, Category = "Scenario|Guide")
	bool IsGuideVisible() const;

	static EScenarioGuideAction ResolveGuideAction(const FScenarioInteraction& Interaction);
	static FText GetDefaultActionLabel(EScenarioGuideAction Action);
	static FText GetDefaultInstruction(EScenarioGuideAction Action);
	static float CalculateDistanceAdjustedScale(
		float BaseScale, float Distance, float ReferenceDistance, float MaximumScale);
	static FRotator CalculateGuideFacingRotation(
		const FVector& GuideLocation, const FVector& ViewLocation);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Guide")
	bool bEnableInteractionGuides = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Guide", meta = (ClampMin = "0.0", Units = "cm"))
	float HeightOffset = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Guide")
	FVector2D DrawSize = FVector2D(440.0f, 150.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Guide", meta = (ClampMin = "0.001"))
	float WorldScale = 0.12f;

	/** Keeps distant observation guides readable while preserving World Space stereo rendering. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Guide")
	bool bScaleWithViewDistance = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Guide",
		meta = (EditCondition = "bScaleWithViewDistance", ClampMin = "1.0", Units = "cm"))
	float ScaleReferenceDistance = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Guide",
		meta = (EditCondition = "bScaleWithViewDistance", ClampMin = "0.001"))
	float MaximumWorldScale = 0.65f;

	/** Active guide only. 30Hz is smooth in VR while avoiding an unnecessary per-frame UI update. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Guide|Performance",
		meta = (ClampMin = "0.0", Units = "s"))
	float ActiveGuideUpdateInterval = 1.0f / 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Guide")
	TSubclassOf<UUserWidget> GuideWidgetClass;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

private:
	UFUNCTION()
	void HandleInteractionRequested(FScenarioInteraction Interaction);

	UFUNCTION()
	void HandleInteractionStateChanged(FScenarioInteraction Interaction, EScenarioInteractionState State);

	UFUNCTION()
	void HandleScenarioStateChanged(EScenarioState OldState, EScenarioState NewState);

	AActor* ResolveTargetActor(
		const FScenarioInteraction& Interaction,
		UScenarioInteractableComponent*& OutInteractor) const;
	bool ResolveViewLocation(FVector& OutViewLocation) const;
	bool TryShowActiveGuide();
	void EnsureWidgetComponent();
	void UpdateGuideTransform();
	void Unbind();

	UPROPERTY(Transient)
	TObjectPtr<UScenarioManagerComponent> ScenarioManager;

	UPROPERTY(Transient)
	TObjectPtr<UWidgetComponent> GuideWidgetComponent;

	UPROPERTY(Transient)
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(Transient)
	TObjectPtr<UScenarioInteractableComponent> TargetInteractor;

	FScenarioInteraction ActiveInteraction;
	FName ActiveInteractionID;
	bool bHasActiveInteraction = false;
};
