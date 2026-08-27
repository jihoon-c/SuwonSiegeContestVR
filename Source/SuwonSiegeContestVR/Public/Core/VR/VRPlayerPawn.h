#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "VRPlayerPawn.generated.h"

class UAudioComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UMotionControllerComponent;
class UNarrationSequenceComponent;
class USceneComponent;
class USkeletalMeshComponent;
class USubtitleWidget;
class UUserWidget;
class UWidgetComponent;
class UWidgetInteractionComponent;

/** Project-owned VR Pawn foundation. Blueprint can add locomotion and the template grab behavior. */
UCLASS(Blueprintable)
class SUWONSIEGECONTESTVR_API AVRPlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	AVRPlayerPawn();
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** Temporarily anchors the HMD camera to a mounted interaction such as the Chongtong. */
	UFUNCTION(BlueprintCallable, Category = "VR|Mounted Interaction")
	void EnterMountedInteraction(USceneComponent* CameraAnchor);

	UFUNCTION(BlueprintCallable, Category = "VR|Mounted Interaction")
	void ExitMountedInteraction(USceneComponent* CameraAnchor = nullptr);

	/**
	 * Locks or unlocks locomotion for an experience that keeps the player on one spot.
	 * Turning, grabbing and mounted interactions stay available.
	 */
	UFUNCTION(BlueprintCallable, Category = "VR|Locomotion")
	void SetLocomotionEnabled(bool bMoveEnabled, bool bTeleportEnabled);

	UFUNCTION(BlueprintCallable, Category = "Narration")
	void DismissNarrationWidget(bool bContinueSequence = true);

	UFUNCTION(BlueprintPure, Category = "Narration")
	UNarrationSequenceComponent* GetNarrationSequence() const { return NarrationSequence; }

	/**
	 * World location where PlayerPhone is expected to be held (left hand grip).
	 * PlayerPhone itself is not implemented yet (docs/ARCHITECTURE.md 3.4, Status: Planned) —
	 * this is a stand-in anchor point for systems (e.g. enemy archer aim) that need to target
	 * "the phone" ahead of that Core feature landing. Update this once PlayerPhone exists.
	 */
	UFUNCTION(BlueprintPure, Category = "VR|Phone")
	FVector GetPhoneAnchorLocation() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void HandleTeleportStarted(const struct FInputActionValue& Value);
	void HandleTeleportTriggered(const struct FInputActionValue& Value);
	void HandleTeleportCompleted(const struct FInputActionValue& Value);
	void HandleTeleportCanceled(const struct FInputActionValue& Value);
	void HandleTurnTriggered(const struct FInputActionValue& Value);
	void HandleTurnCompleted(const struct FInputActionValue& Value);
	void HandleSmoothMoveTriggered(const struct FInputActionValue& Value);
	void HandleGrabLeft(const struct FInputActionValue& Value);
	void HandleGrabRight(const struct FInputActionValue& Value);
	void HandleReleaseLeft(const struct FInputActionValue& Value);
	void HandleReleaseRight(const struct FInputActionValue& Value);
	void HandleTriggerPressedLeft(const struct FInputActionValue& Value);
	void HandleTriggerPressedRight(const struct FInputActionValue& Value);
	void HandleTriggerReleasedLeft(const struct FInputActionValue& Value);
	void HandleTriggerReleasedRight(const struct FInputActionValue& Value);

	void StartTeleportTrace();
	void UpdateTeleportTrace(const FVector2D& InputAxis);
	void EndTeleportTrace(bool bCommitTeleport);
	void SnapTurn(float AxisValue);
	void ApplySmoothMove(const FVector2D& InputAxis);
	void ConfigureLocomotionInput();
	void RemoveLocomotionInput();
	void SetHandGraspAlpha(USkeletalMeshComponent* HandMesh, float Alpha) const;
	void TryGrab(UMotionControllerComponent* MotionController, TObjectPtr<USceneComponent>& HeldComponent);
	void TryRelease(TObjectPtr<USceneComponent>& HeldComponent, UMotionControllerComponent* MotionController = nullptr);
	USceneComponent* FindNearestGrabComponent(const UMotionControllerComponent* MotionController) const;
	bool InvokeGrabFunction(USceneComponent* GrabComponent, FName FunctionName, UMotionControllerComponent* MotionController) const;
	bool InvokeGrabOwnerFunction(USceneComponent* GrabComponent, FName FunctionName,
		UMotionControllerComponent* MotionController, bool bDefaultResult) const;
	float GetAxisX(const struct FInputActionValue& Value) const;

	UFUNCTION()
	void HandleSubtitleChanged(FText SpeakerName, FText Subtitle, bool bVisible);

	UFUNCTION()
	void HandleNarrationWidgetRequested(TSubclassOf<UUserWidget> WidgetClass, FName SourceRow);

	UFUNCTION()
	void HandleNarrationStarted(FName RowName);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Components")
	TObjectPtr<USceneComponent> VROrigin;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Components")
	TObjectPtr<UCameraComponent> VRCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Controllers")
	TObjectPtr<UMotionControllerComponent> MotionControllerLeftGrip;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Controllers")
	TObjectPtr<UMotionControllerComponent> MotionControllerLeftAim;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Controllers")
	TObjectPtr<UMotionControllerComponent> MotionControllerRightGrip;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Controllers")
	TObjectPtr<UMotionControllerComponent> MotionControllerRightAim;

	/** Visible OpenXR mannequin hands. Tracking still comes from the Grip controllers. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Hands")
	TObjectPtr<USkeletalMeshComponent> LeftHandMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Hands")
	TObjectPtr<USkeletalMeshComponent> RightHandMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|UI")
	TObjectPtr<UWidgetInteractionComponent> WidgetInteractionLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|UI")
	TObjectPtr<UWidgetInteractionComponent> WidgetInteractionRight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narration")
	TObjectPtr<UWidgetComponent> SubtitleHUD;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narration")
	TObjectPtr<UWidgetComponent> NarrationEventHUD;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narration")
	TObjectPtr<UAudioComponent> NarrationAudio;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narration")
	TObjectPtr<UNarrationSequenceComponent> NarrationSequence;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR|Input")
	TObjectPtr<UInputAction> TeleportAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR|Input")
	TObjectPtr<UInputAction> TurnAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR|Input")
	TObjectPtr<UInputAction> GrabLeftAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR|Input")
	TObjectPtr<UInputAction> GrabRightAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR|Input")
	TObjectPtr<UInputAction> ReleaseLeftAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR|Input")
	TObjectPtr<UInputAction> ReleaseRightAction;

	/** Added explicitly at possession time so packaged OpenXR builds do not depend on editor-only defaults. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** Contains the persistent OpenXR trigger mappings used by both grab hands. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR|Input")
	TObjectPtr<UInputMappingContext> HandMappingContext;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "VR|Input")
	TObjectPtr<UInputAction> SmoothMoveAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "VR|Input")
	TObjectPtr<UInputAction> ViewTurnAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "VR|Input")
	TObjectPtr<UInputAction> TriggerGrabLeftAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "VR|Input")
	TObjectPtr<UInputAction> TriggerGrabRightAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> RuntimeLocomotionMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR|Grab")
	TSubclassOf<USceneComponent> GrabComponentClass;

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> HeldComponentLeft;

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> HeldComponentRight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR|Grab", meta = (ClampMin = "1.0"))
	float GrabRadiusFromGripPosition = 15.0f;

	/** Enables right-stick smooth locomotion. Turning and interactions remain enabled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Locomotion", meta = (DisplayName = "Enable Move"))
	bool bEnableMove = true;

	/** Enables the teleport arc. Experiences that fix the player to one spot turn this off. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Locomotion", meta = (DisplayName = "Enable Teleport"))
	bool bEnableTeleport = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Locomotion", meta = (ClampMin = "0.0", Units = "cm/s"))
	float SmoothMoveSpeed = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Locomotion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SmoothMoveDeadZone = 0.15f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR|Teleport")
	TSubclassOf<AActor> TeleportVisualizerClass;

	UPROPERTY(Transient)
	TObjectPtr<AActor> TeleportVisualizer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR|Teleport")
	float TeleportHorizontalSpeed = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR|Teleport")
	float TeleportVerticalSpeed = 450.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR|Teleport")
	float TeleportMaxSimulationTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR|Teleport")
	FVector TeleportNavigationExtent = FVector(50.0f, 50.0f, 100.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR|Turn")
	float SnapTurnDegrees = -45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR|Turn", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TurnDeadZone = 0.65f;

	bool bTeleportTraceActive = false;
	bool bValidTeleportLocation = false;
	bool bTurnLatched = false;
	bool bMoveEnabledBeforeMountedInteraction = true;
	bool bWidgetPressLeft = false;
	bool bWidgetPressRight = false;
	FVector ProjectedTeleportLocation = FVector::ZeroVector;
	TWeakObjectPtr<USceneComponent> MountedCameraAnchor;

	/** Distance and height can be tuned per Blueprint for headset comfort. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narration|HUD")
	FVector SubtitleHUDOffset = FVector(85.0f, 0.0f, -28.0f);

	/** Render-target size for the narration subtitle. The default accommodates several wrapped lines. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narration|HUD", meta = (ClampMin = "1.0"))
	FVector2D SubtitleHUDDrawSize = FVector2D(1400.0f, 480.0f);

	/** World scale for the camera-attached subtitle panel. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narration|HUD", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float SubtitleHUDWorldScale = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narration|HUD")
	FVector EventHUDOffset = FVector(180.0f, 0.0f, -5.0f);
};
