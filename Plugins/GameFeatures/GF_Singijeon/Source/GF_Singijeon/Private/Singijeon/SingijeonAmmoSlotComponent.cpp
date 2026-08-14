#include "Singijeon/SingijeonAmmoSlotComponent.h"

#include "Singijeon/SingijeonAmmunitionInterface.h"

USingijeonAmmoSlotComponent::USingijeonAmmoSlotComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    InitBoxExtent(FVector(8.0f, 8.0f, 25.0f));
    SetCollisionProfileName(TEXT("Trigger"));
    SetGenerateOverlapEvents(true);
}

void USingijeonAmmoSlotComponent::BeginPlay()
{
    Super::BeginPlay();
    OnComponentBeginOverlap.AddDynamic(this, &ThisClass::HandleBeginOverlap);
}

void USingijeonAmmoSlotComponent::HandleBeginOverlap(UPrimitiveComponent*, AActor* OtherActor,
    UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    TryLoadAmmunition(OtherActor);
}

bool USingijeonAmmoSlotComponent::TryLoadAmmunition(AActor* Candidate)
{
    if (bLoadInProgress || IsLoaded() || !IsValid(Candidate) || Candidate == GetOwner() ||
        !Candidate->GetClass()->ImplementsInterface(USingijeonAmmunitionInterface::StaticClass()) ||
        !ISingijeonAmmunitionInterface::Execute_CanBeLoaded(Candidate))
    {
        return false;
    }

    TGuardValue<bool> LoadingGuard(bLoadInProgress, true);
    TWeakObjectPtr<AActor> CandidateGuard(Candidate);
    if (!ISingijeonAmmunitionInterface::Execute_PrepareForLoading(Candidate) || !CandidateGuard.IsValid())
    {
        return false;
    }

    LoadedAmmunition = Candidate;
    if (!Candidate->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale))
    {
        LoadedAmmunition = nullptr;
        return false;
    }
    Candidate->OnDestroyed.AddDynamic(this, &ThisClass::HandleAmmunitionDestroyed);
    ISingijeonAmmunitionInterface::Execute_OnLoaded(Candidate, this);
    OnAmmunitionLoaded.Broadcast(this, Candidate);
    return true;
}

AActor* USingijeonAmmoSlotComponent::UnloadAmmunition()
{
    AActor* Result = LoadedAmmunition;
    if (!IsValid(Result))
    {
        LoadedAmmunition = nullptr;
        return nullptr;
    }

    Result->OnDestroyed.RemoveDynamic(this, &ThisClass::HandleAmmunitionDestroyed);
    Result->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    ISingijeonAmmunitionInterface::Execute_OnUnloaded(Result);
    LoadedAmmunition = nullptr;
    OnAmmunitionRemoved.Broadcast(this, Result);
    return Result;
}

bool USingijeonAmmoSlotComponent::LaunchLoadedAmmunition(const FVector Direction, const float Speed)
{
    AActor* Ammunition = LoadedAmmunition;
    if (!IsValid(Ammunition))
    {
        LoadedAmmunition = nullptr;
        return false;
    }

    Ammunition->OnDestroyed.RemoveDynamic(this, &ThisClass::HandleAmmunitionDestroyed);
    LoadedAmmunition = nullptr;
    Ammunition->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    ISingijeonAmmunitionInterface::Execute_OnLaunched(Ammunition, Direction.GetSafeNormal(), Speed);
    OnAmmunitionRemoved.Broadcast(this, Ammunition);
    return true;
}

void USingijeonAmmoSlotComponent::HandleAmmunitionDestroyed(AActor* DestroyedActor)
{
    if (DestroyedActor != LoadedAmmunition)
    {
        return;
    }

    LoadedAmmunition = nullptr;
    OnAmmunitionRemoved.Broadcast(this, DestroyedActor);
}
