#pragma once

#include "CoreMinimal.h"
#include "Core/Scenario/ScenarioTypes.h"
#include "Engine/DataAsset.h"
#include "ScenarioDefinition.generated.h"

class UScenarioSceneData;
class UDataTable;

UCLASS(BlueprintType)
class SUWONSIEGECONTESTVR_API UScenarioDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scenario")
	FName ScenarioID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scenario")
	FText ScenarioName;

	/** All stages and interactions are authored inline so the whole flow is visible in one asset. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scenario|Flow")
	TArray<FScenarioStageDefinition> Stages;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scenario|Flow")
	FName StartStageID;

	/** Optional table resolved automatically by the placed Scenario Manager. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scenario|Presentation")
	TObjectPtr<UDataTable> NarrationTable;

	/** Deprecated authoring format. Kept only so existing assets can be migrated safely. */
	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Use inline Stages."))
	TArray<TObjectPtr<UScenarioSceneData>> Scenes;

	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Use StartStageID."))
	FName StartSceneID;

	const FScenarioStageDefinition* FindStage(FName StageID) const;
	bool ResolveStage(FName StageID, FScenarioStageDefinition& OutStage) const;
	FName GetStartStageID() const;

	UFUNCTION(BlueprintPure, Category = "Scenario")
	UScenarioSceneData* FindScene(FName SceneID) const;

	bool ValidateScenario(FString& OutError) const;
};
