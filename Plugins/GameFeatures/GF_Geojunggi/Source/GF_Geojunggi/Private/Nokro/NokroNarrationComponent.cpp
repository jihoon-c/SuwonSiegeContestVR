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
	Add(TEXT("ScenarioStarted"), TEXT("NK_31"));
	Add(TEXT("HandleGrabbed"), TEXT("NK_09"));
	Add(TEXT("HeightAdjusted"), TEXT("NK_12"));
	Add(TEXT("DirectionAdjusted"), TEXT("NK_16"));
	Add(TEXT("PlacementSucceeded"), TEXT("NK_17"));
	Add(TEXT("PlacementFailed"), TEXT("NK_20"), false);
	Add(TEXT("Progress"), TEXT("NK_23"), false);
	Add(TEXT("LastStone"), TEXT("NK_25"));
	Add(TEXT("ScenarioCompleted"), TEXT("NK_27"));
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
		NarrationSequence->OnNarrationStarted.AddUniqueDynamic(this, &ThisClass::HandleNarrationStarted);
		NarrationSequence->OnNarrationFinished.AddUniqueDynamic(this, &ThisClass::HandleNarrationFinished);
	}
	return NarrationSequence != nullptr;
}

void UNokroNarrationComponent::ResetNarrationState()
{
	if (bOwnsCurrentNarration && NarrationSequence)
	{
		NarrationSequence->StopSequence();
	}
	PendingRows.Reset();
	PlayedOnceEvents.Reset();
	bOwnsCurrentNarration = false;
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

void UNokroNarrationComponent::HandleNarrationStarted(const FName RowName)
{
	OnNarrationRowStarted.Broadcast(RowName);
}

void UNokroNarrationComponent::HandleNarrationFinished(const FName RowName)
{
	OnNarrationRowFinished.Broadcast(RowName);
}

void UNokroNarrationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (NarrationSequence)
	{
		NarrationSequence->OnSequenceFinished.RemoveDynamic(this, &ThisClass::HandleSequenceFinished);
		NarrationSequence->OnNarrationStarted.RemoveDynamic(this, &ThisClass::HandleNarrationStarted);
		NarrationSequence->OnNarrationFinished.RemoveDynamic(this, &ThisClass::HandleNarrationFinished);
	}
	PendingRows.Reset();
	Super::EndPlay(EndPlayReason);
}
