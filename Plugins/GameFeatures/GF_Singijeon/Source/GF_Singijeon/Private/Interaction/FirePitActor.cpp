#include "Interaction/FirePitActor.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/Scenario/ScenarioInteractableComponent.h"
#include "Core/Scenario/ScenarioTypes.h"
#include "Interaction/IgnitionSourceActor.h"
#include "Interaction/IgnitionSourceInterface.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

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

    static ConstructorHelpers::FObjectFinder<USoundBase> LightFireSound(
        TEXT("/GF_Singijeon/Asset/Sound/Effect/lightfire.lightfire"));
    TorchIgnitionSound = LightFireSound.Succeeded() ? LightFireSound.Object : nullptr;
}

void AFirePitActor::BeginPlay()
{
    Super::BeginPlay();
    SnapFireEffectToBowl();
    IgnitionArea->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::HandleIgnitionOverlap);
}

void AFirePitActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    SnapFireEffectToBowl();
}

void AFirePitActor::SnapFireEffectToBowl()
{
    TInlineComponentArray<UNiagaraComponent*> NiagaraComponents(this);
    for (UNiagaraComponent* NiagaraComponent : NiagaraComponents)
    {
        if (!IsValid(NiagaraComponent) || NiagaraComponent->GetFName() != TEXT("FireEffect"))
        {
            continue;
        }

        // Blueprint SCS and placed-actor overrides can otherwise restore an
        // outdated transform. The component remains relative to the Fire Pit
        // root, so it moves with the actor instead of simulating at world zero.
        NiagaraComponent->SetUsingAbsoluteLocation(false);
        NiagaraComponent->SetRelativeLocation(FireEffectRelativeLocation);
        NiagaraComponent->ReinitializeSystem();
    }
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

    if (!IgniteTorchScenarioInteractor->CanReportInteraction(
            EScenarioInteractionType::Trigger))
    {
        return false;
    }

    if (IIgnitionSourceInterface::Execute_IsIgnitionActive(Torch))
    {
        return IgniteTorchScenarioInteractor->ReportInteractionCompleted(
            EScenarioInteractionType::Trigger);
    }

    Torch->SetIgnitionActive(true);
    if (!IgniteTorchScenarioInteractor->ReportInteractionCompleted(
            EScenarioInteractionType::Trigger))
    {
        Torch->SetIgnitionActive(false);
        return false;
    }
    if (TorchIgnitionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, TorchIgnitionSound, Torch->GetActorLocation());
    }
    OnTorchIgnited.Broadcast(Torch);
    return true;
}
