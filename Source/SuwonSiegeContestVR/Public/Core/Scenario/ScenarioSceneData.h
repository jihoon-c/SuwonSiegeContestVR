#pragma once

#include "CoreMinimal.h"
#include "Core/Scenario/ScenarioTypes.h"
#include "Engine/DataAsset.h"
#include "ScenarioSceneData.generated.h"

UCLASS(BlueprintType, meta = (DisplayName = "DEPRECATED - Scenario Scene Data (Use Scenario Definition Stages)"))
class SUWONSIEGECONTESTVR_API UScenarioSceneData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scenario|Scene")
	FName SceneID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scenario|Scene")
	FText SceneName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scenario|Scene")
	TArray<FScenarioInteraction> Interactions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scenario|Scene")
	FName StartInteractionID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scenario|Scene")
	FName NextSceneID;

	const FScenarioInteraction* FindInteraction(FName InteractionID) const;
	bool ValidateScene(FString& OutError) const;
};
