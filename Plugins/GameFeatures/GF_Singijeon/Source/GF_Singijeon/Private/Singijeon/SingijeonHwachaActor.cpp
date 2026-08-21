#include "Singijeon/SingijeonHwachaActor.h"

#include "Components/SceneComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/Scenario/ScenarioInteractableComponent.h"
#include "Core/Scenario/ScenarioTypes.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Interaction/TwoHandCarryComponent.h"
#include "Singijeon/FuseIgnitionComponent.h"
#include "Singijeon/SingijeonAmmunitionInterface.h"
#include "Singijeon/SingijeonAmmoSlotComponent.h"
#include "TimerManager.h"

ASingijeonHwachaActor::ASingijeonHwachaActor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
    SetRootComponent(BodyMesh);
    BodyMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

    RackRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RackRoot"));
    RackRoot->SetupAttachment(BodyMesh);

    DefaultAmmoSlot = CreateDefaultSubobject<USingijeonAmmoSlotComponent>(TEXT("DefaultAmmoSlot"));
    DefaultAmmoSlot->SetupAttachment(RackRoot);

    AutoLoadedArrowInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("AutoLoadedArrowInstances"));
    AutoLoadedArrowInstances->SetupAttachment(RackRoot);
    AutoLoadedArrowInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    AutoLoadedArrowInstances->SetGenerateOverlapEvents(false);

    Fuse = CreateDefaultSubobject<UFuseIgnitionComponent>(TEXT("Fuse"));
    Fuse->SetupAttachment(BodyMesh);

    TwoHandCarry = CreateDefaultSubobject<UTwoHandCarryComponent>(TEXT("TwoHandCarry"));

    LeftHandleHighlight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftHandleHighlight"));
    LeftHandleHighlight->SetupAttachment(BodyMesh);
    LeftHandleHighlight->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    LeftHandleHighlight->SetGenerateOverlapEvents(false);
    LeftHandleHighlight->SetCastShadow(false);
    LeftHandleHighlight->SetVisibility(false);
    LeftHandleHighlight->SetHiddenInGame(true);

    RightHandleHighlight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightHandleHighlight"));
    RightHandleHighlight->SetupAttachment(BodyMesh);
    RightHandleHighlight->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RightHandleHighlight->SetGenerateOverlapEvents(false);
    RightHandleHighlight->SetCastShadow(false);
    RightHandleHighlight->SetVisibility(false);
    RightHandleHighlight->SetHiddenInGame(true);

    LoadScenarioInteractor = CreateDefaultSubobject<UScenarioInteractableComponent>(TEXT("LoadScenarioInteractor"));
    LoadScenarioInteractor->TargetID = TEXT("Hwacha_Load");
    LoadScenarioInteractor->SupportedInteractionTypes = { EScenarioInteractionType::Custom };

    AimScenarioInteractor = CreateDefaultSubobject<UScenarioInteractableComponent>(TEXT("AimScenarioInteractor"));
    AimScenarioInteractor->TargetID = TEXT("Hwacha_Aim");
    AimScenarioInteractor->SupportedInteractionTypes = { EScenarioInteractionType::Custom };

    IgniteScenarioInteractor = CreateDefaultSubobject<UScenarioInteractableComponent>(TEXT("IgniteScenarioInteractor"));
    IgniteScenarioInteractor->TargetID = TEXT("Hwacha_Fuse");
    IgniteScenarioInteractor->SupportedInteractionTypes = { EScenarioInteractionType::Trigger };

    FireScenarioInteractor = CreateDefaultSubobject<UScenarioInteractableComponent>(TEXT("FireScenarioInteractor"));
    FireScenarioInteractor->TargetID = TEXT("Hwacha_Fire");
    FireScenarioInteractor->SupportedInteractionTypes = { EScenarioInteractionType::Combat };
}

void ASingijeonHwachaActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    if (AutoLoadedArrowInstances && AutoFillArrowMesh)
    {
        AutoLoadedArrowInstances->SetStaticMesh(AutoFillArrowMesh);
    }
    if (AutoLoadedArrowInstances && AutoFillArrowMaterial)
    {
        AutoLoadedArrowInstances->SetMaterial(0, AutoFillArrowMaterial);
    }
    SetHandleHighlightsVisible(false);
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
    TwoHandCarry->OnCarryStateChanged.AddDynamic(this, &ThisClass::HandleCarryStateChanged);
    ClearAutoFilledAmmunition();
    RefreshLoadState();
}

void ASingijeonHwachaActor::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bAimInteractionComplete || !bHasAimStartTransform || !TwoHandCarry->IsBeingCarried())
    {
        SetActorTickEnabled(false);
        return;
    }

    const float DistanceMoved = FVector::Dist(
        AimStartTransform.GetLocation(), GetActorLocation());
    const float YawMoved = FMath::Abs(FMath::FindDeltaAngleDegrees(
        AimStartTransform.Rotator().Yaw, GetActorRotation().Yaw));
    const bool bDistanceComplete = AimCompletionDistance > 0.0f &&
        DistanceMoved >= AimCompletionDistance;
    const bool bYawComplete = AimCompletionYawDegrees > 0.0f &&
        YawMoved >= AimCompletionYawDegrees;
    if (bDistanceComplete || bYawComplete)
    {
        if (!CompleteAimInteraction())
        {
            // Do not report every frame when the Scenario has not reached Hwacha_Aim yet.
            // Releasing restores the guide so the player can retry at the correct step.
            SetActorTickEnabled(false);
        }
    }
}

int32 ASingijeonHwachaActor::GetLoadedAmmunitionCount() const
{
    return GetPhysicallyLoadedAmmunitionCount() +
        (AutoLoadedArrowInstances ? AutoLoadedArrowInstances->GetInstanceCount() : 0);
}

int32 ASingijeonHwachaActor::GetPhysicallyLoadedAmmunitionCount() const
{
    int32 LoadedCount = 0;
    for (const USingijeonAmmoSlotComponent* Slot : AmmoSlots)
    {
        LoadedCount += IsValid(Slot) && Slot->IsLoaded() ? 1 : 0;
    }
    return LoadedCount;
}

int32 ASingijeonHwachaActor::GetAmmunitionCapacity() const
{
    if (bAutoFillOnFirstLoad)
    {
        return FMath::Max(1, AutoFillRows) * FMath::Max(1, AutoFillColumns);
    }
    return AmmoSlots.Num();
}

bool ASingijeonHwachaActor::IsReadyToIgnite() const
{
    return HwachaState == ESingijeonHwachaState::Loaded &&
        GetLoadedAmmunitionCount() >= MinimumLoadedAmmunition;
}

void ASingijeonHwachaActor::HandleSlotChanged(USingijeonAmmoSlotComponent* Slot, AActor* Ammunition)
{
    if (HwachaState != ESingijeonHwachaState::Fired)
    {
        if (bAutoFillOnFirstLoad && !bAutoFilled && IsValid(Slot) && Slot->IsLoaded() && IsValid(Ammunition))
        {
            AutoFillRemainingAmmunition(Ammunition);
        }
        else if (bAutoFilled && GetPhysicallyLoadedAmmunitionCount() == 0)
        {
            ClearAutoFilledAmmunition();
        }

        RefreshLoadState();
		if (GetLoadedAmmunitionCount() >= MinimumLoadedAmmunition)
		{
			LoadScenarioInteractor->ReportInteractionCompleted(EScenarioInteractionType::Custom);
		}
    }
}

void ASingijeonHwachaActor::AutoFillRemainingAmmunition(AActor* SourceAmmunition)
{
    if (!AutoLoadedArrowInstances || !RackRoot || !IsValid(SourceAmmunition))
    {
        return;
    }

    UStaticMeshComponent* SourceMesh = Cast<UStaticMeshComponent>(SourceAmmunition->GetRootComponent());
    if (!SourceMesh)
    {
        SourceMesh = SourceAmmunition->FindComponentByClass<UStaticMeshComponent>();
    }

    UStaticMesh* InstanceMesh = AutoFillArrowMesh.Get();
    if (!InstanceMesh && SourceMesh)
    {
        InstanceMesh = SourceMesh->GetStaticMesh().Get();
    }
    if (!InstanceMesh || !SourceMesh)
    {
        return;
    }

    AutoLoadedArrowInstances->ClearInstances();
    AutoLoadedArrowInstances->SetStaticMesh(InstanceMesh);
    if (AutoFillArrowMaterial)
    {
        AutoLoadedArrowInstances->SetMaterial(0, AutoFillArrowMaterial);
    }
    else
    {
        for (int32 MaterialIndex = 0; MaterialIndex < SourceMesh->GetNumMaterials(); ++MaterialIndex)
        {
            if (UMaterialInterface* SourceMaterial = SourceMesh->GetMaterial(MaterialIndex))
            {
                AutoLoadedArrowInstances->SetMaterial(MaterialIndex, SourceMaterial);
            }
        }
    }
    AutoFilledAmmunitionClass = SourceAmmunition->GetClass();

    const FTransform RackWorldTransform = RackRoot->GetComponentTransform();
    const FTransform SourceMeshLocalTransform = SourceMesh->GetComponentTransform().GetRelativeTransform(RackWorldTransform);
    const int32 Rows = FMath::Max(1, AutoFillRows);
    const int32 Columns = FMath::Max(1, AutoFillColumns);
    for (int32 Row = 0; Row < Rows; ++Row)
    {
        for (int32 Column = 0; Column < Columns; ++Column)
        {
            // The player's physical arrow occupies the first grid position.
            if (Row == 0 && Column == 0)
            {
                continue;
            }

            FTransform InstanceTransform = SourceMeshLocalTransform;
            InstanceTransform.AddToTranslation(AutoFillGridOffset + FVector(
                0.0f, Column * AutoFillColumnSpacing, Row * AutoFillRowSpacing));
            AutoLoadedArrowInstances->AddInstance(InstanceTransform);
        }
    }
    bAutoFilled = AutoLoadedArrowInstances->GetInstanceCount() > 0;
}

void ASingijeonHwachaActor::ClearAutoFilledAmmunition()
{
    if (AutoLoadedArrowInstances)
    {
        AutoLoadedArrowInstances->ClearInstances();
    }
    AutoFilledAmmunitionClass = nullptr;
    bAutoFilled = false;
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

    if (PendingLaunchSlots.Num() == 0 &&
        (!AutoLoadedArrowInstances || AutoLoadedArrowInstances->GetInstanceCount() == 0))
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
    if (PendingLaunchSlots.IsValidIndex(NextLaunchIndex))
    {
        USingijeonAmmoSlotComponent* Slot = PendingLaunchSlots[NextLaunchIndex++];
        if (IsValid(Slot))
        {
            Slot->LaunchLoadedAmmunition(Slot->GetForwardVector(), LaunchSpeed);
        }
    }
    else if (!LaunchNextAutoFilledAmmunition())
    {
        GetWorldTimerManager().ClearTimer(LaunchTimerHandle);
        PendingLaunchSlots.Reset();
		bAutoFilled = false;
		AutoFilledAmmunitionClass = nullptr;
		FireScenarioInteractor->ReportInteractionCompleted(EScenarioInteractionType::Combat);
        OnVolleyLaunched.Broadcast();
        return;
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

void ASingijeonHwachaActor::HandleCarryStateChanged(const bool bIsCarrying)
{
    if (bAimInteractionComplete)
    {
        SetHandleHighlightsVisible(false);
        SetActorTickEnabled(false);
        return;
    }

    // The highlight is a guide: hide it once both hands are correctly placed,
    // and restore it if the player releases before completing the movement.
    SetHandleHighlightsVisible(bEnableAimGuideHighlight && !bIsCarrying && bHasAimStartTransform);
    SetActorTickEnabled(bIsCarrying && bHasAimStartTransform);
}

bool ASingijeonHwachaActor::CompleteAimInteraction()
{
    if (bAimInteractionComplete)
    {
        return true;
    }
    if (!bHasAimStartTransform || !AimScenarioInteractor ||
        !AimScenarioInteractor->ReportInteractionCompleted(EScenarioInteractionType::Custom))
    {
        return false;
    }

    bAimInteractionComplete = true;
    SetHandleHighlightsVisible(false);
    SetActorTickEnabled(false);
    return true;
}

void ASingijeonHwachaActor::SetHandleHighlightsVisible(const bool bVisible)
{
    for (UStaticMeshComponent* Highlight : { LeftHandleHighlight.Get(), RightHandleHighlight.Get() })
    {
        if (Highlight)
        {
            Highlight->SetVisibility(bVisible, true);
            Highlight->SetHiddenInGame(!bVisible, true);
        }
    }
}

bool ASingijeonHwachaActor::LaunchNextAutoFilledAmmunition()
{
    if (!AutoLoadedArrowInstances || AutoLoadedArrowInstances->GetInstanceCount() == 0 ||
        !AutoFilledAmmunitionClass)
    {
        return false;
    }

    FTransform LaunchTransform;
    if (!AutoLoadedArrowInstances->GetInstanceTransform(0, LaunchTransform, true))
    {
        AutoLoadedArrowInstances->RemoveInstance(0);
        return true;
    }
    AutoLoadedArrowInstances->RemoveInstance(0);

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.Owner = this;
    SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* SpawnedAmmunition = GetWorld()->SpawnActor<AActor>(
        AutoFilledAmmunitionClass, LaunchTransform, SpawnParameters);
    if (IsValid(SpawnedAmmunition) &&
        SpawnedAmmunition->GetClass()->ImplementsInterface(USingijeonAmmunitionInterface::StaticClass()))
    {
        ISingijeonAmmunitionInterface::Execute_OnLaunched(
            SpawnedAmmunition,
            DefaultAmmoSlot ? DefaultAmmoSlot->GetForwardVector() : GetActorForwardVector(),
            LaunchSpeed);
    }
    return true;
}

void ASingijeonHwachaActor::ResetHwacha()
{
    GetWorldTimerManager().ClearTimer(LaunchTimerHandle);
    PendingLaunchSlots.Reset();
    NextLaunchIndex = 0;
    ClearAutoFilledAmmunition();
    bAimInteractionComplete = false;
    bHasAimStartTransform = false;
    SetHandleHighlightsVisible(false);
    SetActorTickEnabled(false);
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
    if (bHasEnoughAmmo && !bHasAimStartTransform)
    {
        AimStartTransform = GetActorTransform();
        bHasAimStartTransform = true;
        bAimInteractionComplete = false;
    }
    else if (!bHasEnoughAmmo)
    {
        bHasAimStartTransform = false;
        bAimInteractionComplete = false;
    }
    SetHandleHighlightsVisible(
        bEnableAimGuideHighlight && bHasEnoughAmmo && !bAimInteractionComplete &&
        !TwoHandCarry->IsBeingCarried());
    OnLoadCountChanged.Broadcast(LoadedCount, GetAmmunitionCapacity());
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
