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

/** Session-only checkpoint used to resume a level-local Scenario after Experience travel. */
USTRUCT(BlueprintType)
struct SUWONSIEGECONTESTVR_API FScenarioResumeCheckpoint
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Experience|Scenario")
	FName ScenarioID;

	UPROPERTY(BlueprintReadOnly, Category = "Experience|Scenario", meta = (DisplayName = "Stage ID"))
	FName SceneID;

	UPROPERTY(BlueprintReadOnly, Category = "Experience|Scenario")
	FName InteractionID;

	bool IsValid() const
	{
		return !ScenarioID.IsNone() && !SceneID.IsNone() && !InteractionID.IsNone();
	}
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
