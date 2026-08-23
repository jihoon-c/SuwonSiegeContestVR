#include "Singijeon/SingijeonAmmoSlotComponent.h"

#include "Singijeon/SingijeonAmmunitionInterface.h"
#include "Singijeon/SingijeonHwachaActor.h"

USingijeonAmmoSlotComponent::USingijeonAmmoSlotComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    InitBoxExtent(FVector(8.0f, 8.0f, 25.0f));
    SetCollisionProfileName(TEXT("Trigger"));
    SetGenerateOverlapEvents(true);
}

void USingijeonAmmoSlotComponent::TickComponent(const float DeltaTime, const ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    EnforceLoadedAttachment();
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
    const ASingijeonHwachaActor* Hwacha = Cast<ASingijeonHwachaActor>(GetOwner());
    if (bLoadInProgress || IsLoaded() || !IsValid(Candidate) || Candidate == GetOwner() ||
        (Hwacha && !Hwacha->CanAcceptAmmunitionNow()) ||
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
    if (USceneComponent* Root = Candidate->GetRootComponent())
    {
        Root->SetUsingAbsoluteLocation(false);
        Root->SetUsingAbsoluteRotation(false);
        Root->SetUsingAbsoluteScale(false);
    }
    if (!Candidate->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale))
    {
        LoadedAmmunition = nullptr;
        return false;
    }
    Candidate->OnDestroyed.AddDynamic(this, &ThisClass::HandleAmmunitionDestroyed);
    ISingijeonAmmunitionInterface::Execute_OnLoaded(Candidate, this);
    EnforceLoadedAttachment();
    SetComponentTickEnabled(true);
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
    SetComponentTickEnabled(false);
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
    SetComponentTickEnabled(false);
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
    SetComponentTickEnabled(false);
    OnAmmunitionRemoved.Broadcast(this, DestroyedActor);
}

void USingijeonAmmoSlotComponent::EnforceLoadedAttachment()
{
    if (!IsValid(LoadedAmmunition))
    {
        LoadedAmmunition = nullptr;
        SetComponentTickEnabled(false);
        return;
    }

    USceneComponent* Root = LoadedAmmunition->GetRootComponent();
    if (!IsValid(Root))
    {
        return;
    }

    Root->SetUsingAbsoluteLocation(false);
    Root->SetUsingAbsoluteRotation(false);
    Root->SetUsingAbsoluteScale(false);
    if (Root->GetAttachParent() != this)
    {
        LoadedAmmunition->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    }
    else
    {
        Root->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator,
            false, nullptr, ETeleportType::TeleportPhysics);
    }
    // BP_GrabComponent can restore its pre-grab material after OnLoaded. Refresh
    // after physics so the first physical arrow cannot remain unmaterialed.
    ISingijeonAmmunitionInterface::Execute_RefreshLoadedVisual(LoadedAmmunition);
}
