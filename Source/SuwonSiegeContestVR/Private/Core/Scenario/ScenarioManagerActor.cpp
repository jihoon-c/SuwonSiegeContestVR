#include "Core/Scenario/ScenarioManagerActor.h"

#include "Components/SceneComponent.h"
#include "Core/Experience/ExperienceDefinition.h"
#include "Core/Scenario/ScenarioExperienceBridgeComponent.h"
#include "Core/Scenario/ScenarioDefinition.h"
#include "Core/Scenario/ScenarioManagerComponent.h"
#include "Core/Scenario/ScenarioNarrationBridgeComponent.h"

AScenarioManagerActor::AScenarioManagerActor()
{
	PrimaryActorTick.bCanEverTick = false;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
	ScenarioManager = CreateDefaultSubobject<UScenarioManagerComponent>(TEXT("ScenarioManager"));
	NarrationBridge = CreateDefaultSubobject<UScenarioNarrationBridgeComponent>(TEXT("NarrationBridge"));
	ExperienceBridge = CreateDefaultSubobject<UScenarioExperienceBridgeComponent>(TEXT("ExperienceBridge"));
}

void AScenarioManagerActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyConfiguration();
}

void AScenarioManagerActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	ApplyConfiguration();
}

void AScenarioManagerActor::BeginPlay()
{
	Super::BeginPlay();
	ApplyConfiguration();
	if (bAutoStartScenario)
	{
		if (StartConfiguredScenario() && ExperienceBridge)
		{
			ExperienceBridge->RestoreScenarioCheckpoint();
		}
	}
}

bool AScenarioManagerActor::StartConfiguredScenario()
{
	ApplyConfiguration();
	if (!ScenarioManager || !ScenarioDefinition)
	{
		UE_LOG(LogTemp, Warning, TEXT("ScenarioManagerActor %s cannot start: Scenario Definition is not assigned."), *GetName());
		return false;
	}
	return ScenarioManager->StartScenario(ScenarioDefinition);
}

void AScenarioManagerActor::SetScenarioDefinition(UScenarioDefinition* NewScenarioDefinition)
{
	StandaloneScenarioDefinition = NewScenarioDefinition;
	ApplyConfiguration();
}

void AScenarioManagerActor::RefreshResolvedConfiguration()
{
	ApplyConfiguration();
}

void AScenarioManagerActor::ApplyConfiguration()
{
	ScenarioDefinition = ExperienceDefinition && ExperienceDefinition->ScenarioDefinition
		? ExperienceDefinition->ScenarioDefinition.Get()
		: StandaloneScenarioDefinition.Get();
	NarrationTable = ScenarioDefinition ? ScenarioDefinition->NarrationTable.Get() : nullptr;
	if (ExperienceDefinition)
	{
		bAutoStartScenario = ExperienceDefinition->bAutoStartScenario;
		bCompleteExperienceOnScenarioFinished = ExperienceDefinition->bCompleteOnScenarioFinished;
	}
	else
	{
		bAutoStartScenario = ScenarioDefinition != nullptr;
		bCompleteExperienceOnScenarioFinished = false;
	}

	if (ScenarioManager)
	{
		ScenarioManager->ScenarioDefinition = ScenarioDefinition;
	}
	if (NarrationBridge)
	{
		NarrationBridge->NarrationTable = NarrationTable;
	}
	if (ExperienceBridge)
	{
		ExperienceBridge->ExperienceDefinition = ExperienceDefinition;
		ExperienceBridge->bActivateExperienceWhenOpenedDirectly = bActivateExperienceWhenOpenedDirectly;
		ExperienceBridge->bCompleteExperienceOnScenarioFinished = bCompleteExperienceOnScenarioFinished;
		ExperienceBridge->bRestoreScenarioCheckpoint = bRestoreScenarioCheckpoint;
	}
}
