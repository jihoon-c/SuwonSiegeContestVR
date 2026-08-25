#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/VR/InteractionHighlightComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Ongseong/ChongtongCannonActor.h"
#include "Ongseong/ChongtongLoadingItemActor.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FOngseongInteractionPromptTest,
	"SuwonSiegeContestVR.Ongseong.Interaction.Prompts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FOngseongInteractionPromptTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("Test world is created"), World)) return false;
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);

	AChongtongLoadingItemActor* Item = World->SpawnActor<AChongtongLoadingItemActor>();
	if (TestNotNull(TEXT("Loading item is spawned"), Item))
	{
		TestNotNull(TEXT("Loading item carries a Core interaction highlight"),
			Item->FindComponentByClass<UInteractionHighlightComponent>());
		TestFalse(TEXT("The prompt starts off"), Item->IsLoadingPromptActive());
		Item->SetLoadingPromptActive(true);
		TestTrue(TEXT("The prompt turns on for the required item"), Item->IsLoadingPromptActive());
		Item->SetLoadingPromptActive(false);
		TestFalse(TEXT("The prompt turns off again"), Item->IsLoadingPromptActive());

		// A consumed item is hidden until it respawns, and must not glow while invisible.
		Item->SetActorHiddenInGame(true);
		Item->SetLoadingPromptActive(true);
		TestFalse(TEXT("A hidden item never glows"), Item->IsLoadingPromptActive());
	}

	AChongtongCannonActor* Cannon = World->SpawnActor<AChongtongCannonActor>();
	if (TestNotNull(TEXT("Chongtong is spawned"), Cannon))
	{
		TestNotNull(TEXT("Chongtong marks its two-hand grip with a Core interaction highlight"),
			Cannon->FindComponentByClass<UInteractionHighlightComponent>());
	}

	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}

#endif
