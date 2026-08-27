#pragma once

#include "CoreMinimal.h"

namespace SingijeonLaunchSpread
{
    inline FVector Apply(
        const FVector& BaseDirection,
        FRandomStream& RandomStream,
        const float HorizontalHalfAngleDegrees,
        const float VerticalHalfAngleDegrees)
    {
        const FVector NormalizedDirection = BaseDirection.GetSafeNormal();
        if (NormalizedDirection.IsNearlyZero())
        {
            return FVector::ForwardVector;
        }

        FRotator SpreadRotation = NormalizedDirection.Rotation();
        SpreadRotation.Yaw += RandomStream.FRandRange(
            -FMath::Max(0.0f, HorizontalHalfAngleDegrees),
            FMath::Max(0.0f, HorizontalHalfAngleDegrees));
        SpreadRotation.Pitch += RandomStream.FRandRange(
            -FMath::Max(0.0f, VerticalHalfAngleDegrees),
            FMath::Max(0.0f, VerticalHalfAngleDegrees));
        return SpreadRotation.Vector().GetSafeNormal();
    }
}
