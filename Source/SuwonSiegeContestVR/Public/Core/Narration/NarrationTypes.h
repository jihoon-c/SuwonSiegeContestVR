#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "NarrationTypes.generated.h"

class USoundBase;
class UUserWidget;

UENUM(BlueprintType)
enum class ENarrationAdvanceMode : uint8
{
	Auto UMETA(DisplayName = "Auto Advance"),
	WaitForContinue UMETA(DisplayName = "Wait For Continue"),
	Stop UMETA(DisplayName = "Stop Sequence")
};

/** One narration step. Rows are linked explicitly through NextRow. */
USTRUCT(BlueprintType)
struct SUWONSIEGECONTESTVR_API FNarrationSequenceRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narration")
	FText SpeakerName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narration", meta = (MultiLine = true))
	FText Subtitle;

	/** Soft reference keeps the complete narration library out of memory. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narration")
	TSoftObjectPtr<USoundBase> NarrationSound;

	/** Used when NarrationSound is empty, which is useful while recording is not ready. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narration", meta = (ClampMin = "0.0"))
	float PreviewDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flow")
	FName NextRow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flow")
	ENarrationAdvanceMode AdvanceMode = ENarrationAdvanceMode::Auto;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flow", meta = (ClampMin = "0.0"))
	float AdvanceDelay = 0.0f;

	/** Every name is broadcast after audio and subtitle completion. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Post Narration")
	TArray<FName> CompletionEvents;

	/** Optional VR panel shown after narration. The Pawn decides how it is presented. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Post Narration")
	TSoftClassPtr<UUserWidget> PostNarrationWidgetClass;
};
