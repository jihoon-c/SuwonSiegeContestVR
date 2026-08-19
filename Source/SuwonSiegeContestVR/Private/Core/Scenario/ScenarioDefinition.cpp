#include "Core/Scenario/ScenarioDefinition.h"

#include "Core/Scenario/ScenarioSceneData.h"

bool FScenarioStageDefinition::ValidateStage(FString& OutError) const
{
	if (StageID.IsNone())
	{
		OutError = TEXT("StageID is None.");
		return false;
	}
	if (Interactions.IsEmpty())
	{
		OutError = FString::Printf(TEXT("Stage %s has no interactions."), *StageID.ToString());
		return false;
	}

	TSet<FName> InteractionIDs;
	for (const FScenarioInteraction& Interaction : Interactions)
	{
		if (Interaction.InteractionID.IsNone() || InteractionIDs.Contains(Interaction.InteractionID))
		{
			OutError = FString::Printf(TEXT("Stage %s has an empty or duplicate interaction ID %s."),
				*StageID.ToString(), *Interaction.InteractionID.ToString());
			return false;
		}
		InteractionIDs.Add(Interaction.InteractionID);
	}
	if (StartInteractionID.IsNone() || !InteractionIDs.Contains(StartInteractionID))
	{
		OutError = FString::Printf(TEXT("Stage %s has an invalid StartInteractionID %s."),
			*StageID.ToString(), *StartInteractionID.ToString());
		return false;
	}
	for (const FScenarioInteraction& Interaction : Interactions)
	{
		for (const FName Target : {
			Interaction.NextInteractionID, Interaction.SuccessInteractionID, Interaction.FailInteractionID})
		{
			if (!Target.IsNone() && !InteractionIDs.Contains(Target))
			{
				OutError = FString::Printf(TEXT("Interaction %s references missing interaction %s."),
					*Interaction.InteractionID.ToString(), *Target.ToString());
				return false;
			}
		}
	}
	OutError.Reset();
	return true;
}

const FScenarioStageDefinition* UScenarioDefinition::FindStage(const FName StageID) const
{
	return Stages.FindByPredicate(
		[StageID](const FScenarioStageDefinition& Stage)
		{
			return Stage.StageID == StageID;
		});
}

bool UScenarioDefinition::ResolveStage(
	const FName StageID, FScenarioStageDefinition& OutStage) const
{
	if (const FScenarioStageDefinition* Stage = FindStage(StageID))
	{
		OutStage = *Stage;
		return true;
	}
	if (const UScenarioSceneData* Scene = FindScene(StageID))
	{
		OutStage.StageID = Scene->SceneID;
		OutStage.StageName = Scene->SceneName;
		OutStage.StartInteractionID = Scene->StartInteractionID;
		OutStage.NextStageID = Scene->NextSceneID;
		OutStage.Interactions = Scene->Interactions;
		return true;
	}
	return false;
}

FName UScenarioDefinition::GetStartStageID() const
{
	return !StartStageID.IsNone() ? StartStageID : StartSceneID;
}

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
	if (Stages.IsEmpty() && Scenes.IsEmpty())
	{
		OutError = FString::Printf(TEXT("Scenario %s has no stages."), *ScenarioID.ToString());
		return false;
	}

	if (!Stages.IsEmpty())
	{
		TSet<FName> StageIDs;
		for (const FScenarioStageDefinition& Stage : Stages)
		{
			if (StageIDs.Contains(Stage.StageID))
			{
				OutError = FString::Printf(TEXT("Scenario %s contains duplicate stage ID %s."),
					*ScenarioID.ToString(), *Stage.StageID.ToString());
				return false;
			}
			FString StageError;
			if (!Stage.ValidateStage(StageError))
			{
				OutError = StageError;
				return false;
			}
			StageIDs.Add(Stage.StageID);
		}
		if (StartStageID.IsNone() || !StageIDs.Contains(StartStageID))
		{
			OutError = FString::Printf(TEXT("Scenario %s has an invalid StartStageID %s."),
				*ScenarioID.ToString(), *StartStageID.ToString());
			return false;
		}
		for (const FScenarioStageDefinition& Stage : Stages)
		{
			if (!Stage.NextStageID.IsNone() && !StageIDs.Contains(Stage.NextStageID))
			{
				OutError = FString::Printf(TEXT("Stage %s references missing next stage %s."),
					*Stage.StageID.ToString(), *Stage.NextStageID.ToString());
				return false;
			}
		}
		OutError.Reset();
		return true;
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
