#include "Interaction/FirePitActor.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/Scenario/ScenarioInteractableComponent.h"
#include "Core/Scenario/ScenarioTypes.h"
#include "Interaction/IgnitionSourceActor.h"
#include "Interaction/IgnitionSourceInterface.h"

AFirePitActor::AFirePitActor()
{
    PrimaryActorTick.bCanEverTick = false;

    FirePitMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FirePitMesh"));
    SetRootComponent(FirePitMesh);
    FirePitMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

    IgnitionArea = CreateDefaultSubobject<USphereComponent>(TEXT("IgnitionArea"));
    IgnitionArea->SetupAttachment(FirePitMesh);
    IgnitionArea->InitSphereRadius(45.0f);
    IgnitionArea->SetCollisionProfileName(TEXT("Trigger"));
    IgnitionArea->SetGenerateOverlapEvents(true);

    IgniteTorchScenarioInteractor = CreateDefaultSubobject<UScenarioInteractableComponent>(
        TEXT("IgniteTorchScenarioInteractor"));
    IgniteTorchScenarioInteractor->TargetID = TEXT("Torch_Ignite");
    IgniteTorchScenarioInteractor->SupportedInteractionTypes = {
        EScenarioInteractionType::Trigger
    };
}

void AFirePitActor::BeginPlay()
{
    Super::BeginPlay();
    IgnitionArea->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::HandleIgnitionOverlap);
}

void AFirePitActor::HandleIgnitionOverlap(UPrimitiveComponent*, AActor* OtherActor,
    UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    TryIgniteTorch(OtherActor);
}

bool AFirePitActor::TryIgniteTorch(AActor* Candidate)
{
    AIgnitionSourceActor* Torch = Cast<AIgnitionSourceActor>(Candidate);
    if (!IsValid(Torch))
    {
        return false;
    }

    // When a Scenario Manager exists it must accept Torch_Ignite before the
    // physical state changes. This prevents an accidental early overlap from
    // lighting the torch and leaving INT_05 unable to complete later.
    if (!IgniteTorchScenarioInteractor->ReportInteractionCompleted(
            EScenarioInteractionType::Trigger))
    {
        return false;
    }

    if (IIgnitionSourceInterface::Execute_IsIgnitionActive(Torch))
    {
        return true;
    }

    Torch->SetIgnitionActive(true);
    OnTorchIgnited.Broadcast(Torch);
    return true;
}
