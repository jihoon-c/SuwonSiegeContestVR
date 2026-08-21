#include "Interaction/IgnitionSourceActor.h"

#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/Scenario/ScenarioInteractableComponent.h"
#include "Core/Scenario/ScenarioTypes.h"
#include "Particles/ParticleSystemComponent.h"

AIgnitionSourceActor::AIgnitionSourceActor()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    SourceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SourceMesh"));
    SourceMesh->SetupAttachment(SceneRoot);
    SourceMesh->SetCollisionProfileName(TEXT("PhysicsActor"));

    IgnitionArea = CreateDefaultSubobject<USphereComponent>(TEXT("IgnitionArea"));
    IgnitionArea->SetupAttachment(SourceMesh);
    IgnitionArea->InitSphereRadius(8.0f);
    IgnitionArea->SetCollisionProfileName(TEXT("Trigger"));

    GrabScenarioInteractor = CreateDefaultSubobject<UScenarioInteractableComponent>(TEXT("GrabScenarioInteractor"));
    GrabScenarioInteractor->TargetID = TEXT("Singijeon_Torch");
    GrabScenarioInteractor->SupportedInteractionTypes = { EScenarioInteractionType::Grab };
}

void AIgnitionSourceActor::BeginPlay()
{
    Super::BeginPlay();
    RefreshIgnitionVisuals();
}

bool AIgnitionSourceActor::IsIgnitionActive_Implementation() const
{
    return bIgnitionActive;
}

void AIgnitionSourceActor::SetIgnitionActive(const bool bNewActive)
{
    if (bIgnitionActive == bNewActive)
    {
        return;
    }

    bIgnitionActive = bNewActive;
    RefreshIgnitionVisuals();
    OnIgnitionSourceStateChanged.Broadcast(bIgnitionActive);
}

void AIgnitionSourceActor::RefreshIgnitionVisuals()
{
    TInlineComponentArray<UFXSystemComponent*> Effects(this);
    for (UFXSystemComponent* Effect : Effects)
    {
        if (!IsValid(Effect))
        {
            continue;
        }

        if (bIgnitionActive)
        {
            Effect->Activate(true);
        }
        else
        {
            Effect->DeactivateImmediate();
        }
    }
}
