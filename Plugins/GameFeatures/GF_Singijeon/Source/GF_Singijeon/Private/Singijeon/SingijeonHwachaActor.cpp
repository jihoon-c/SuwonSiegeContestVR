#include "Singijeon/SingijeonHwachaActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/Scenario/ScenarioInteractableComponent.h"
#include "Core/Scenario/ScenarioTypes.h"
#include "Engine/World.h"
#include "Interaction/TwoHandCarryComponent.h"
#include "Singijeon/FuseIgnitionComponent.h"
#include "Singijeon/SingijeonAmmoSlotComponent.h"
#include "TimerManager.h"

ASingijeonHwachaActor::ASingijeonHwachaActor()
{
    PrimaryActorTick.bCanEverTick = false;

    BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
    SetRootComponent(BodyMesh);
    BodyMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

    RackRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RackRoot"));
    RackRoot->SetupAttachment(BodyMesh);

    DefaultAmmoSlot = CreateDefaultSubobject<USingijeonAmmoSlotComponent>(TEXT("DefaultAmmoSlot"));
    DefaultAmmoSlot->SetupAttachment(RackRoot);

    Fuse = CreateDefaultSubobject<UFuseIgnitionComponent>(TEXT("Fuse"));
    Fuse->SetupAttachment(BodyMesh);

    TwoHandCarry = CreateDefaultSubobject<UTwoHandCarryComponent>(TEXT("TwoHandCarry"));

    LoadScenarioInteractor = CreateDefaultSubobject<UScenarioInteractableComponent>(TEXT("LoadScenarioInteractor"));
    LoadScenarioInteractor->TargetID = TEXT("Hwacha_Load");
    LoadScenarioInteractor->SupportedInteractionTypes = { EScenarioInteractionType::Custom };

    IgniteScenarioInteractor = CreateDefaultSubobject<UScenarioInteractableComponent>(TEXT("IgniteScenarioInteractor"));
    IgniteScenarioInteractor->TargetID = TEXT("Hwacha_Fuse");
    IgniteScenarioInteractor->SupportedInteractionTypes = { EScenarioInteractionType::Trigger };

    FireScenarioInteractor = CreateDefaultSubobject<UScenarioInteractableComponent>(TEXT("FireScenarioInteractor"));
    FireScenarioInteractor->TargetID = TEXT("Hwacha_Fire");
    FireScenarioInteractor->SupportedInteractionTypes = { EScenarioInteractionType::Combat };
}

void ASingijeonHwachaActor::BeginPlay()
{
    Super::BeginPlay();

    TInlineComponentArray<USingijeonAmmoSlotComponent*> FoundSlots(this);
    GetComponents(FoundSlots);
    AmmoSlots.Reset(FoundSlots.Num());
    for (USingijeonAmmoSlotComponent* Slot : FoundSlots)
    {
        if (!IsValid(Slot))
        {
            continue;
        }

        AmmoSlots.Add(Slot);
        Slot->OnAmmunitionLoaded.AddDynamic(this, &ThisClass::HandleSlotChanged);
        Slot->OnAmmunitionRemoved.AddDynamic(this, &ThisClass::HandleSlotChanged);
    }

    Fuse->OnIgnitionStarted.AddDynamic(this, &ThisClass::HandleIgnitionStarted);
    Fuse->OnIgnitionCanceled.AddDynamic(this, &ThisClass::HandleIgnitionCanceled);
    Fuse->OnIgnited.AddDynamic(this, &ThisClass::HandleFuseIgnited);
    RefreshLoadState();
}

int32 ASingijeonHwachaActor::GetLoadedAmmunitionCount() const
{
    int32 LoadedCount = 0;
    for (const USingijeonAmmoSlotComponent* Slot : AmmoSlots)
    {
        LoadedCount += IsValid(Slot) && Slot->IsLoaded() ? 1 : 0;
    }
    return LoadedCount;
}

bool ASingijeonHwachaActor::IsReadyToIgnite() const
{
    return HwachaState == ESingijeonHwachaState::Loaded &&
        GetLoadedAmmunitionCount() >= MinimumLoadedAmmunition;
}

void ASingijeonHwachaActor::HandleSlotChanged(USingijeonAmmoSlotComponent*, AActor*)
{
    if (HwachaState != ESingijeonHwachaState::Fired)
    {
        RefreshLoadState();
		if (GetLoadedAmmunitionCount() >= MinimumLoadedAmmunition)
		{
			LoadScenarioInteractor->ReportInteractionCompleted(EScenarioInteractionType::Custom);
		}
    }
}

void ASingijeonHwachaActor::HandleIgnitionStarted(AActor*)
{
    if (!IsReadyToIgnite())
    {
        Fuse->CancelIgnition();
        return;
    }

    SetHwachaState(ESingijeonHwachaState::Igniting);
    TwoHandCarry->SetCarryEnabled(false);
}

void ASingijeonHwachaActor::HandleIgnitionCanceled(AActor*)
{
    if (HwachaState == ESingijeonHwachaState::Igniting)
    {
        RefreshLoadState();
    }
}

void ASingijeonHwachaActor::HandleFuseIgnited()
{
	IgniteScenarioInteractor->ReportInteractionCompleted(EScenarioInteractionType::Trigger);
    LaunchVolley();
}

void ASingijeonHwachaActor::LaunchVolley()
{
    if (HwachaState != ESingijeonHwachaState::Igniting && !IsReadyToIgnite())
    {
        return;
    }

    PendingLaunchSlots.Reset();
    for (USingijeonAmmoSlotComponent* Slot : AmmoSlots)
    {
        if (IsValid(Slot) && Slot->IsLoaded())
        {
            PendingLaunchSlots.Add(Slot);
        }
    }

    if (PendingLaunchSlots.Num() == 0)
    {
        RefreshLoadState();
        return;
    }

    SetHwachaState(ESingijeonHwachaState::Fired);
    Fuse->SetIgnitionEnabled(false);
    TwoHandCarry->SetCarryEnabled(false);
    NextLaunchIndex = 0;
    LaunchNextAmmunition();
}

void ASingijeonHwachaActor::LaunchNextAmmunition()
{
    if (!PendingLaunchSlots.IsValidIndex(NextLaunchIndex))
    {
        GetWorldTimerManager().ClearTimer(LaunchTimerHandle);
        PendingLaunchSlots.Reset();
		FireScenarioInteractor->ReportInteractionCompleted(EScenarioInteractionType::Combat);
        OnVolleyLaunched.Broadcast();
        return;
    }

    USingijeonAmmoSlotComponent* Slot = PendingLaunchSlots[NextLaunchIndex++];
    if (IsValid(Slot))
    {
        Slot->LaunchLoadedAmmunition(Slot->GetForwardVector(), LaunchSpeed);
    }

    if (LaunchInterval <= KINDA_SMALL_NUMBER)
    {
        LaunchNextAmmunition();
    }
    else
    {
        GetWorldTimerManager().SetTimer(LaunchTimerHandle, this, &ThisClass::LaunchNextAmmunition,
            LaunchInterval, false);
    }
}

void ASingijeonHwachaActor::ResetHwacha()
{
    GetWorldTimerManager().ClearTimer(LaunchTimerHandle);
    PendingLaunchSlots.Reset();
    NextLaunchIndex = 0;
    for (USingijeonAmmoSlotComponent* Slot : AmmoSlots)
    {
        if (IsValid(Slot) && Slot->IsLoaded())
        {
            Slot->UnloadAmmunition();
        }
    }
    SetHwachaState(ESingijeonHwachaState::Empty);
    RefreshLoadState();
}

void ASingijeonHwachaActor::RefreshLoadState()
{
    const int32 LoadedCount = GetLoadedAmmunitionCount();
    const bool bHasEnoughAmmo = LoadedCount >= MinimumLoadedAmmunition;
    SetHwachaState(bHasEnoughAmmo ? ESingijeonHwachaState::Loaded : ESingijeonHwachaState::Empty);
    Fuse->SetIgnitionEnabled(bHasEnoughAmmo);
    TwoHandCarry->SetCarryEnabled(bHasEnoughAmmo);
    OnLoadCountChanged.Broadcast(LoadedCount, AmmoSlots.Num());
}

void ASingijeonHwachaActor::SetHwachaState(const ESingijeonHwachaState NewState)
{
    if (HwachaState == NewState)
    {
        return;
    }

    const ESingijeonHwachaState PreviousState = HwachaState;
    HwachaState = NewState;
    OnHwachaStateChanged.Broadcast(PreviousState, HwachaState);
}
