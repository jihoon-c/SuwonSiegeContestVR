#include "Interaction/IgnitionSourceActor.h"

#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/Scenario/ScenarioInteractableComponent.h"
#include "Core/Scenario/ScenarioTypes.h"
#include "NiagaraComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "UObject/ConstructorHelpers.h"

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

    TorchFlameEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TorchFlameEffect"));
    TorchFlameEffect->SetupAttachment(IgnitionArea);
    TorchFlameEffect->SetRelativeLocation(FVector::ZeroVector);
    TorchFlameEffect->SetRelativeScale3D(FVector(0.18f));
    TorchFlameEffect->bAutoActivate = false;
    TorchFlameEffect->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    static ConstructorHelpers::FObjectFinder<UParticleSystem> PointFireTemplate(
        TEXT("/Game/StarterContent/Particles/P_Fire.P_Fire"));
    if (PointFireTemplate.Succeeded())
    {
        TorchFlameEffect->SetTemplate(PointFireTemplate.Object);
    }
}

void AIgnitionSourceActor::BeginPlay()
{
    Super::BeginPlay();
    PrepareIgnitionVisuals();
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

        if (UNiagaraComponent* NiagaraEffect = Cast<UNiagaraComponent>(Effect))
        {
            if (bUsePointSourceFlame)
            {
                NiagaraEffect->SetRenderingEnabled(false);
                NiagaraEffect->SetPaused(true);
                NiagaraEffect->DeactivateImmediate();
                continue;
            }
            if (bIgnitionActive)
            {
                // The instance is already allocated and warmed by BeginPlay. Do not
                // reset it on overlap; simply resume simulation and rendering.
                if (!NiagaraEffect->IsActive())
                {
                    NiagaraEffect->Activate(false);
                }
                NiagaraEffect->SetPaused(false);
                NiagaraEffect->SetRenderingEnabled(true);
            }
            else
            {
                NiagaraEffect->SetRenderingEnabled(false);
                NiagaraEffect->SetPaused(true);
            }
        }
        else if (bIgnitionActive)
        {
            Effect->Activate(false);
        }
        else
        {
            Effect->DeactivateImmediate();
        }
    }
}

void AIgnitionSourceActor::PrepareIgnitionVisuals()
{
    bIgnitionVisualsPrepared = false;
    TInlineComponentArray<UNiagaraComponent*> NiagaraEffects(this);
    for (UNiagaraComponent* NiagaraEffect : NiagaraEffects)
    {
        if (!IsValid(NiagaraEffect) || !NiagaraEffect->GetAsset())
        {
            continue;
        }

        if (bUsePointSourceFlame)
        {
            NiagaraEffect->SetRenderingEnabled(false);
            NiagaraEffect->SetPaused(true);
            NiagaraEffect->DeactivateImmediate();
            continue;
        }

        NiagaraEffect->SetAllowScalability(true);
        NiagaraEffect->SetCullDistance(FMath::Max(0.0f, IgnitionEffectCullDistance));
        NiagaraEffect->SetRenderingEnabled(false);
        if (!NiagaraEffect->IsActive())
        {
            NiagaraEffect->Activate(false);
        }
        if (IgnitionEffectWarmupTicks > 0)
        {
            NiagaraEffect->AdvanceSimulation(IgnitionEffectWarmupTicks, 1.0f / 30.0f);
        }
        NiagaraEffect->SetPaused(true);
        bIgnitionVisualsPrepared = true;
    }

    if (bUsePointSourceFlame && TorchFlameEffect && TorchFlameEffect->Template)
    {
        TorchFlameEffect->DeactivateSystem();
        bIgnitionVisualsPrepared = true;
    }
}
