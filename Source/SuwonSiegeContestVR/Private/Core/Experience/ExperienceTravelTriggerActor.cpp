#include "Core/Experience/ExperienceTravelTriggerActor.h"

#include "Components/BoxComponent.h"
#include "Core/Experience/ExperienceDefinition.h"
#include "Core/Experience/ExperienceSubsystem.h"
#include "Core/Scenario/ScenarioManagerComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"

AExperienceTravelTriggerActor::AExperienceTravelTriggerActor()
{
	PrimaryActorTick.bCanEverTick = true;
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	SetRootComponent(TriggerBox);
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::HandleBeginOverlap);
}

void AExperienceTravelTriggerActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bTriggerOnPlayerViewLocation || !TriggerBox || (bTriggerOnce && bHasTriggered))
	{
		bPlayerViewWasInsideTrigger = false;
		return;
	}

	const APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!CameraManager && !PlayerPawn)
	{
		bPlayerViewWasInsideTrigger = false;
		return;
	}

	const FVector PlayerViewLocation = CameraManager
		? CameraManager->GetCameraLocation()
		: PlayerPawn->GetActorLocation();
	const FVector LocalLocation = TriggerBox->GetComponentTransform().InverseTransformPosition(PlayerViewLocation);
	const FVector BoxExtent = TriggerBox->GetUnscaledBoxExtent();
	const bool bIsInsideTrigger =
		FMath::Abs(LocalLocation.X) <= BoxExtent.X &&
		FMath::Abs(LocalLocation.Y) <= BoxExtent.Y &&
		FMath::Abs(LocalLocation.Z) <= BoxExtent.Z;

	if (bIsInsideTrigger && !bPlayerViewWasInsideTrigger)
	{
		TriggerExperienceTravel(const_cast<APawn*>(PlayerPawn));
	}

	bPlayerViewWasInsideTrigger = bIsInsideTrigger;
}

void AExperienceTravelTriggerActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (TriggerBox)
	{
		TriggerBox->SetBoxExtent(TriggerExtent);
	}
}

bool AExperienceTravelTriggerActor::TriggerExperienceTravel(AActor* TriggeringActor)
{
	if ((bTriggerOnce && bHasTriggered) || !DestinationExperience ||
		!IsInteractionRequirementMet())
	{
		return false;
	}

	UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UExperienceSubsystem* ExperienceSubsystem = GameInstance
		? GameInstance->GetSubsystem<UExperienceSubsystem>()
		: nullptr;
	if (!ExperienceSubsystem)
	{
		return false;
	}

	if (!ExperienceSubsystem->SetScenarioResumeCheckpoint(
		ReturnScenarioID, ReturnSceneID, ReturnInteractionID))
	{
		return false;
	}

	if (!ExperienceSubsystem->StartExperience(DestinationExperience, false))
	{
		ExperienceSubsystem->ClearScenarioResumeCheckpoint(ReturnScenarioID);
		return false;
	}

	bHasTriggered = true;
	return true;
}

bool AExperienceTravelTriggerActor::IsInteractionRequirementMet() const
{
	if (RequiredInteractionID.IsNone())
	{
		return true;
	}

	if (!GetWorld())
	{
		return false;
	}

	for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
	{
		if (const UScenarioManagerComponent* Manager =
			ActorIterator->FindComponentByClass<UScenarioManagerComponent>())
		{
			return Manager->GetCurrentInteraction().InteractionID == RequiredInteractionID;
		}
	}

	return false;
}

void AExperienceTravelTriggerActor::HandleBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bTriggerOnPawnOverlap && Cast<APawn>(OtherActor))
	{
		TriggerExperienceTravel(OtherActor);
	}
}
