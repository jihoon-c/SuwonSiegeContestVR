#pragma once

#include "CoreMinimal.h"
#include "ScenarioTypes.generated.h"

UENUM(BlueprintType)
enum class EScenarioInteractionType : uint8
{
	Narration,
	Objective,
	Grab,
	Press,
	Observe,
	Move,
	Trigger,
	Quiz,
	VoiceCommand,
	Combat,
	Wait,
	Sequence,
	Spawn,
	Custom
};

UENUM(BlueprintType)
enum class EScenarioInteractionState : uint8
{
	Inactive,
	Ready,
	Running,
	Completed,
	Failed,
	Skipped
};

UENUM(BlueprintType)
enum class EScenarioSceneState : uint8
{
	Inactive,
	Running,
	Completed,
	Failed
};

UENUM(BlueprintType)
enum class EScenarioState : uint8
{
	Inactive,
	Running,
	Completed,
	Failed
};

/** One data-driven unit of scenario progress. IDs, rather than array indices, define flow. */
USTRUCT(BlueprintType)
struct SUWONSIEGECONTESTVR_API FScenarioInteraction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario")
	FName InteractionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario")
	EScenarioInteractionType InteractionType = EScenarioInteractionType::Custom;

	/** Logical level target, quiz, voice command, or custom content ID. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario")
	FName TargetID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario")
	FText ObjectiveText;

	/** Row name consumed by the existing NarrationSequenceComponent. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario")
	FName NarrationID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0"))
	float DelayBeforeStart = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0"))
	float DelayAfterComplete = 0.0f;

	/** Used by Wait, or by custom Blueprint logic as an expected duration. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0"))
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
	bool bRequired = true;

	/** Useful for instant Objective, Spawn, Sequence, or Custom steps. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
	bool bCompleteOnStart = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
	FName NextInteractionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
	FName SuccessInteractionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
	FName FailInteractionID;
};

USTRUCT(BlueprintType)
struct SUWONSIEGECONTESTVR_API FScenarioDebugSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Scenario|Debug")
	FName ScenarioID;

	UPROPERTY(BlueprintReadOnly, Category = "Scenario|Debug")
	FName SceneID;

	UPROPERTY(BlueprintReadOnly, Category = "Scenario|Debug")
	FName InteractionID;

	UPROPERTY(BlueprintReadOnly, Category = "Scenario|Debug")
	EScenarioInteractionType InteractionType = EScenarioInteractionType::Custom;

	UPROPERTY(BlueprintReadOnly, Category = "Scenario|Debug")
	FName TargetID;

	UPROPERTY(BlueprintReadOnly, Category = "Scenario|Debug")
	EScenarioState ScenarioState = EScenarioState::Inactive;

	UPROPERTY(BlueprintReadOnly, Category = "Scenario|Debug")
	EScenarioSceneState SceneState = EScenarioSceneState::Inactive;

	UPROPERTY(BlueprintReadOnly, Category = "Scenario|Debug")
	EScenarioInteractionState InteractionState = EScenarioInteractionState::Inactive;
};
