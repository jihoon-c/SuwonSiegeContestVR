#pragma once

#include "CoreMinimal.h"
#include "ExperienceTypes.generated.h"

UENUM(BlueprintType)
enum class EExperienceState : uint8
{
	Inactive,
	Traveling,
	Active,
	Completed,
	Failed
};

USTRUCT(BlueprintType)
struct SUWONSIEGECONTESTVR_API FExperienceProgressSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Experience")
	FName CurrentExperienceID;

	UPROPERTY(BlueprintReadOnly, Category = "Experience")
	EExperienceState State = EExperienceState::Inactive;

	UPROPERTY(BlueprintReadOnly, Category = "Experience")
	TArray<FName> CompletedExperienceIDs;
};
