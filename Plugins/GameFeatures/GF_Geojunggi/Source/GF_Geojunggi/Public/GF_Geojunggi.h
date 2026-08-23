#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FGF_GeojunggiModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void HandleWorldTickStart(UWorld* World, ELevelTick TickType, float DeltaSeconds);
	FDelegateHandle WorldTickStartHandle;
	TSet<TWeakObjectPtr<UWorld>> BootstrappedWorlds;
};
