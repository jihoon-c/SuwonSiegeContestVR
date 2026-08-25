#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TwoHandCarryComponent.generated.h"

class USceneComponent;

UENUM(BlueprintType)
enum class ECarryGripSide : uint8
{
    Left,
    Right
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTwoHandCarryStateChanged, bool, bIsCarrying);

UCLASS(ClassGroup = (VRInteraction), meta = (BlueprintSpawnableComponent))
class GF_SINGIJEON_API UTwoHandCarryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTwoHandCarryComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "VR Interaction|Two Hand Carry")
    bool BeginGrip(ECarryGripSide Side, USceneComponent* HandTransform);

    UFUNCTION(BlueprintCallable, Category = "VR Interaction|Two Hand Carry")
    void EndGrip(ECarryGripSide Side, USceneComponent* HandTransform);

    UFUNCTION(BlueprintCallable, Category = "VR Interaction|Two Hand Carry")
    void SetCarryEnabled(bool bEnabled);

    /** Locks ground-constrained carrying to an authored world height until cleared. */
    UFUNCTION(BlueprintCallable, Category = "VR Interaction|Two Hand Carry")
    void SetConstrainedWorldZ(float WorldZ);

    UFUNCTION(BlueprintCallable, Category = "VR Interaction|Two Hand Carry")
    void ClearConstrainedWorldZ();

    UFUNCTION(BlueprintPure, Category = "VR Interaction|Two Hand Carry")
    bool IsBeingCarried() const;

    UFUNCTION(BlueprintPure, Category = "VR Interaction|Two Hand Carry")
    bool IsCarryEnabled() const { return bCarryEnabled; }

    UFUNCTION(BlueprintPure, Category = "VR Interaction|Two Hand Carry")
    bool HasLeftGrip() const { return LeftHand.IsValid(); }

    UFUNCTION(BlueprintPure, Category = "VR Interaction|Two Hand Carry")
    bool HasRightGrip() const { return RightHand.IsValid(); }

    UPROPERTY(BlueprintAssignable, Category = "VR Interaction|Two Hand Carry")
    FOnTwoHandCarryStateChanged OnCarryStateChanged;

protected:
    void CaptureGripBaseline();
    void ClearGrips();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Interaction|Two Hand Carry")
    bool bConstrainToGroundPlane = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Interaction|Two Hand Carry")
    bool bYawRotationOnly = true;

    /** When enabled, either the left or right hand can translate the owner. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Interaction|Two Hand Carry")
    bool bAllowSingleHandCarry = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Interaction|Two Hand Carry", meta = (ClampMin = "1.0"))
    float MaxLinearSpeed = 300.0f;

    /** Keeps a grabbed cart locked to the hand delta instead of visibly trailing behind locomotion. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Interaction|Two Hand Carry")
    bool bFollowHandWithoutLag = true;

    /**
     * Sweeping a large cart that already touches the floor can reject every hand delta.
     * Leave this disabled for direct VR dragging; enable it only for actors whose root
     * collision is authored with enough clearance for swept movement.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Interaction|Two Hand Carry")
    bool bSweepMovement = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Interaction|Two Hand Carry", meta = (ClampMin = "1.0"))
    float MaxAngularSpeed = 120.0f;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "VR Interaction|Two Hand Carry")
    bool bCarryEnabled = false;

    UPROPERTY()
    TWeakObjectPtr<USceneComponent> LeftHand;

    UPROPERTY()
    TWeakObjectPtr<USceneComponent> RightHand;

    FTransform BaselineActorTransform;
    FVector BaselineHandMidpoint = FVector::ZeroVector;
    FVector BaselineHandDirection = FVector::ForwardVector;
    bool bBaselineUsesTwoHands = false;
    bool bHasBaseline = false;
    bool bLastBroadcastCarryState = false;
    bool bHasConstrainedWorldZ = false;
    float ConstrainedWorldZ = 0.0f;
};
