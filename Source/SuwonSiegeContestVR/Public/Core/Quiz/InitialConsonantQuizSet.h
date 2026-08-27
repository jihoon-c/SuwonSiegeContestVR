#pragma once

#include "CoreMinimal.h"
#include "Core/Quiz/InitialConsonantQuizTypes.h"
#include "Engine/DataAsset.h"
#include "InitialConsonantQuizSet.generated.h"

/**
 * Shared quiz library (DA_). Several experiences can point at the same asset, and a level can add
 * its own entries on the component without copying this one.
 */
UCLASS(BlueprintType)
class SUWONSIEGECONTESTVR_API UInitialConsonantQuizSet : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quiz", meta = (TitleProperty = "QuizID"))
	TArray<FInitialConsonantQuizDefinition> Quizzes;

	UFUNCTION(BlueprintPure, Category = "Quiz")
	bool FindQuiz(FName QuizID, FInitialConsonantQuizDefinition& OutQuiz) const;

	/** Reports duplicate IDs and entries without an answer. */
	UFUNCTION(BlueprintPure, Category = "Quiz")
	bool ValidateQuizSet(FString& OutError) const;
};
