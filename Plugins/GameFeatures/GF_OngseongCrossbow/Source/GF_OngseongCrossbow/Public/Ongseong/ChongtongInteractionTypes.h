#pragma once

#include "CoreMinimal.h"
#include "ChongtongInteractionTypes.generated.h"

UENUM(BlueprintType)
enum class EChongtongLoadingItemType : uint8
{
	Powder,
	Rammer,
	Cannonball
};

UENUM(BlueprintType)
enum class EChongtongLoadingState : uint8
{
	NeedsPowder,
	NeedsRamming,
	NeedsCannonball,
	ReadyToAim,
	Completed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChongtongLoadingStateChanged, EChongtongLoadingState, NewState, int32, CompletedShots);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChongtongRammingProgress, int32, CompletedRams, int32, RequiredRams);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChongtongExperienceCompleted, int32, TotalShots);
