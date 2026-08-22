#include "GF_Geojunggi.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "Modules/ModuleManager.h"
#include "Nokro/NokroScenarioManager.h"

void FGF_GeojunggiModule::StartupModule()
{
	WorldTickStartHandle = FWorldDelegates::OnWorldTickStart.AddRaw(this, &FGF_GeojunggiModule::HandleWorldTickStart);
}

void FGF_GeojunggiModule::ShutdownModule()
{
	FWorldDelegates::OnWorldTickStart.Remove(WorldTickStartHandle);
	BootstrappedWorlds.Reset();
}

void FGF_GeojunggiModule::HandleWorldTickStart(UWorld* World, ELevelTick TickType, float DeltaSeconds)
{
	if (!World || BootstrappedWorlds.Contains(World) ||
		(World->WorldType != EWorldType::Game && World->WorldType != EWorldType::PIE) ||
		!World->GetMapName().Contains(TEXT("LV_Nokro")))
	{
		return;
	}

	for (TActorIterator<ANokroScenarioManager> It(World); It; ++It)
	{
		BootstrappedWorlds.Add(World);
		return;
	}

	FActorSpawnParameters Params;
	Params.Name = TEXT("NokroScenarioManager_Auto");
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	World->SpawnActor<ANokroScenarioManager>(ANokroScenarioManager::StaticClass(), FTransform::Identity, Params);
	BootstrappedWorlds.Add(World);
}

IMPLEMENT_MODULE(FGF_GeojunggiModule, GF_Geojunggi)
