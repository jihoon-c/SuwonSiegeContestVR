#include "Singijeon/FuseIgnitionComponent.h"

#include "Engine/World.h"
#include "Interaction/IgnitionSourceInterface.h"
#include "Singijeon/SingijeonHwachaActor.h"

UFuseIgnitionComponent::UFuseIgnitionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    // A 12 cm target was too small for a held torch in VR. The wider overlap
    // still requires an active IgnitionSource, so it does not weaken ordering.
    InitSphereRadius(24.0f);
    SetCollisionProfileName(TEXT("Trigger"));
    SetGenerateOverlapEvents(true);
}

void UFuseIgnitionComponent::BeginPlay()
{
    Super::BeginPlay();
    OnComponentBeginOverlap.AddDynamic(this, &ThisClass::HandleBeginOverlap);
    OnComponentEndOverlap.AddDynamic(this, &ThisClass::HandleEndOverlap);
}

void UFuseIgnitionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!IsValidActiveSource(ActiveSource) || !IsOverlappingActor(ActiveSource))
    {
        CancelIgnition();
        return;
    }

    if (GetIgnitionProgress() >= 1.0f)
    {
        CompleteIgnition();
    }
}

void UFuseIgnitionComponent::HandleBeginOverlap(UPrimitiveComponent*, AActor* OtherActor,
    UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    TryBeginIgnition(OtherActor);
}

void UFuseIgnitionComponent::HandleEndOverlap(UPrimitiveComponent*, AActor* OtherActor,
    UPrimitiveComponent*, int32)
{
    if (OtherActor == ActiveSource)
    {
        CancelIgnition();
    }
}

bool UFuseIgnitionComponent::TryBeginIgnition(AActor* SourceActor)
{
    const ASingijeonHwachaActor* Hwacha = Cast<ASingijeonHwachaActor>(GetOwner());
    if (!bIgnitionEnabled || IsValid(ActiveSource) || !IsValidActiveSource(SourceActor))
    {
        return false;
    }
    if (Hwacha && !Hwacha->CanBeginFuseIgnitionNow())
    {
        return false;
    }

    ActiveSource = SourceActor;
    IgnitionStartTime = GetWorld()->GetTimeSeconds();
    SetComponentTickEnabled(true);
    OnIgnitionStarted.Broadcast(SourceActor);
    return true;
}

void UFuseIgnitionComponent::CancelIgnition()
{
    if (!IsValid(ActiveSource))
    {
        SetComponentTickEnabled(false);
        return;
    }

    AActor* PreviousSource = ActiveSource;
    ActiveSource = nullptr;
    SetComponentTickEnabled(false);
    OnIgnitionCanceled.Broadcast(PreviousSource);
}

void UFuseIgnitionComponent::SetIgnitionEnabled(const bool bEnabled)
{
    bIgnitionEnabled = bEnabled;
    if (!bIgnitionEnabled)
    {
        CancelIgnition();
        return;
    }

    TArray<AActor*> OverlappingActors;
    GetOverlappingActors(OverlappingActors);
    for (AActor* OverlappingActor : OverlappingActors)
    {
        if (TryBeginIgnition(OverlappingActor))
        {
            break;
        }
    }
}

float UFuseIgnitionComponent::GetIgnitionProgress() const
{
    if (!IsValid(ActiveSource) || !GetWorld())
    {
        return 0.0f;
    }

    if (IgnitionDuration <= KINDA_SMALL_NUMBER)
    {
        return 1.0f;
    }

    return FMath::Clamp((GetWorld()->GetTimeSeconds() - IgnitionStartTime) / IgnitionDuration, 0.0f, 1.0f);
}

void UFuseIgnitionComponent::CompleteIgnition()
{
    ActiveSource = nullptr;
    bIgnitionEnabled = false;
    SetComponentTickEnabled(false);
    OnIgnited.Broadcast();
}

bool UFuseIgnitionComponent::IsValidActiveSource(AActor* Candidate) const
{
    return IsValid(Candidate) &&
        Candidate->GetClass()->ImplementsInterface(UIgnitionSourceInterface::StaticClass()) &&
        IIgnitionSourceInterface::Execute_IsIgnitionActive(Candidate);
}
