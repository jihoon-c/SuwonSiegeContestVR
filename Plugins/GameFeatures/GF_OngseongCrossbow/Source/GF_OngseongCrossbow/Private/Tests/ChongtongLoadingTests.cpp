#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Ongseong/ChongtongCannonActor.h"
#include "Ongseong/ChongtongAutomaticFireComponent.h"
#include "Ongseong/ChongtongLoadingItemActor.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FChongtongLoadingSequenceTest,
	"SuwonSiegeContestVR.Ongseong.Chongtong.LoadingSequence",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FChongtongLoadingSequenceTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("Test world is created"), World)) return false;
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);

	AChongtongCannonActor* Cannon = World->SpawnActor<AChongtongCannonActor>();
	AChongtongLoadingItemActor* LoadingItem = World->SpawnActor<AChongtongLoadingItemActor>();
	if (TestNotNull(TEXT("Loading item is spawned without constructor-helper crash"), LoadingItem))
	{
		UStaticMeshComponent* ItemMesh = LoadingItem->FindComponentByClass<UStaticMeshComponent>();
		TestTrue(TEXT("The visible loading prop is a direct VR grab target"),
			ItemMesh && ItemMesh->ComponentHasTag(TEXT("VRGrab")));
		TestNotNull(TEXT("The visible-prop grab forwarding hook exists"),
			LoadingItem->FindFunction(TEXT("HandleVRGrabbed")));
		TestNotNull(TEXT("The visible-prop release forwarding hook exists"),
			LoadingItem->FindFunction(TEXT("HandleVRReleased")));
		LoadingItem->ConfigureItem(EChongtongLoadingItemType::Powder);
		LoadingItem->ConfigureItem(EChongtongLoadingItemType::Rammer);
		LoadingItem->ConfigureItem(EChongtongLoadingItemType::Cannonball);
		TestFalse(TEXT("An ungrabbed loading prop cannot be inserted"), LoadingItem->IsHeldForInteraction());
	}
	if (TestNotNull(TEXT("Cannon is spawned"), Cannon))
	{
		const UChongtongAutomaticFireComponent* AutomaticFire = Cannon->FindComponentByClass<UChongtongAutomaticFireComponent>();
		if (TestNotNull(TEXT("Cannon composes the automatic-fire behavior"), AutomaticFire))
		{
			TestFalse(TEXT("Automatic fire is opt-in for Blueprint variants"), AutomaticFire->IsAutomaticFireEnabled());
			TestEqual(TEXT("Allied automatic-fire cooldown defaults to five seconds"), AutomaticFire->GetFireInterval(), 5.0f);
		}
		TestEqual(TEXT("Initial step requests powder"), Cannon->GetLoadingState(), EChongtongLoadingState::NeedsPowder);
		TestFalse(TEXT("Cannonball cannot skip powder"), Cannon->TryLoadItem(EChongtongLoadingItemType::Cannonball));
		TestTrue(TEXT("Powder is accepted"), Cannon->TryLoadItem(EChongtongLoadingItemType::Powder));
		TestEqual(TEXT("Powder advances to ramming"), Cannon->GetLoadingState(), EChongtongLoadingState::NeedsRamming);
		TestTrue(TEXT("First ram is counted"), Cannon->RegisterRammerStroke());
		TestTrue(TEXT("Second ram is counted"), Cannon->RegisterRammerStroke());
		TestTrue(TEXT("Third ram is counted"), Cannon->RegisterRammerStroke());
		TestEqual(TEXT("Three rams unlock cannonball"), Cannon->GetLoadingState(), EChongtongLoadingState::NeedsCannonball);
		TestTrue(TEXT("Cannonball is accepted"), Cannon->TryLoadItem(EChongtongLoadingItemType::Cannonball));
		TestEqual(TEXT("Completed load enters ready state"), Cannon->GetLoadingState(), EChongtongLoadingState::ReadyToAim);
		TestFalse(TEXT("Player fire is gated by a two-hand grip"), Cannon->TryFirePlayer());
	}

	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}

#endif
