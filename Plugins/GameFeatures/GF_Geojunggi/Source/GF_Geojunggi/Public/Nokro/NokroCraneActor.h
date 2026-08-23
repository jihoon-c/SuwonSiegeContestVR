#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NokroCraneActor.generated.h"

class UNokroControlGripComponent;
class USceneComponent;
class UScenarioInteractableComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNokroSimpleEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNokroPlacementRequested, FTransform, StoneTransform);

/** Playable pulley crane: crank controls height, thumbstick controls yaw, two triggers request placement. */
UCLASS(Blueprintable)
class GF_GEOJUNGGI_API ANokroCraneActor : public AActor
{
	GENERATED_BODY()

public:
	ANokroCraneActor();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category="Nokro|Control")
	void ApplyHandleRotation(float DeltaDegrees);

	UFUNCTION(BlueprintCallable, Category="Nokro|Control")
	void SetBoomYaw(float NewYaw);

	UFUNCTION(BlueprintCallable, Category="Nokro|Control")
	void RequestStonePlacement();

	UFUNCTION(BlueprintCallable, Category="Nokro|Control")
	void ResetCarriedStone(bool bShowStone = true);

	UFUNCTION(BlueprintCallable, Category="Nokro|Control")
	void SetPlacementEnabled(bool bEnabled) { bPlacementEnabled = bEnabled; }

	UFUNCTION(BlueprintCallable, Category="Nokro|Control")
	void NotifyHandleGrabbed();

	/** Captures the right-stick axis at higher input priority while any hand operates the crane. */
	void SetJoystickInputCapture(bool bCapture);

	UFUNCTION(BlueprintPure, Category="Nokro|Control")
	FTransform GetCarriedStoneTransform() const;

	UFUNCTION(BlueprintPure, Category="Nokro|Control")
	float GetCurrentHeight() const { return CurrentHeight; }

	UFUNCTION(BlueprintPure, Category="Nokro|Control")
	float GetCurrentBoomYaw() const;

	UPROPERTY(BlueprintAssignable, Category="Nokro|Events")
	FOnNokroSimpleEvent OnHandleGrabbed;
	UPROPERTY(BlueprintAssignable, Category="Nokro|Events")
	FOnNokroSimpleEvent OnHeightAdjusted;
	UPROPERTY(BlueprintAssignable, Category="Nokro|Events")
	FOnNokroSimpleEvent OnDirectionAdjusted;
	UPROPERTY(BlueprintAssignable, Category="Nokro|Events")
	FOnNokroPlacementRequested OnPlacementRequested;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Nokro|Components")
	TObjectPtr<UNokroControlGripComponent> ControlGrip;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Nokro|Components")
	TObjectPtr<USceneComponent> BoomPivot;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Nokro|Components")
	TObjectPtr<USceneComponent> HandlePivot;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Nokro|Components")
	TObjectPtr<USceneComponent> StoneRoot;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Nokro|Components")
	TObjectPtr<UStaticMeshComponent> CarriedStone;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Nokro|Components")
	TObjectPtr<UStaticMeshComponent> RopeVisual;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Nokro|Scenario")
	TObjectPtr<UScenarioInteractableComponent> ScenarioInteraction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Nokro|Control", meta=(ClampMin="0.1"))
	float HeightPerHandleDegree = 1.1f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Nokro|Control")
	float MinimumHeight = 80.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Nokro|Control")
	float MaximumHeight = 500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Nokro|Control")
	float InitialHeight = 80.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Nokro|Control", meta=(ClampMin="1.0"))
	float BoomLength = 400.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Nokro|Control", meta=(ClampMin="1.0"))
	float RotationSpeedDegrees = 45.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Nokro|Control", meta=(ClampMin="0.0", ClampMax="1.0"))
	float JoystickDeadZone = 0.2f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Nokro|Control")
	bool bPlacementEnabled = false;

private:
	float ReadRotationJoystick() const;
	void UpdateStoneVisual();
	void BuildPrimitiveVisuals();

	float CurrentHeight = 80.0f;
	bool bReportedHeightAdjustment = false;
	bool bReportedDirectionAdjustment = false;
};
