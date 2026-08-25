#pragma once

#include "Modules/ModuleManager.h"

class FComponentVisualizer;

class FSuwonSiegeContestVREditorModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterMenus();

	TSharedPtr<FComponentVisualizer> ScenarioGuideVisualizer;
};
