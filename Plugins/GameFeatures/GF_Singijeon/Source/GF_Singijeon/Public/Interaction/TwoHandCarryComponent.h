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

    UFUNCTION(BlueprintPure, Category = "VR Interaction|Two Hand Carry")
    bool IsBeingCarried() const;

    UFUNCTION(BlueprintPure, Category = "VR Interaction|Two Hand Carry")
    bool IsCarryEnabled() const { return bCarryEnabled; }

    UPROPERTY(BlueprintAssignable, Category = "VR Interaction|Two Hand Carry")
    FOnTwoHandCarryStateChanged OnCarryStateChanged;

protected:
    void CaptureGripBaseline();
    void ClearGrips();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Interaction|Two Hand Carry")
    bool bConstrainToGroundPlane = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Interaction|Two Hand Carry")
    bool bYawRotationOnly = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Interaction|Two Hand Carry", meta = (ClampMin = "1.0"))
    float MaxLinearSpeed = 300.0f;

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
    bool bHasBaseline = false;
    bool bLastBroadcastCarryState = false;
};
