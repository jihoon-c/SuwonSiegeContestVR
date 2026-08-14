#include "Core/Scenario/ScenarioDefinition.h"

#include "Core/Scenario/ScenarioSceneData.h"

UScenarioSceneData* UScenarioDefinition::FindScene(const FName SceneID) const
{
	const TObjectPtr<UScenarioSceneData>* Result = Scenes.FindByPredicate(
		[SceneID](const TObjectPtr<UScenarioSceneData>& Scene)
		{
			return IsValid(Scene) && Scene->SceneID == SceneID;
		});
	return Result ? Result->Get() : nullptr;
}

bool UScenarioDefinition::ValidateScenario(FString& OutError) const
{
	if (ScenarioID.IsNone())
	{
		OutError = TEXT("ScenarioID is None.");
		return false;
	}
	if (Scenes.IsEmpty())
	{
		OutError = FString::Printf(TEXT("Scenario %s has no scenes."), *ScenarioID.ToString());
		return false;
	}

	TSet<FName> SceneIDs;
	for (const UScenarioSceneData* Scene : Scenes)
	{
		if (!IsValid(Scene))
		{
			OutError = FString::Printf(TEXT("Scenario %s contains a null scene."), *ScenarioID.ToString());
			return false;
		}
		if (SceneIDs.Contains(Scene->SceneID))
		{
			OutError = FString::Printf(TEXT("Scenario %s contains duplicate scene ID %s."),
				*ScenarioID.ToString(), *Scene->SceneID.ToString());
			return false;
		}
		SceneIDs.Add(Scene->SceneID);

		FString SceneError;
		if (!Scene->ValidateScene(SceneError))
		{
			OutError = SceneError;
			return false;
		}
	}

	if (StartSceneID.IsNone() || !SceneIDs.Contains(StartSceneID))
	{
		OutError = FString::Printf(TEXT("Scenario %s has an invalid StartSceneID %s."),
			*ScenarioID.ToString(), *StartSceneID.ToString());
		return false;
	}

	for (const UScenarioSceneData* Scene : Scenes)
	{
		if (!Scene->NextSceneID.IsNone() && !SceneIDs.Contains(Scene->NextSceneID))
		{
			OutError = FString::Printf(TEXT("Scene %s references missing next scene %s."),
				*Scene->SceneID.ToString(), *Scene->NextSceneID.ToString());
			return false;
		}
	}

	OutError.Reset();
	return true;
}
