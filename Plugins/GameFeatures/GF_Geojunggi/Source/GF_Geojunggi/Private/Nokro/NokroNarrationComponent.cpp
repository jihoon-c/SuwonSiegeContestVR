#include "Nokro/NokroNarrationComponent.h"

#include "Core/Narration/NarrationSequenceComponent.h"
#include "Engine/DataTable.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UNokroNarrationComponent::UNokroNarrationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	NarrationTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/GF_Geojunggi/Data/DT_NokroNarration.DT_NokroNarration")));
	const auto Add = [this](const TCHAR* Event, const TCHAR* Row, const bool bOnce = true)
	{
		FNokroNarrationEventBinding& Binding = EventBindings.AddDefaulted_GetRef();
		Binding.EventName = FName(Event);
		Binding.NarrationRow = FName(Row);
		Binding.bPlayOnce = bOnce;
	};
	Add(TEXT("ScenarioStarted"), TEXT("NK_01"));
	Add(TEXT("HandleGrabbed"), TEXT("NK_09"));
	Add(TEXT("HeightAdjusted"), TEXT("NK_12"));
	Add(TEXT("DirectionAdjusted"), TEXT("NK_16"));
	Add(TEXT("PlacementSucceeded"), TEXT("NK_18"), false);
	Add(TEXT("PlacementFailed"), TEXT("NK_21"), false);
	Add(TEXT("Progress"), TEXT("NK_24"), false);
	Add(TEXT("LastStone"), TEXT("NK_26"));
	Add(TEXT("ScenarioCompleted"), TEXT("NK_28"));
}

bool UNokroNarrationComponent::InitializeNarration()
{
	if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		NarrationSequence = Pawn->FindComponentByClass<UNarrationSequenceComponent>();
	}
	if (NarrationSequence)
	{
		NarrationSequence->OnSequenceFinished.AddUniqueDynamic(this, &ThisClass::HandleSequenceFinished);
	}
	return NarrationSequence != nullptr;
}

void UNokroNarrationComponent::ReportScenarioEvent(const FName EventName)
{
	const FNokroNarrationEventBinding* Binding = EventBindings.FindByPredicate([EventName](const FNokroNarrationEventBinding& Candidate)
	{
		return Candidate.EventName == EventName;
	});
	if (!Binding || (Binding->bPlayOnce && PlayedOnceEvents.Contains(EventName))) return;
	if (Binding->bPlayOnce) PlayedOnceEvents.Add(EventName);
	PendingRows.Add(Binding->NarrationRow);
	TryPlayNext();
}

void UNokroNarrationComponent::TryPlayNext()
{
	if (!NarrationSequence) InitializeNarration();
	if (!NarrationSequence || bOwnsCurrentNarration || NarrationSequence->IsNarrationPlaying() || PendingRows.IsEmpty()) return;
	UDataTable* Table = NarrationTable.LoadSynchronous();
	const FName Row = PendingRows[0];
	PendingRows.RemoveAt(0);
	bOwnsCurrentNarration = Table && NarrationSequence->PlaySequence(Table, Row);
	if (!bOwnsCurrentNarration)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not play Nokro narration row %s."), *Row.ToString());
		TryPlayNext();
	}
}

void UNokroNarrationComponent::HandleSequenceFinished()
{
	if (bOwnsCurrentNarration) bOwnsCurrentNarration = false;
	TryPlayNext();
}

void UNokroNarrationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (NarrationSequence)
	{
		NarrationSequence->OnSequenceFinished.RemoveDynamic(this, &ThisClass::HandleSequenceFinished);
	}
	PendingRows.Reset();
	Super::EndPlay(EndPlayReason);
}
