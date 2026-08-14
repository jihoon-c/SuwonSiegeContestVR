#include "Interaction/TwoHandCarryComponent.h"

#include "Components/SceneComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/HitResult.h"
#include "GameFramework/Actor.h"

UTwoHandCarryComponent::UTwoHandCarryComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

bool UTwoHandCarryComponent::BeginGrip(const ECarryGripSide Side, USceneComponent* HandTransform)
{
    if (!bCarryEnabled || !IsValid(HandTransform))
    {
        return false;
    }

    if (Side == ECarryGripSide::Left)
    {
        LeftHand = HandTransform;
    }
    else
    {
        RightHand = HandTransform;
    }

    if (IsBeingCarried())
    {
        CaptureGripBaseline();
        SetComponentTickEnabled(true);
        if (!bLastBroadcastCarryState)
        {
            bLastBroadcastCarryState = true;
            OnCarryStateChanged.Broadcast(true);
        }
    }

    return true;
}

void UTwoHandCarryComponent::EndGrip(const ECarryGripSide Side, USceneComponent* HandTransform)
{
    TWeakObjectPtr<USceneComponent>& Grip = Side == ECarryGripSide::Left ? LeftHand : RightHand;
    if (!HandTransform || Grip.Get() == HandTransform)
    {
        Grip.Reset();
    }

    bHasBaseline = false;
    SetComponentTickEnabled(false);
    if (bLastBroadcastCarryState)
    {
        bLastBroadcastCarryState = false;
        OnCarryStateChanged.Broadcast(false);
    }
}

void UTwoHandCarryComponent::SetCarryEnabled(const bool bEnabled)
{
    if (bCarryEnabled == bEnabled)
    {
        return;
    }

    bCarryEnabled = bEnabled;
    if (!bCarryEnabled)
    {
        ClearGrips();
    }
}

bool UTwoHandCarryComponent::IsBeingCarried() const
{
    return bCarryEnabled && LeftHand.IsValid() && RightHand.IsValid();
}

void UTwoHandCarryComponent::CaptureGripBaseline()
{
    if (!IsBeingCarried() || !GetOwner())
    {
        bHasBaseline = false;
        return;
    }

    const FVector LeftLocation = LeftHand->GetComponentLocation();
    const FVector RightLocation = RightHand->GetComponentLocation();
    BaselineActorTransform = GetOwner()->GetActorTransform();
    BaselineHandMidpoint = (LeftLocation + RightLocation) * 0.5f;
    BaselineHandDirection = bYawRotationOnly
        ? (RightLocation - LeftLocation).GetSafeNormal2D()
        : (RightLocation - LeftLocation).GetSafeNormal();
    bHasBaseline = !BaselineHandDirection.IsNearlyZero();
}

void UTwoHandCarryComponent::TickComponent(const float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!IsBeingCarried() || !bHasBaseline || !GetOwner())
    {
        ClearGrips();
        return;
    }

    const FVector CurrentLeft = LeftHand->GetComponentLocation();
    const FVector CurrentRight = RightHand->GetComponentLocation();
    const FVector CurrentMidpoint = (CurrentLeft + CurrentRight) * 0.5f;
    FVector DesiredLocation = BaselineActorTransform.GetLocation() + (CurrentMidpoint - BaselineHandMidpoint);
    if (bConstrainToGroundPlane)
    {
        DesiredLocation.Z = BaselineActorTransform.GetLocation().Z;
    }

    FQuat DesiredRotation = BaselineActorTransform.GetRotation();
    const FVector CurrentDirection = bYawRotationOnly
        ? (CurrentRight - CurrentLeft).GetSafeNormal2D()
        : (CurrentRight - CurrentLeft).GetSafeNormal();
    if (!CurrentDirection.IsNearlyZero())
    {
        if (bYawRotationOnly)
        {
            const float CrossZ = FVector::CrossProduct(BaselineHandDirection, CurrentDirection).Z;
            const float Dot = FVector::DotProduct(BaselineHandDirection, CurrentDirection);
            const float DeltaYawDegrees = FMath::RadiansToDegrees(FMath::Atan2(CrossZ, Dot));
            const FQuat YawDelta(FVector::UpVector, FMath::DegreesToRadians(DeltaYawDegrees));
            DesiredRotation = YawDelta * BaselineActorTransform.GetRotation();
        }
        else
        {
            DesiredRotation = FQuat::FindBetweenNormals(BaselineHandDirection, CurrentDirection) *
                BaselineActorTransform.GetRotation();
        }
    }

    const FVector CurrentActorLocation = GetOwner()->GetActorLocation();
    const FVector NewLocation = CurrentActorLocation +
        (DesiredLocation - CurrentActorLocation).GetClampedToMaxSize(MaxLinearSpeed * DeltaTime);

    const FQuat CurrentRotation = GetOwner()->GetActorQuat();
    const float MaxAngleRadians = FMath::DegreesToRadians(MaxAngularSpeed * DeltaTime);
    const FQuat NewRotation = FQuat::Slerp(
        CurrentRotation,
        DesiredRotation,
        FMath::Min(1.0f, MaxAngleRadians / FMath::Max(CurrentRotation.AngularDistance(DesiredRotation), KINDA_SMALL_NUMBER)));

    FHitResult SweepResult;
    GetOwner()->SetActorLocationAndRotation(NewLocation, NewRotation, true, &SweepResult, ETeleportType::TeleportPhysics);
}

void UTwoHandCarryComponent::ClearGrips()
{
    LeftHand.Reset();
    RightHand.Reset();
    bHasBaseline = false;
    SetComponentTickEnabled(false);
    if (bLastBroadcastCarryState)
    {
        bLastBroadcastCarryState = false;
        OnCarryStateChanged.Broadcast(false);
    }
}
