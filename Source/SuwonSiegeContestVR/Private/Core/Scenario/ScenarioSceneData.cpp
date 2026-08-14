#include "Core/Scenario/ScenarioSceneData.h"

const FScenarioInteraction* UScenarioSceneData::FindInteraction(const FName InteractionID) const
{
	return Interactions.FindByPredicate(
		[InteractionID](const FScenarioInteraction& Interaction)
		{
			return Interaction.InteractionID == InteractionID;
		});
}

bool UScenarioSceneData::ValidateScene(FString& OutError) const
{
	if (SceneID.IsNone())
	{
		OutError = TEXT("SceneID is None.");
		return false;
	}

	if (Interactions.IsEmpty())
	{
		OutError = FString::Printf(TEXT("Scene %s has no interactions."), *SceneID.ToString());
		return false;
	}

	TSet<FName> IDs;
	for (const FScenarioInteraction& Interaction : Interactions)
	{
		if (Interaction.InteractionID.IsNone())
		{
			OutError = FString::Printf(TEXT("Scene %s contains an interaction with no ID."), *SceneID.ToString());
			return false;
		}
		if (IDs.Contains(Interaction.InteractionID))
		{
			OutError = FString::Printf(TEXT("Scene %s contains duplicate interaction ID %s."),
				*SceneID.ToString(), *Interaction.InteractionID.ToString());
			return false;
		}
		IDs.Add(Interaction.InteractionID);
	}

	if (StartInteractionID.IsNone() || !IDs.Contains(StartInteractionID))
	{
		OutError = FString::Printf(TEXT("Scene %s has an invalid StartInteractionID %s."),
			*SceneID.ToString(), *StartInteractionID.ToString());
		return false;
	}

	for (const FScenarioInteraction& Interaction : Interactions)
	{
		for (const FName Link : {Interaction.NextInteractionID, Interaction.SuccessInteractionID, Interaction.FailInteractionID})
		{
			if (!Link.IsNone() && !IDs.Contains(Link))
			{
				OutError = FString::Printf(TEXT("Interaction %s references missing interaction %s in scene %s."),
					*Interaction.InteractionID.ToString(), *Link.ToString(), *SceneID.ToString());
				return false;
			}
		}
	}

	OutError.Reset();
	return true;
}
