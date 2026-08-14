#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ExperienceDefinition.generated.h"

class UWorld;

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
