#pragma once

#include "CoreMinimal.h"
#include "Core/Scenario/ScenarioTypes.h"
#include "MainEducationTypes.generated.h"

class UExperienceDefinition;
class UTexture2D;

UENUM(BlueprintType)
enum class EMainEducationContentType : uint8
{
	Instructor,
	Image,
	Quiz,
	Comparison,
	Summary
};

/** Presentation payload consumed by the Main-level VR widget or Blueprint. */
USTRUCT(BlueprintType)
struct SUWONSIEGECONTESTVR_API FMainEducationContent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Education")
	FName ContentID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Education")
	EMainEducationContentType ContentType = EMainEducationContentType::Instructor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Education")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Education")
	FText SpeakerName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Education", meta = (MultiLine = true))
	FText Body;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Education")
	FText HighlightText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Education")
	TArray<FText> Callouts;

	/** Player-facing prompt. Main HUD and the final education Widget can display this directly. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Education|Guide", meta = (MultiLine = true))
	FText InteractionGuideText;

	/** Designer-only context such as intended camera framing or image replacement notes. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Education|Editor", meta = (MultiLine = true))
	FText EditorNotes;

	/** Replaceable presentation image. The project supplies clearly marked example textures. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Education|Visual")
	TSoftObjectPtr<UTexture2D> Image;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Education|Quiz")
	FText InitialConsonants;

	/** First value is treated as the canonical answer; following values are accepted aliases. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Education|Quiz")
	TArray<FText> AcceptedAnswers;
};

/** Data-only route. Main depends on a Core Experience asset, never on a GF C++ module. */
USTRUCT(BlueprintType)
struct SUWONSIEGECONTESTVR_API FMainEducationExperienceRoute
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Education|Experience")
	FName RouteID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Education|Experience")
	TSoftObjectPtr<UExperienceDefinition> Experience;
};

/** Simplified authoring choices. Runtime Scenario interaction types are generated from these values. */
UENUM(BlueprintType)
enum class EMainEducationAuthoringStepType : uint8
{
	Narration UMETA(DisplayName = "Narration + Presentation"),
	Presentation UMETA(DisplayName = "Presentation / Confirm"),
	Quiz UMETA(DisplayName = "Quiz / Answer"),
	Travel UMETA(DisplayName = "Experience Travel")
};

/** One editor-friendly beat. Array order defines the next beat automatically. */
USTRUCT(BlueprintType)
struct SUWONSIEGECONTESTVR_API FMainEducationAuthoringStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
	FName StepID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
	EMainEducationAuthoringStepType StepType = EMainEducationAuthoringStepType::Presentation;

	/** Screen data is kept next to its flow step so designers do not have to synchronize two arrays. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presentation",
		meta = (EditCondition = "StepType != EMainEducationAuthoringStepType::Travel", EditConditionHides))
	FMainEducationContent Content;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narration",
		meta = (EditCondition = "StepType == EMainEducationAuthoringStepType::Narration", EditConditionHides))
	FName NarrationStartRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Experience",
		meta = (EditCondition = "StepType == EMainEducationAuthoringStepType::Travel", EditConditionHides))
	FName ExperienceRouteID;

	/** Short description visible only to designers. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design", meta = (MultiLine = true))
	FText PlayerAction;

	/** Explains which event advances this step. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design", meta = (MultiLine = true))
	FText CompletionCondition;

	/** Optional world-space guide for a target Actor. UI-only steps primarily use Content.InteractionGuideText. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guide")
	EScenarioGuideAction WorldGuideAction = EScenarioGuideAction::Hidden;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guide", meta = (MultiLine = true))
	FText WorldGuideText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0"))
	float DelayBeforeStart = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "0.0"))
	float DelayAfterComplete = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
	bool bRequired = true;
};

/** Reorder stages or steps in this tree; Start/Next IDs are generated from array order. */
USTRUCT(BlueprintType)
struct SUWONSIEGECONTESTVR_API FMainEducationAuthoringStage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
	FName StageID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
	FText StageName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage", meta = (TitleProperty = "StepID"))
	TArray<FMainEducationAuthoringStep> Steps;
};
