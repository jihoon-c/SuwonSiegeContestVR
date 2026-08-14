#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ScenarioDefinition.generated.h"

class UScenarioSceneData;

UCLASS(BlueprintType)
class SUWONSIEGECONTESTVR_API UScenarioDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scenario")
	FName ScenarioID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scenario")
	FText ScenarioName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scenario")
	TArray<TObjectPtr<UScenarioSceneData>> Scenes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scenario")
	FName StartSceneID;

	UFUNCTION(BlueprintPure, Category = "Scenario")
	UScenarioSceneData* FindScene(FName SceneID) const;

	bool ValidateScenario(FString& OutError) const;
};
