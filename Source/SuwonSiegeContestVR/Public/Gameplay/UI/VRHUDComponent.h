#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/UI/VRHUDTypes.h"
#include "VRHUDComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVRHUDStateChanged, FVRHUDState, State);

/**
 * Feature-neutral VR HUD state owner. Feature adapters publish text and scalar progress here;
 * presentation is handled by the player Pawn's world-space widget.
 */
UCLASS(ClassGroup=(VR), meta=(BlueprintSpawnableComponent))
class SUWONSIEGECONTESTVR_API UVRHUDComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVRHUDComponent();

	UFUNCTION(BlueprintCallable, Category="VR|UI")
	void SetObjective(FText Objective, FText Detail);

	UFUNCTION(BlueprintCallable, Category="VR|UI")
	void ClearObjective();

	UFUNCTION(BlueprintCallable, Category="VR|UI")
	void SetProgress(FText Label, int32 Current, int32 Total);

	UFUNCTION(BlueprintCallable, Category="VR|UI")
	void ClearProgress();

	UFUNCTION(BlueprintCallable, Category="VR|UI")
	void ShowPrompt(FText Prompt);

	UFUNCTION(BlueprintCallable, Category="VR|UI")
	void ClearPrompt();

	UFUNCTION(BlueprintCallable, Category="VR|UI", meta=(AdvancedDisplay="Duration"))
	void ShowNotification(FText Message, EVRHUDNotificationType Type = EVRHUDNotificationType::Info, float Duration = 3.0f);

	UFUNCTION(BlueprintCallable, Category="VR|UI")
	void ClearNotification();

	UFUNCTION(BlueprintCallable, Category="VR|UI")
	void ClearAll();

	UFUNCTION(BlueprintPure, Category="VR|UI")
	FVRHUDState GetHUDState() const { return HUDState; }

	UPROPERTY(BlueprintAssignable, Category="VR|UI")
	FOnVRHUDStateChanged OnHUDStateChanged;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void BroadcastState();

	UPROPERTY(Transient)
	FVRHUDState HUDState;

	FTimerHandle NotificationTimer;
};
