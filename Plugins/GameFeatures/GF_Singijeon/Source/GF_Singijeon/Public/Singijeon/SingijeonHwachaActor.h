#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SingijeonHwachaActor.generated.h"

class UFuseIgnitionComponent;
class UInstancedStaticMeshComponent;
class UMaterialInterface;
class UMotionControllerComponent;
class UNiagaraComponent;
class USceneComponent;
class UScenarioInteractableComponent;
class USingijeonAmmoSlotComponent;
class USoundBase;
class UStaticMesh;
class UStaticMeshComponent;
class UTwoHandCarryComponent;

UENUM(BlueprintType)
enum class ESingijeonHwachaState : uint8
{
    Empty,
    Loaded,
    Igniting,
    Fired
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHwachaStateChanged, ESingijeonHwachaState, OldState, ESingijeonHwachaState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHwachaLoadCountChanged, int32, LoadedCount, int32, TotalSlots);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHwachaVolleyLaunched);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHwachaAimCompleted);

UCLASS(Blueprintable)
class GF_SINGIJEON_API ASingijeonHwachaActor : public AActor
{
    GENERATED_BODY()

public:
    ASingijeonHwachaActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintPure, Category = "Singijeon|Hwacha")
    int32 GetLoadedAmmunitionCount() const;

    UFUNCTION(BlueprintPure, Category = "Singijeon|Hwacha")
    int32 GetAmmunitionCapacity() const;

    UFUNCTION(BlueprintPure, Category = "Singijeon|Hwacha")
    bool IsReadyToIgnite() const;

    UFUNCTION(BlueprintPure, Category = "Singijeon|Hwacha")
    bool CanAcceptAmmunitionNow() const;

    UFUNCTION(BlueprintPure, Category = "Singijeon|Hwacha")
    bool CanBeginFuseIgnitionNow() const;

    UFUNCTION(BlueprintCallable, Category = "Singijeon|Hwacha")
    void LaunchVolley();

    /** Keeps a fuse-completed launch request alive until Hwacha_Fire becomes current. */
    UFUNCTION(BlueprintCallable, Category = "Singijeon|Hwacha")
    void TryStartPendingVolley();

    UFUNCTION(BlueprintCallable, Category = "Singijeon|Hwacha")
    void ResetHwacha();

    /** Completes the Hwacha_Aim scenario interaction and dismisses the handle guide. */
    UFUNCTION(BlueprintCallable, Category = "Singijeon|Hwacha")
    bool CompleteAimInteraction();

    UFUNCTION(BlueprintPure, Category = "Singijeon|Hwacha")
    bool IsAimInteractionComplete() const { return bAimInteractionComplete; }

    UFUNCTION(BlueprintPure, Category = "Singijeon|Hwacha")
    bool IsFuseIgnitionEffectRequested() const { return bFuseIgnitionEffectRequested; }

    UFUNCTION(BlueprintPure, Category = "Singijeon|Hwacha")
    bool IsVolleyLaunchPending() const { return bPendingLaunchAfterFuse; }

    UFUNCTION(BlueprintPure, Category = "Singijeon|Hwacha")
    bool IsFuseGuideVisible() const;

    UFUNCTION(BlueprintPure, Category = "Singijeon|Hwacha")
    ESingijeonHwachaState GetHwachaState() const { return HwachaState; }

    UFUNCTION(BlueprintPure, Category = "Singijeon|Launch")
    float GetVolleyDuration() const { return VolleyDuration; }

    UFUNCTION(BlueprintPure, Category = "Singijeon|Launch")
    float GetCurrentLaunchInterval() const { return CurrentLaunchInterval; }

    /** Optional one-shot played after each arrow successfully leaves the Hwacha. */
    UFUNCTION(BlueprintPure, Category = "Singijeon|Launch|Audio")
    USoundBase* GetArrowLaunchSound() const { return ArrowLaunchSound; }

    UPROPERTY(BlueprintAssignable, Category = "Singijeon|Hwacha")
    FOnHwachaStateChanged OnHwachaStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Singijeon|Hwacha")
    FOnHwachaLoadCountChanged OnLoadCountChanged;

    UPROPERTY(BlueprintAssignable, Category = "Singijeon|Hwacha")
    FOnHwachaVolleyLaunched OnVolleyLaunched;

    UPROPERTY(BlueprintAssignable, Category = "Singijeon|Hwacha")
    FOnHwachaAimCompleted OnAimCompleted;

protected:
    virtual void OnConstruction(const FTransform& Transform) override;

    UFUNCTION()
    void HandleSlotChanged(USingijeonAmmoSlotComponent* Slot, AActor* Ammunition);

    UFUNCTION()
    void HandleIgnitionStarted(AActor* SourceActor);

    UFUNCTION()
    void HandleIgnitionCanceled(AActor* SourceActor);

    UFUNCTION()
    void HandleFuseIgnited();

    UFUNCTION()
    void HandleCarryStateChanged(bool bIsCarrying);

    UFUNCTION()
    bool HandleVRGrabbed(USceneComponent* GrabComponent, UMotionControllerComponent* MotionController);

    UFUNCTION()
    void HandleVRReleased(USceneComponent* GrabComponent, UMotionControllerComponent* MotionController);

    void RefreshLoadState();
    void SetHwachaState(ESingijeonHwachaState NewState);
    void SetHandleHighlightsVisible(bool bVisible);
    void SetMoveTargetVisible(bool bVisible);
    void SetFuseIgnitionEffectActive(bool bActive);
    void SetFuseGuideVisible(bool bVisible);

    UFUNCTION()
    void LaunchNextAmmunition();
    void FinishVolley();
    void AutoFillRemainingAmmunition(AActor* SourceAmmunition);
    void ClearAutoFilledAmmunition();
    int32 GetPhysicallyLoadedAmmunitionCount() const;
    bool LaunchNextAutoFilledAmmunition(int32 InstanceIndex);
    void ConfigureLaunchedAmmunitionCollision(AActor* Ammunition);
    void PlayArrowLaunchSound(const FVector& LaunchLocation) const;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> BodyMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> RackRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USingijeonAmmoSlotComponent> DefaultAmmoSlot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UInstancedStaticMeshComponent> AutoLoadedArrowInstances;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UFuseIgnitionComponent> Fuse;

    /** Visible confirmation while the lit torch is held against the Fuse. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UNiagaraComponent> FuseIgnitionEffect;

    /** Small emissive marker showing exactly where the torch flame must touch. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> FuseGuide;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UTwoHandCarryComponent> TwoHandCarry;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> LeftHandleGrabPoint;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> RightHandleGrabPoint;

    /** Visual-only proxies aligned over the two wooden handles. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> LeftHandleHighlight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> RightHandleHighlight;

    /** World-space destination shown after loading. It detaches at BeginPlay so it does not follow the Hwacha. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> MoveTargetMarker;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UScenarioInteractableComponent> LoadScenarioInteractor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UScenarioInteractableComponent> AimScenarioInteractor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UScenarioInteractableComponent> IgniteScenarioInteractor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UScenarioInteractableComponent> FireScenarioInteractor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Launch", meta = (ClampMin = "1"))
    int32 MinimumLoadedAmmunition = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Launch", meta = (ClampMin = "1.0"))
    float LaunchSpeed = 3500.0f;

    /** Time from the first launched arrow to the final launched arrow. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Launch", meta = (ClampMin = "0.0", Units = "s"))
    float VolleyDuration = 10.0f;

    /** Assign a Sound Wave or Sound Cue here in BP_SingijeonHwacha or on a placed Hwacha actor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Launch|Audio")
    TObjectPtr<USoundBase> ArrowLaunchSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Launch|Audio", meta = (ClampMin = "0.0"))
    float ArrowLaunchSoundVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Launch|Audio", meta = (ClampMin = "0.0"))
    float ArrowLaunchSoundPitchMin = 0.95f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Launch|Audio", meta = (ClampMin = "0.0"))
    float ArrowLaunchSoundPitchMax = 1.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Auto Fill")
    bool bAutoFillOnFirstLoad = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Auto Fill", meta = (ClampMin = "1"))
    int32 AutoFillRows = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Auto Fill", meta = (ClampMin = "1"))
    int32 AutoFillColumns = 15;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Auto Fill", meta = (ClampMin = "0.1"))
    float AutoFillColumnSpacing = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Auto Fill", meta = (ClampMin = "0.1"))
    float AutoFillRowSpacing = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Auto Fill")
    FVector AutoFillGridOffset = FVector::ZeroVector;

    /** Optional fixed mesh. When empty, the first loaded ammunition mesh is used. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Auto Fill")
    TObjectPtr<UStaticMesh> AutoFillArrowMesh;

    /** Optional material override for auto-filled arrow instances. Falls back to the loaded arrow material. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Auto Fill")
    TObjectPtr<UMaterialInterface> AutoFillArrowMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Aim Guide")
    bool bEnableAimGuideHighlight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Aim Guide")
    bool bShowMoveTargetMarker = true;

    /** Horizontal distance from the marker that completes Hwacha_Aim. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Aim Guide", meta = (ClampMin = "1.0"))
    float MoveTargetAcceptanceRadius = 45.0f;

    /** Moving at least this far while carrying completes Hwacha_Aim. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Aim Guide", meta = (ClampMin = "0.0"))
    float AimCompletionDistance = 30.0f;

    /** Rotating at least this many degrees while carrying also completes Hwacha_Aim. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Aim Guide", meta = (ClampMin = "0.0", ClampMax = "180.0"))
    float AimCompletionYawDegrees = 10.0f;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Singijeon|Hwacha")
    ESingijeonHwachaState HwachaState = ESingijeonHwachaState::Empty;

    UPROPERTY(Transient)
    TArray<TObjectPtr<USingijeonAmmoSlotComponent>> AmmoSlots;

    UPROPERTY(Transient)
    TArray<TObjectPtr<USingijeonAmmoSlotComponent>> PendingLaunchSlots;

    UPROPERTY(Transient)
    TSubclassOf<AActor> AutoFilledAmmunitionClass;

    bool bAutoFilled = false;
    bool bAutoFillInProgress = false;
    bool bLoadInteractionReported = false;

    FTransform AimStartTransform;
    bool bHasAimStartTransform = false;
    bool bAimInteractionComplete = false;
    bool bFuseIgnitionEffectRequested = false;

    float CurrentLaunchInterval = 0.0f;
    FTimerHandle LaunchTimerHandle;
    FTimerHandle PendingVolleyTimerHandle;
    bool bPendingLaunchAfterFuse = false;

    /** Prevents volley arrows from colliding with the rack or one another at spawn. */
    TArray<TWeakObjectPtr<AActor>> ActiveLaunchedAmmunition;
};
