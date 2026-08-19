#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ExperienceDefinition.generated.h"

class UWorld;
class UScenarioDefinition;

/** Data-only definition for one level-based educational experience. */
UCLASS(BlueprintType)
class SUWONSIEGECONTESTVR_API UExperienceDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Experience")
	FName ExperienceID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Experience")
	FText DisplayName;

	/** Single source of Scenario flow for this Experience. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Experience|Scenario")
	TObjectPtr<UScenarioDefinition> ScenarioDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Experience|Scenario")
	bool bAutoStartScenario = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Experience|Scenario")
	bool bCompleteOnScenarioFinished = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Travel")
	TSoftObjectPtr<UWorld> ExperienceLevel;

	/** Optional destination used by ReturnToMain and automatic completion return. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Travel")
	TSoftObjectPtr<UWorld> ReturnLevel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Travel")
	bool bReturnOnCompletion = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Travel")
	FString TravelOptions;

	bool ValidateDefinition(FString& OutError) const;
};
