#include "Core/Quiz/ScenarioQuizBridgeComponent.h"

#include "Core/Quiz/InitialConsonantQuizComponent.h"
#include "Core/Scenario/ScenarioManagerComponent.h"
#include "GameFramework/Actor.h"

UScenarioQuizBridgeComponent::UScenarioQuizBridgeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UScenarioQuizBridgeComponent::BeginPlay()
{
	Super::BeginPlay();
	if (bInitializeOnBeginPlay)
	{
		InitializeBridge();
	}
}

void UScenarioQuizBridgeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Unbind();
	Super::EndPlay(EndPlayReason);
}

bool UScenarioQuizBridgeComponent::InitializeBridge()
{
	Unbind();

	AActor* Owner = GetOwner();
	ScenarioManager = Owner ? Owner->FindComponentByClass<UScenarioManagerComponent>() : nullptr;
	if (!ScenarioManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("ScenarioQuizBridge could not find ScenarioManager on its owner."));
		return false;
	}

	QuizComponent = QuizComponentOverride
		? QuizComponentOverride.Get()
		: (Owner ? Owner->FindComponentByClass<UInitialConsonantQuizComponent>() : nullptr);
	if (!QuizComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("ScenarioQuizBridge could not find an InitialConsonantQuizComponent."));
		return false;
	}

	ScenarioManager->OnInteractionRequested.AddUniqueDynamic(this, &ThisClass::HandleInteractionRequested);
	QuizComponent->OnQuizFinished.AddUniqueDynamic(this, &ThisClass::HandleQuizFinished);
	return true;
}

void UScenarioQuizBridgeComponent::HandleInteractionRequested(FScenarioInteraction Interaction)
{
	if (Interaction.InteractionType != EScenarioInteractionType::Quiz || !QuizComponent)
	{
		return;
	}

	PendingInteractionID = Interaction.InteractionID;
	PendingQuizID = Interaction.TargetID;

	if (!QuizComponent->StartQuiz(Interaction.TargetID))
	{
		PendingInteractionID = NAME_None;
		PendingQuizID = NAME_None;
		if (ScenarioManager)
		{
			ScenarioManager->FailInteraction(Interaction.InteractionID);
		}
	}
}

void UScenarioQuizBridgeComponent::HandleQuizFinished(
	const FName QuizID, const bool bCorrect, const EInitialConsonantQuizOutcome Outcome)
{
	if (PendingInteractionID.IsNone() || QuizID != PendingQuizID)
	{
		return;
	}

	const FName InteractionID = PendingInteractionID;
	PendingInteractionID = NAME_None;
	PendingQuizID = NAME_None;

	if (!ScenarioManager || Outcome == EInitialConsonantQuizOutcome::Canceled)
	{
		return;
	}

	if (bCorrect || bCompleteInteractionOnWrongAnswer)
	{
		ScenarioManager->CompleteInteraction(InteractionID);
	}
	else
	{
		ScenarioManager->FailInteraction(InteractionID);
	}
}

void UScenarioQuizBridgeComponent::Unbind()
{
	if (ScenarioManager)
	{
		ScenarioManager->OnInteractionRequested.RemoveDynamic(this, &ThisClass::HandleInteractionRequested);
		ScenarioManager = nullptr;
	}
	if (QuizComponent)
	{
		QuizComponent->OnQuizFinished.RemoveDynamic(this, &ThisClass::HandleQuizFinished);
		QuizComponent = nullptr;
	}
	PendingInteractionID = NAME_None;
	PendingQuizID = NAME_None;
}
