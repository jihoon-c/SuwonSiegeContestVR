#include "Core/Experience/ExperienceDefinition.h"

#include "Core/Scenario/ScenarioDefinition.h"

bool UExperienceDefinition::ValidateDefinition(FString& OutError) const
{
	if (ExperienceID.IsNone())
	{
		OutError = TEXT("ExperienceID is None.");
		return false;
	}

	if (ExperienceLevel.IsNull())
	{
		OutError = FString::Printf(TEXT("Experience %s has no ExperienceLevel."), *ExperienceID.ToString());
		return false;
	}

	if (ScenarioDefinition && !ScenarioDefinition->ValidateScenario(OutError))
	{
		return false;
	}

	OutError.Reset();
	return true;
}
