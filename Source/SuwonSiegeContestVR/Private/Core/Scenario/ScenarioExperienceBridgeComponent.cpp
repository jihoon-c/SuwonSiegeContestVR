#include "Core/Scenario/ScenarioExperienceBridgeComponent.h"

#include "Core/Experience/ExperienceDefinition.h"
#include "Core/Experience/ExperienceSubsystem.h"
#include "Core/Scenario/ScenarioManagerComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

UScenarioExperienceBridgeComponent::UScenarioExperienceBridgeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UScenarioExperienceBridgeComponent::BeginPlay()
{
	Super::BeginPlay();
	if (bInitializeOnBeginPlay)
	{
		InitializeBridge();
	}
}

bool UScenarioExperienceBridgeComponent::InitializeBridge()
{
	Unbind();
	ScenarioManager = GetOwner() ? GetOwner()->FindComponentByClass<UScenarioManagerComponent>() : nullptr;
	if (!ScenarioManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("ScenarioExperienceBridge could not find ScenarioManager on its owner."));
		return false;
	}

	ScenarioManager->OnScenarioFinished.AddUniqueDynamic(this, &ThisClass::HandleScenarioFinished);

	UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UExperienceSubsystem* ExperienceSubsystem = GameInstance
		? GameInstance->GetSubsystem<UExperienceSubsystem>()
		: nullptr;
	if (bActivateExperienceWhenOpenedDirectly && ExperienceSubsystem && ExperienceDefinition &&
		ExperienceSubsystem->GetCurrentExperienceID() != ExperienceDefinition->ExperienceID)
	{
		ExperienceSubsystem->ActivateExperienceForCurrentLevel(ExperienceDefinition);
	}
	return true;
}

void UScenarioExperienceBridgeComponent::HandleScenarioFinished()
{
	if (!bCompleteExperienceOnScenarioFinished)
	{
		return;
	}

	UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UExperienceSubsystem* ExperienceSubsystem = GameInstance
		? GameInstance->GetSubsystem<UExperienceSubsystem>()
		: nullptr;
	if (!ExperienceSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("ScenarioExperienceBridge could not find ExperienceSubsystem."));
		return;
	}

	const FName ExperienceID = ExperienceDefinition
		? ExperienceDefinition->ExperienceID
		: ExperienceSubsystem->GetCurrentExperienceID();
	if (!ExperienceSubsystem->CompleteExperience(ExperienceID, true))
	{
		UE_LOG(LogTemp, Warning, TEXT("ScenarioExperienceBridge could not complete Experience %s."),
			*ExperienceID.ToString());
	}
}

void UScenarioExperienceBridgeComponent::Unbind()
{
	if (ScenarioManager)
	{
		ScenarioManager->OnScenarioFinished.RemoveDynamic(this, &ThisClass::HandleScenarioFinished);
	}
	ScenarioManager = nullptr;
}

void UScenarioExperienceBridgeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Unbind();
	Super::EndPlay(EndPlayReason);
}
