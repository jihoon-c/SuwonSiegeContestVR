#include "Core/Scenario/ScenarioNarrationBridgeComponent.h"

#include "Core/Narration/NarrationSequenceComponent.h"
#include "Core/Scenario/ScenarioManagerComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UScenarioNarrationBridgeComponent::UScenarioNarrationBridgeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UScenarioNarrationBridgeComponent::BeginPlay()
{
	Super::BeginPlay();
	if (bInitializeOnBeginPlay)
	{
		InitializeBridge();
	}
}

bool UScenarioNarrationBridgeComponent::InitializeBridge()
{
	Unbind();
	ScenarioManager = GetOwner() ? GetOwner()->FindComponentByClass<UScenarioManagerComponent>() : nullptr;
	if (!ScenarioManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("ScenarioNarrationBridge could not find ScenarioManager on its owner."));
		return false;
	}

	ScenarioManager->OnNarrationRequested.AddUniqueDynamic(this, &ThisClass::HandleNarrationRequested);
	ResolveNarrationSequence();
	return true;
}

void UScenarioNarrationBridgeComponent::CancelPendingNarration()
{
	PendingInteractionID = NAME_None;
	if (NarrationSequence)
	{
		NarrationSequence->StopSequence();
	}
}

void UScenarioNarrationBridgeComponent::HandleNarrationRequested(
	const FName NarrationID,
	const FName InteractionID)
{
	if (!NarrationTable)
	{
		// A Blueprint listener may own narration when no table is assigned to this optional bridge.
		return;
	}
	if (!ResolveNarrationSequence())
	{
		UE_LOG(LogTemp, Warning, TEXT("Scenario narration %s could not start because the player NarrationSequenceComponent was not found."),
			*NarrationID.ToString());
		if (ScenarioManager)
		{
			ScenarioManager->FailInteraction(InteractionID);
		}
		return;
	}

	PendingInteractionID = InteractionID;
	if (!NarrationSequence->PlaySequence(NarrationTable, NarrationID))
	{
		PendingInteractionID = NAME_None;
		ScenarioManager->FailInteraction(InteractionID);
	}
}

bool UScenarioNarrationBridgeComponent::ResolveNarrationSequence()
{
	if (NarrationSequence)
	{
		return true;
	}

	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		NarrationSequence = PlayerPawn->FindComponentByClass<UNarrationSequenceComponent>();
	}
	if (NarrationSequence)
	{
		NarrationSequence->OnSequenceFinished.AddUniqueDynamic(this, &ThisClass::HandleNarrationSequenceFinished);
		return true;
	}
	return false;
}

void UScenarioNarrationBridgeComponent::HandleNarrationSequenceFinished()
{
	if (ScenarioManager && !PendingInteractionID.IsNone())
	{
		const FName CompletedID = PendingInteractionID;
		PendingInteractionID = NAME_None;
		ScenarioManager->CompleteInteraction(CompletedID);
	}
}

void UScenarioNarrationBridgeComponent::Unbind()
{
	if (ScenarioManager)
	{
		ScenarioManager->OnNarrationRequested.RemoveDynamic(this, &ThisClass::HandleNarrationRequested);
	}
	if (NarrationSequence)
	{
		NarrationSequence->OnSequenceFinished.RemoveDynamic(this, &ThisClass::HandleNarrationSequenceFinished);
	}
	ScenarioManager = nullptr;
	NarrationSequence = nullptr;
	PendingInteractionID = NAME_None;
}

void UScenarioNarrationBridgeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Unbind();
	Super::EndPlay(EndPlayReason);
}
