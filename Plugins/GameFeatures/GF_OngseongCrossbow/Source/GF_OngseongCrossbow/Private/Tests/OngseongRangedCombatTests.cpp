#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Gameplay/Combat/CombatTypes.h"
#include "Ongseong/OngseongArcherCombatComponent.h"
#include "Ongseong/OngseongBoltProjectileActor.h"
#include "Ongseong/OngseongCrossbowActor.h"
#include "Ongseong/OngseongGateActor.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FOngseongRangedCombatContractsTest,
	"SuwonSiegeContestVR.Ongseong.RangedCombat.Contracts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FOngseongRangedCombatContractsTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("Test world is created"), World)) return false;
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);

	AOngseongCrossbowActor* Crossbow = World->SpawnActor<AOngseongCrossbowActor>();
	AOngseongBoltProjectileActor* Bolt = World->SpawnActor<AOngseongBoltProjectileActor>();
	TestNotNull(TEXT("Mounted crossbow can be spawned without content assets"), Crossbow);
	TestNotNull(TEXT("Physical bolt can be spawned without content assets"), Bolt);
	if (Crossbow)
	{
		TestEqual(TEXT("Crossbow starts loaded"), Crossbow->GetCrossbowState(), EOngseongCrossbowState::Loaded);
		TestTrue(TEXT("Crossbow supports timed reload"), Crossbow->BeginReload());
		TestEqual(TEXT("Reloading state is observable"), Crossbow->GetCrossbowState(), EOngseongCrossbowState::Reloading);
		Crossbow->CompleteReload();
		TestEqual(TEXT("Reload completes back to loaded"), Crossbow->GetCrossbowState(), EOngseongCrossbowState::Loaded);
	}

	AOngseongGateActor* Primary = World->SpawnActor<AOngseongGateActor>();
	AOngseongGateActor* Fallback = World->SpawnActor<AOngseongGateActor>();
	AActor* ArcherOwner = World->SpawnActor<AActor>();
	UOngseongArcherCombatComponent* ArcherCombat = NewObject<UOngseongArcherCombatComponent>(ArcherOwner);
	ArcherCombat->RegisterComponent();
	ArcherCombat->ConfigureCombat(Primary, Fallback, nullptr);
	TestEqual(TEXT("Archer prioritizes the living cannon target"), ArcherCombat->GetCurrentTarget(), static_cast<AActor*>(Primary));
	FCombatDamageSpec FatalDamage;
	FatalDamage.Amount = 1000.0f;
	FatalDamage.bIgnoreFaction = true;
	Primary->ReceiveCombatDamage_Implementation(FatalDamage);
	TestEqual(TEXT("Archer falls back when the cannon is destroyed"), ArcherCombat->GetCurrentTarget(), static_cast<AActor*>(Fallback));

	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}

#endif
