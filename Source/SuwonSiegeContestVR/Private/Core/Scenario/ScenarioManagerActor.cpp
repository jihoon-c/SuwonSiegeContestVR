#include "Core/Scenario/ScenarioManagerActor.h"

#include "Components/SceneComponent.h"
#include "Core/Experience/ExperienceDefinition.h"
#include "Core/Scenario/ScenarioExperienceBridgeComponent.h"
#include "Core/Scenario/ScenarioInteractionGuideComponent.h"
#include "Core/Scenario/ScenarioDefinition.h"
#include "Core/Scenario/ScenarioManagerComponent.h"
#include "Core/Scenario/ScenarioNarrationBridgeComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"

AScenarioManagerActor::AScenarioManagerActor()
{
	PrimaryActorTick.bCanEverTick = false;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
	ScenarioManager = CreateDefaultSubobject<UScenarioManagerComponent>(TEXT("ScenarioManager"));
	NarrationBridge = CreateDefaultSubobject<UScenarioNarrationBridgeComponent>(TEXT("NarrationBridge"));
	ExperienceBridge = CreateDefaultSubobject<UScenarioExperienceBridgeComponent>(TEXT("ExperienceBridge"));
	InteractionGuide = CreateDefaultSubobject<UScenarioInteractionGuideComponent>(TEXT("InteractionGuide"));
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
	SetupDebugInput();
	if (bAutoStartScenario && ShouldAutoStartScenario())
	{
		if (StartConfiguredScenario() && ExperienceBridge)
		{
			ExperienceBridge->RestoreScenarioCheckpoint();
		}
	}
}

void AScenarioManagerActor::SetupDebugInput()
{
#if !UE_BUILD_SHIPPING
	if (!bEnableSpacebarDebugAdvance || bDebugInputBound)
	{
		return;
	}

	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		EnableInput(PlayerController);
		if (InputComponent)
		{
			FInputKeyBinding& Binding = InputComponent->BindKey(
				EKeys::SpaceBar, IE_Pressed, this, &ThisClass::HandleSpacebarDebugAdvance);
			Binding.bConsumeInput = false;
			bDebugInputBound = true;
		}
	}
#endif
}

void AScenarioManagerActor::HandleSpacebarDebugAdvance()
{
	DebugAdvanceCurrentInteraction();
}

bool AScenarioManagerActor::DebugAdvanceCurrentInteraction()
{
#if UE_BUILD_SHIPPING
	return false;
#else
	if (!bEnableSpacebarDebugAdvance || !ScenarioManager)
	{
		return false;
	}

	const FScenarioDebugSnapshot Before = ScenarioManager->GetDebugSnapshot();
	if (Before.InteractionState != EScenarioInteractionState::Running)
	{
		return false;
	}

	if (NarrationBridge)
	{
		NarrationBridge->CancelPendingNarration();
	}
	const bool bAdvanced = ScenarioManager->CompleteCurrentInteraction();
	UE_LOG(LogTemp, Display, TEXT("Scenario debug Space advance: %s/%s/%s -> %s"),
		*Before.ScenarioID.ToString(), *Before.SceneID.ToString(), *Before.InteractionID.ToString(),
		bAdvanced ? TEXT("Success") : TEXT("Rejected"));
	return bAdvanced;
#endif
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
	NarrationTable = LevelNarrationTable
		? LevelNarrationTable.Get()
		: (ScenarioDefinition ? ScenarioDefinition->NarrationTable.Get() : nullptr);
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
