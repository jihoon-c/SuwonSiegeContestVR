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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Common")
	FName InteractionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Common")
	EScenarioInteractionType InteractionType = EScenarioInteractionType::Custom;

	/** Logical level target, quiz, voice command, or custom content ID. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction",
		meta = (EditCondition = "InteractionType != EScenarioInteractionType::Narration && InteractionType != EScenarioInteractionType::Objective && InteractionType != EScenarioInteractionType::Wait", EditConditionHides))
	FName TargetID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective",
		meta = (EditCondition = "InteractionType == EScenarioInteractionType::Objective", EditConditionHides, MultiLine = true))
	FText ObjectiveText;

	/** Row name consumed by the existing NarrationSequenceComponent. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narration",
		meta = (EditCondition = "InteractionType == EScenarioInteractionType::Narration", EditConditionHides))
	FName NarrationID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0"))
	float DelayBeforeStart = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0"))
	float DelayAfterComplete = 0.0f;

	/** Used by Wait, or by custom Blueprint logic as an expected duration. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wait",
		meta = (ClampMin = "0.0", EditCondition = "InteractionType == EScenarioInteractionType::Wait", EditConditionHides))
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
	bool bRequired = true;

	/** Useful for instant Objective, Spawn, Sequence, or Custom steps. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow",
		meta = (EditCondition = "InteractionType == EScenarioInteractionType::Objective || InteractionType == EScenarioInteractionType::Spawn || InteractionType == EScenarioInteractionType::Sequence || InteractionType == EScenarioInteractionType::Custom", EditConditionHides))
	bool bCompleteOnStart = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
	FName NextInteractionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow",
		meta = (EditCondition = "InteractionType != EScenarioInteractionType::Narration", EditConditionHides))
	FName SuccessInteractionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow",
		meta = (EditCondition = "InteractionType != EScenarioInteractionType::Narration", EditConditionHides))
	FName FailInteractionID;
};

/** Authoring group stored directly inside a Scenario Definition. */
USTRUCT(BlueprintType)
struct SUWONSIEGECONTESTVR_API FScenarioStageDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
	FName StageID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
	FText StageName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
	FName StartInteractionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
	FName NextStageID;

	/** The complete flow is visible and editable here without opening another Data Asset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage", meta = (TitleProperty = "InteractionID"))
	TArray<FScenarioInteraction> Interactions;

	const FScenarioInteraction* FindInteraction(const FName InteractionID) const
	{
		return Interactions.FindByPredicate(
			[InteractionID](const FScenarioInteraction& Interaction)
			{
				return Interaction.InteractionID == InteractionID;
			});
	}

	bool ValidateStage(FString& OutError) const;
};

USTRUCT(BlueprintType)
struct SUWONSIEGECONTESTVR_API FScenarioDebugSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Scenario|Debug")
	FName ScenarioID;

	UPROPERTY(BlueprintReadOnly, Category = "Scenario|Debug", meta = (DisplayName = "Stage ID"))
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
