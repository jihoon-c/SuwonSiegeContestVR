#include "Singijeon/SingijeonHwachaActor.h"

#include "Components/SceneComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/Scenario/ScenarioInteractableComponent.h"
#include "Core/Scenario/ScenarioTypes.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundBase.h"
#include "Interaction/TwoHandCarryComponent.h"
#include "MotionControllerComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Singijeon/FuseIgnitionComponent.h"
#include "Singijeon/SingijeonAmmunitionInterface.h"
#include "Singijeon/SingijeonAmmoSlotComponent.h"
#include "Singijeon/SingijeonLaunchSpread.h"
#include "Singijeon/SingijeonProjectileActor.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ASingijeonHwachaActor::ASingijeonHwachaActor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
    static ConstructorHelpers::FObjectFinder<USoundBase> LaunchSound(TEXT("/GF_Singijeon/Asset/Sound/Effect/SingijeonLaunch.SingijeonLaunch"));
    ArrowLaunchSound = LaunchSound.Succeeded() ? LaunchSound.Object : nullptr;

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

#if WITH_EDITORONLY_DATA
    AmmoGridEditorPreview = CreateEditorOnlyDefaultSubobject<UInstancedStaticMeshComponent>(
        TEXT("AmmoGridEditorPreview"));
    // WITH_EDITORONLY_DATA is also defined for an Editor target launched with
    // -game, where CreateEditorOnlyDefaultSubobject can intentionally return
    // null. Never let the authoring preview block standalone/VR startup.
    if (AmmoGridEditorPreview)
    {
        AmmoGridEditorPreview->SetupAttachment(RackRoot);
        AmmoGridEditorPreview->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        AmmoGridEditorPreview->SetGenerateOverlapEvents(false);
        AmmoGridEditorPreview->SetCanEverAffectNavigation(false);
        AmmoGridEditorPreview->SetCastShadow(false);
        AmmoGridEditorPreview->SetHiddenInGame(true);
    }
#endif

    AmmoGridCenterArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("AmmoGridCenterArrow"));
    AmmoGridCenterArrow->SetupAttachment(RackRoot);
    AmmoGridCenterArrow->ArrowColor = FColor(255, 200, 32);
    AmmoGridCenterArrow->ArrowSize = 1.5f;
    AmmoGridCenterArrow->ArrowLength = 80.0f;
    AmmoGridCenterArrow->bIsScreenSizeScaled = true;
    AmmoGridCenterArrow->SetHiddenInGame(true);
    AmmoGridCenterArrow->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    Fuse = CreateDefaultSubobject<UFuseIgnitionComponent>(TEXT("Fuse"));
    Fuse->SetupAttachment(BodyMesh);

    FuseIgnitionEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FuseIgnitionEffect"));
    FuseIgnitionEffect->SetupAttachment(Fuse);
    FuseIgnitionEffect->SetAutoActivate(false);
    FuseIgnitionEffect->SetUsingAbsoluteLocation(false);
    FuseIgnitionEffect->SetRelativeScale3D(FVector(0.16f));
    static ConstructorHelpers::FObjectFinder<UNiagaraSystem> DefaultFuseEffect(
        TEXT("/Game/NiagaraExamples/FX_Misc/NS_Fire.NS_Fire"));
    if (DefaultFuseEffect.Succeeded())
    {
        FuseIgnitionEffect->SetAsset(DefaultFuseEffect.Object);
    }

    FuseGuide = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FuseGuide"));
    FuseGuide->SetupAttachment(Fuse);
    FuseGuide->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    FuseGuide->SetGenerateOverlapEvents(false);
    FuseGuide->SetCastShadow(false);
    FuseGuide->SetRelativeScale3D(FVector(0.12f));
    FuseGuide->SetVisibility(false);
    FuseGuide->SetHiddenInGame(true);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> FuseGuideMeshFinder(
        TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (FuseGuideMeshFinder.Succeeded())
    {
        FuseGuide->SetStaticMesh(FuseGuideMeshFinder.Object);
    }
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> FuseGuideMaterialFinder(
        TEXT("/Game/LevelPrototyping/Interactable/JumpPad/Assets/Materials/MI_GlowNT.MI_GlowNT"));
    if (FuseGuideMaterialFinder.Succeeded())
    {
        FuseGuide->SetMaterial(0, FuseGuideMaterialFinder.Object);
    }

    FuseCord = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FuseCord"));
    FuseCord->SetupAttachment(BodyMesh);
    FuseCord->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    FuseCord->SetGenerateOverlapEvents(false);
    FuseCord->SetCastShadow(false);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> FuseCordMeshFinder(
        TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (FuseCordMeshFinder.Succeeded())
    {
        FuseCord->SetStaticMesh(FuseCordMeshFinder.Object);
    }
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> FuseCordMaterialFinder(
        TEXT("/Game/NiagaraExamples/Gallery/StaticMesh/BeachBall/M_BeachBallWhite.M_BeachBallWhite"));
    if (FuseCordMaterialFinder.Succeeded())
    {
        FuseCord->SetMaterial(0, FuseCordMaterialFinder.Object);
    }

    TwoHandCarry = CreateDefaultSubobject<UTwoHandCarryComponent>(TEXT("TwoHandCarry"));

    LeftHandleGrabPoint = CreateDefaultSubobject<USceneComponent>(TEXT("LeftHandleGrabPoint"));
    LeftHandleGrabPoint->SetupAttachment(BodyMesh);
    LeftHandleGrabPoint->SetRelativeLocation(FVector(-47.0f, -43.0f, 61.0f));
    LeftHandleGrabPoint->ComponentTags.Add(TEXT("VRGrab"));

    RightHandleGrabPoint = CreateDefaultSubobject<USceneComponent>(TEXT("RightHandleGrabPoint"));
    RightHandleGrabPoint->SetupAttachment(BodyMesh);
    RightHandleGrabPoint->SetRelativeLocation(FVector(-47.0f, 43.0f, 61.0f));
    RightHandleGrabPoint->ComponentTags.Add(TEXT("VRGrab"));

    LeftHandleHighlight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftHandleHighlight"));
    LeftHandleHighlight->SetupAttachment(BodyMesh);
    LeftHandleHighlight->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    LeftHandleHighlight->SetGenerateOverlapEvents(false);
    LeftHandleHighlight->SetCastShadow(false);
    LeftHandleHighlight->SetVisibility(false);
    LeftHandleHighlight->SetHiddenInGame(true);
    LeftHandleHighlight->ComponentTags.Add(TEXT("VRGrab"));

    RightHandleHighlight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightHandleHighlight"));
    RightHandleHighlight->SetupAttachment(BodyMesh);
    RightHandleHighlight->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RightHandleHighlight->SetGenerateOverlapEvents(false);
    RightHandleHighlight->SetCastShadow(false);
    RightHandleHighlight->SetVisibility(false);
    RightHandleHighlight->SetHiddenInGame(true);
    RightHandleHighlight->ComponentTags.Add(TEXT("VRGrab"));

    MoveTargetMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MoveTargetMarker"));
    MoveTargetMarker->SetupAttachment(BodyMesh);
    MoveTargetMarker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MoveTargetMarker->SetGenerateOverlapEvents(false);
    MoveTargetMarker->SetCastShadow(false);
    MoveTargetMarker->SetVisibility(false);
    MoveTargetMarker->SetHiddenInGame(true);

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
        if (!AutoFillArrowMaterial->CheckMaterialUsage(MATUSAGE_InstancedStaticMeshes))
        {
            UE_LOG(LogTemp, Error,
                TEXT("%s AutoFillArrowMaterial %s has no Instanced Static Mesh shader permutation."),
                *GetName(), *GetNameSafe(AutoFillArrowMaterial));
        }
        // The imported arrow may expose more than one material slot. Applying
        // the authored override to only slot 0 leaves later sections with the
        // mesh default (or an unset fallback) after the ISM is created.
        const int32 MaterialSlotCount = FMath::Max(1, AutoLoadedArrowInstances->GetNumMaterials());
        for (int32 MaterialIndex = 0; MaterialIndex < MaterialSlotCount; ++MaterialIndex)
        {
            AutoLoadedArrowInstances->SetMaterial(MaterialIndex, AutoFillArrowMaterial);
        }
    }
    SetHandleHighlightsVisible(false);
    SetMoveTargetVisible(false);
    SetFuseGuideVisible(false);
    UpdateAuthoringVisuals();
}

void ASingijeonHwachaActor::UpdateAuthoringVisuals()
{
#if WITH_EDITOR
    RefreshAmmoGridEditorPreview();
#endif

    if (AmmoGridCenterArrow && DefaultAmmoSlot)
    {
        const FVector GridCenterOffset = AutoFillGridOffset + FVector(
            0.0f,
            0.5f * FMath::Max(0, AutoFillColumns - 1) * AutoFillColumnSpacing,
            0.5f * FMath::Max(0, AutoFillRows - 1) * AutoFillRowSpacing);
        AmmoGridCenterArrow->SetRelativeLocation(
            DefaultAmmoSlot->GetRelativeLocation() + GridCenterOffset + AmmoGridCenterArrowOffset);
        AmmoGridCenterArrow->SetRelativeRotation(
            (DefaultAmmoSlot->GetRelativeRotation().Quaternion() *
                AmmoGridCenterArrowRotation.Quaternion()).Rotator());
    }

    if (FuseCord && Fuse)
    {
        const FVector FuseCordEnd = Fuse->GetRelativeLocation();
        const FVector CordVector = FuseCordEnd - FuseCordStart;
        const float CordLength = CordVector.Size();
        if (CordLength > KINDA_SMALL_NUMBER)
        {
            FuseCord->SetRelativeLocation((FuseCordStart + FuseCordEnd) * 0.5f);
            FuseCord->SetRelativeRotation(
                FQuat::FindBetweenNormals(FVector::UpVector, CordVector / CordLength));
            // Engine Cylinder is 100 cm high with a 50 cm radius.
            FuseCord->SetRelativeScale3D(FVector(
                FMath::Max(FuseCordRadius, 0.1f) / 50.0f,
                FMath::Max(FuseCordRadius, 0.1f) / 50.0f,
                CordLength / 100.0f));
        }
    }
}

#if WITH_EDITOR
void ASingijeonHwachaActor::RefreshAmmoGridEditorPreview()
{
#if WITH_EDITORONLY_DATA
    if (!AmmoGridEditorPreview)
    {
        return;
    }

    AmmoGridEditorPreview->ClearInstances();
    AmmoGridEditorPreview->SetVisibility(bShowAmmoGridPreviewInEditor);
    AmmoGridEditorPreview->SetHiddenInGame(true);
    if (!bShowAmmoGridPreviewInEditor || !DefaultAmmoSlot || !AutoFillArrowMesh)
    {
        AmmoGridEditorPreview->SetStaticMesh(nullptr);
        return;
    }

    AmmoGridEditorPreview->SetRelativeTransform(FTransform::Identity);
    AmmoGridEditorPreview->SetStaticMesh(AutoFillArrowMesh);
    if (AutoFillArrowMaterial)
    {
        const int32 MaterialSlotCount = FMath::Max(1, AmmoGridEditorPreview->GetNumMaterials());
        for (int32 MaterialIndex = 0; MaterialIndex < MaterialSlotCount; ++MaterialIndex)
        {
            AmmoGridEditorPreview->SetMaterial(MaterialIndex, AutoFillArrowMaterial);
        }
    }

    const FTransform SlotTransform = DefaultAmmoSlot->GetRelativeTransform();
    const int32 Rows = FMath::Max(1, AutoFillRows);
    const int32 Columns = FMath::Max(1, AutoFillColumns);
    for (int32 Row = 0; Row < Rows; ++Row)
    {
        for (int32 Column = 0; Column < Columns; ++Column)
        {
            FTransform InstanceTransform = SlotTransform;
            InstanceTransform.AddToTranslation(AutoFillGridOffset + FVector(
                0.0f, Column * AutoFillColumnSpacing, Row * AutoFillRowSpacing));
            AmmoGridEditorPreview->AddInstance(InstanceTransform);
        }
    }
#endif
}
#endif

void ASingijeonHwachaActor::BeginPlay()
{
    Super::BeginPlay();

#if WITH_EDITORONLY_DATA
    if (AmmoGridEditorPreview)
    {
        AmmoGridEditorPreview->ClearInstances();
    }
#endif

    // The authored relative transform defines the destination in the Blueprint
    // viewport. Detaching freezes it in the level while the Hwacha is dragged.
    MoveTargetMarker->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

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
        Slot->OnAmmunitionLoaded.AddUniqueDynamic(this, &ThisClass::HandleSlotChanged);
        Slot->OnAmmunitionRemoved.AddUniqueDynamic(this, &ThisClass::HandleSlotChanged);
    }

    Fuse->OnIgnitionStarted.AddUniqueDynamic(this, &ThisClass::HandleIgnitionStarted);
    Fuse->OnIgnitionCanceled.AddUniqueDynamic(this, &ThisClass::HandleIgnitionCanceled);
    Fuse->OnIgnited.AddUniqueDynamic(this, &ThisClass::HandleFuseIgnited);
    TwoHandCarry->OnCarryStateChanged.AddUniqueDynamic(this, &ThisClass::HandleCarryStateChanged);
    ClearAutoFilledAmmunition();

    // Allocate and warm the fuse effect before the interaction frame. Ignition
    // then only resumes the existing simulation instead of resetting NS_Fire.
    if (FuseIgnitionEffect && FuseIgnitionEffect->GetAsset())
    {
        FuseIgnitionEffect->SetAllowScalability(true);
        FuseIgnitionEffect->SetCullDistance(1500.0f);
        FuseIgnitionEffect->SetRenderingEnabled(false);
        if (!FuseIgnitionEffect->IsActive())
        {
            FuseIgnitionEffect->Activate(false);
        }
        FuseIgnitionEffect->AdvanceSimulation(1, 1.0f / 30.0f);
        FuseIgnitionEffect->SetPaused(true);
    }
    SetFuseIgnitionEffectActive(false);
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

    const float DistanceToTarget = FVector::Dist2D(
        MoveTargetMarker->GetComponentLocation(), GetActorLocation());
    if (DistanceToTarget <= MoveTargetAcceptanceRadius)
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

bool ASingijeonHwachaActor::CanAcceptAmmunitionNow() const
{
    return LoadScenarioInteractor &&
        LoadScenarioInteractor->CanReportInteraction(EScenarioInteractionType::Custom);
}

bool ASingijeonHwachaActor::CanBeginFuseIgnitionNow() const
{
    return IsReadyToIgnite() && IgniteScenarioInteractor &&
        IgniteScenarioInteractor->CanReportInteraction(EScenarioInteractionType::Trigger);
}

void ASingijeonHwachaActor::HandleSlotChanged(USingijeonAmmoSlotComponent* Slot, AActor* Ammunition)
{
    if (HwachaState != ESingijeonHwachaState::Fired)
    {
        if (bAutoFillOnFirstLoad && !bAutoFilled && !bAutoFillInProgress &&
            IsValid(Slot) && Slot->IsLoaded() && IsValid(Ammunition))
        {
            AutoFillRemainingAmmunition(Ammunition);
        }
        else if (bAutoFilled && GetPhysicallyLoadedAmmunitionCount() == 0)
        {
            ClearAutoFilledAmmunition();
        }

        RefreshLoadState();
		if (!bLoadInteractionReported && GetLoadedAmmunitionCount() >= MinimumLoadedAmmunition)
		{
			bLoadInteractionReported = LoadScenarioInteractor &&
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

    bAutoFillInProgress = true;
    AutoLoadedArrowInstances->ClearInstances();
    AutoLoadedArrowInstances->SetUsingAbsoluteLocation(false);
    AutoLoadedArrowInstances->SetUsingAbsoluteRotation(false);
    AutoLoadedArrowInstances->SetUsingAbsoluteScale(false);
    if (AutoLoadedArrowInstances->GetAttachParent() != RackRoot)
    {
        AutoLoadedArrowInstances->AttachToComponent(
            RackRoot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    }
    AutoLoadedArrowInstances->SetRelativeTransform(FTransform::Identity);
    AutoLoadedArrowInstances->SetStaticMesh(InstanceMesh);
    if (AutoFillArrowMaterial)
    {
        if (!AutoFillArrowMaterial->CheckMaterialUsage(MATUSAGE_InstancedStaticMeshes))
        {
            UE_LOG(LogTemp, Error,
                TEXT("%s AutoFillArrowMaterial %s cannot render on Instanced Static Meshes."),
                *GetName(), *GetNameSafe(AutoFillArrowMaterial));
        }
        const int32 MaterialSlotCount = FMath::Max(1, AutoLoadedArrowInstances->GetNumMaterials());
        for (int32 MaterialIndex = 0; MaterialIndex < MaterialSlotCount; ++MaterialIndex)
        {
            AutoLoadedArrowInstances->SetMaterial(MaterialIndex, AutoFillArrowMaterial);
        }
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

    const FTransform SourceMeshLocalTransform = SourceMesh->GetComponentTransform().GetRelativeTransform(
        AutoLoadedArrowInstances->GetComponentTransform());
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
    bAutoFillInProgress = false;
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
    if (!CanBeginFuseIgnitionNow())
    {
        Fuse->CancelIgnition();
        return;
    }

    SetHwachaState(ESingijeonHwachaState::Igniting);
    SetFuseGuideVisible(false);
    SetFuseIgnitionEffectActive(true);
    TwoHandCarry->SetCarryEnabled(false);
    SetMoveTargetVisible(false);
}

void ASingijeonHwachaActor::HandleIgnitionCanceled(AActor*)
{
    if (HwachaState == ESingijeonHwachaState::Igniting)
    {
        SetFuseIgnitionEffectActive(false);
        RefreshLoadState();
        SetFuseGuideVisible(bAimInteractionComplete && IsReadyToIgnite());
    }
}

void ASingijeonHwachaActor::HandleFuseIgnited()
{
	SetFuseIgnitionEffectActive(false);
	SetFuseGuideVisible(false);
	if (!IgniteScenarioInteractor ||
		!IgniteScenarioInteractor->ReportInteractionCompleted(EScenarioInteractionType::Trigger))
	{
		RefreshLoadState();
		return;
	}
    bPendingLaunchAfterFuse = true;
    TryStartPendingVolley();
}

void ASingijeonHwachaActor::TryStartPendingVolley()
{
    if (!bPendingLaunchAfterFuse)
    {
        return;
    }

    if (FireScenarioInteractor &&
        FireScenarioInteractor->CanReportInteraction(EScenarioInteractionType::Combat))
    {
        bPendingLaunchAfterFuse = false;
        GetWorldTimerManager().ClearTimer(PendingVolleyTimerHandle);
        LaunchVolley();
        return;
    }

    // The Fuse interaction normally advances into narration before Hwacha_Fire.
    // Preserve the launch request and retry cheaply until that authored step opens.
    if (GetWorld())
    {
        GetWorldTimerManager().SetTimer(
            PendingVolleyTimerHandle, this, &ThisClass::TryStartPendingVolley, 0.1f, false);
    }
}

void ASingijeonHwachaActor::LaunchVolley()
{
    if (HwachaState != ESingijeonHwachaState::Igniting && !IsReadyToIgnite())
    {
        return;
    }
    if (!FireScenarioInteractor ||
        !FireScenarioInteractor->CanReportInteraction(EScenarioInteractionType::Combat))
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
    bPendingLaunchAfterFuse = false;
    GetWorldTimerManager().ClearTimer(PendingVolleyTimerHandle);
    Fuse->SetIgnitionEnabled(false);
    TwoHandCarry->SetCarryEnabled(false);
    SetMoveTargetVisible(false);
    SetFuseGuideVisible(false);
    const int32 TotalAmmunition = PendingLaunchSlots.Num() +
        (AutoLoadedArrowInstances ? AutoLoadedArrowInstances->GetInstanceCount() : 0);
    CurrentLaunchInterval = TotalAmmunition > 1
        ? FMath::Max(0.0f, VolleyDuration) / static_cast<float>(TotalAmmunition - 1)
        : 0.0f;
    VolleyRandomStream.Initialize(FMath::Rand());
    LaunchNextAmmunition();
}

void ASingijeonHwachaActor::LaunchNextAmmunition()
{
    const int32 InstanceCount = AutoLoadedArrowInstances
        ? AutoLoadedArrowInstances->GetInstanceCount()
        : 0;
    const int32 TotalRemaining = PendingLaunchSlots.Num() + InstanceCount;
    if (TotalRemaining <= 0)
    {
        FinishVolley();
        return;
    }

    const int32 RandomIndex = VolleyRandomStream.RandRange(0, TotalRemaining - 1);
    if (RandomIndex < PendingLaunchSlots.Num())
    {
        USingijeonAmmoSlotComponent* Slot = PendingLaunchSlots[RandomIndex];
        PendingLaunchSlots.RemoveAtSwap(RandomIndex, 1, EAllowShrinking::No);
        if (IsValid(Slot))
        {
            AActor* Ammunition = Slot->GetLoadedAmmunition();
            const FVector LaunchLocation = IsValid(Ammunition)
                ? Ammunition->GetActorLocation()
                : Slot->GetComponentLocation();
            FVector BaseDirection = Slot->GetForwardVector();
            if (const ASingijeonProjectileActor* Projectile = Cast<ASingijeonProjectileActor>(Ammunition))
            {
                BaseDirection = Projectile->GetArrowTipDirection();
            }
            const FVector LaunchDirection = SingijeonLaunchSpread::Apply(
                BaseDirection,
                VolleyRandomStream,
                VolleyHorizontalSpreadHalfAngle,
                VolleyVerticalSpreadHalfAngle);
            ConfigureLaunchedAmmunitionCollision(Ammunition);
            if (Slot->LaunchLoadedAmmunition(LaunchDirection, LaunchSpeed))
            {
                PlayArrowLaunchSound(LaunchLocation);
            }
        }
    }
    else
    {
        LaunchNextAutoFilledAmmunition(RandomIndex - PendingLaunchSlots.Num());
    }

    const int32 RemainingAfterLaunch = PendingLaunchSlots.Num() +
        (AutoLoadedArrowInstances ? AutoLoadedArrowInstances->GetInstanceCount() : 0);
    OnLoadCountChanged.Broadcast(RemainingAfterLaunch, GetAmmunitionCapacity());
    if (RemainingAfterLaunch <= 0)
    {
        FinishVolley();
    }
    else if (CurrentLaunchInterval <= KINDA_SMALL_NUMBER)
    {
        LaunchNextAmmunition();
    }
    else
    {
        GetWorldTimerManager().SetTimer(LaunchTimerHandle, this, &ThisClass::LaunchNextAmmunition,
            CurrentLaunchInterval, false);
    }
}

void ASingijeonHwachaActor::FinishVolley()
{
    GetWorldTimerManager().ClearTimer(LaunchTimerHandle);
    PendingLaunchSlots.Reset();
    bPendingLaunchAfterFuse = false;
    bAutoFilled = false;
    AutoFilledAmmunitionClass = nullptr;
    if (FireScenarioInteractor)
    {
        FireScenarioInteractor->ReportInteractionCompleted(EScenarioInteractionType::Combat);
    }
    OnVolleyLaunched.Broadcast();
}

void ASingijeonHwachaActor::HandleCarryStateChanged(const bool bIsCarrying)
{
    if (bAimInteractionComplete)
    {
        SetHandleHighlightsVisible(false);
        SetActorTickEnabled(false);
        return;
    }

    // Legacy handle proxies remain optional. The destination marker stays fixed
    // in world space and therefore remains useful while either hand is dragging.
    SetHandleHighlightsVisible(bEnableAimGuideHighlight && !bIsCarrying && bHasAimStartTransform);
    SetActorTickEnabled(bIsCarrying && bHasAimStartTransform);
}

bool ASingijeonHwachaActor::HandleVRGrabbed(
    USceneComponent* GrabComponent,
    UMotionControllerComponent* MotionController)
{
    if (!TwoHandCarry || !IsValid(MotionController))
    {
        return false;
    }
    if (GrabComponent == LeftHandleGrabPoint || GrabComponent == LeftHandleHighlight)
    {
        return TwoHandCarry->BeginGrip(ECarryGripSide::Left, MotionController);
    }
    if (GrabComponent == RightHandleGrabPoint || GrabComponent == RightHandleHighlight)
    {
        return TwoHandCarry->BeginGrip(ECarryGripSide::Right, MotionController);
    }
    return false;
}

void ASingijeonHwachaActor::HandleVRReleased(
    USceneComponent* GrabComponent,
    UMotionControllerComponent*)
{
    if (!TwoHandCarry)
    {
        return;
    }
    if (GrabComponent == LeftHandleGrabPoint || GrabComponent == LeftHandleHighlight)
    {
        TwoHandCarry->EndGrip(ECarryGripSide::Left, nullptr);
    }
    else if (GrabComponent == RightHandleGrabPoint || GrabComponent == RightHandleHighlight)
    {
        TwoHandCarry->EndGrip(ECarryGripSide::Right, nullptr);
    }
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

    FVector TargetLocation = MoveTargetMarker->GetComponentLocation();
    TargetLocation.Z = AimStartTransform.GetLocation().Z;
    const FRotator TargetRotation = MoveTargetMarker->GetComponentRotation();
    bAimInteractionComplete = true;
    TwoHandCarry->SetCarryEnabled(false);
    SetActorLocationAndRotation(TargetLocation, TargetRotation, false, nullptr, ETeleportType::TeleportPhysics);
    SetHandleHighlightsVisible(false);
    SetMoveTargetVisible(false);
    SetActorTickEnabled(false);
    SetFuseGuideVisible(IsReadyToIgnite());
    OnAimCompleted.Broadcast();
    return true;
}

void ASingijeonHwachaActor::SetMoveTargetVisible(const bool bVisible)
{
    if (MoveTargetMarker)
    {
        MoveTargetMarker->SetVisibility(bVisible, true);
        MoveTargetMarker->SetHiddenInGame(!bVisible, true);
    }
}

void ASingijeonHwachaActor::SetFuseIgnitionEffectActive(const bool bActive)
{
    bFuseIgnitionEffectRequested = bActive;
    if (!FuseIgnitionEffect)
    {
        return;
    }

    if (bActive)
    {
        if (!FuseIgnitionEffect->IsActive())
        {
            FuseIgnitionEffect->Activate(false);
        }
        FuseIgnitionEffect->SetPaused(false);
        FuseIgnitionEffect->SetRenderingEnabled(true);
    }
    else
    {
        FuseIgnitionEffect->SetRenderingEnabled(false);
        FuseIgnitionEffect->SetPaused(true);
    }
}

void ASingijeonHwachaActor::SetFuseGuideVisible(const bool bVisible)
{
    if (FuseGuide)
    {
        FuseGuide->SetVisibility(bVisible, true);
        FuseGuide->SetHiddenInGame(!bVisible, true);
    }
}

bool ASingijeonHwachaActor::IsFuseGuideVisible() const
{
    return FuseGuide && FuseGuide->IsVisible();
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

bool ASingijeonHwachaActor::LaunchNextAutoFilledAmmunition(const int32 InstanceIndex)
{
    if (!AutoLoadedArrowInstances || AutoLoadedArrowInstances->GetInstanceCount() == 0 ||
        !AutoFilledAmmunitionClass || !AutoLoadedArrowInstances->IsValidInstance(InstanceIndex))
    {
        return false;
    }

    FTransform LaunchTransform;
    if (!AutoLoadedArrowInstances->GetInstanceTransform(InstanceIndex, LaunchTransform, true))
    {
        return false;
    }
    if (!AutoLoadedArrowInstances->RemoveInstance(InstanceIndex))
    {
        return false;
    }
    AutoLoadedArrowInstances->MarkRenderStateDirty();

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.Owner = this;
    SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* SpawnedAmmunition = GetWorld()->SpawnActor<AActor>(
        AutoFilledAmmunitionClass, LaunchTransform, SpawnParameters);
    if (IsValid(SpawnedAmmunition) &&
        SpawnedAmmunition->GetClass()->ImplementsInterface(USingijeonAmmunitionInterface::StaticClass()))
    {
        ConfigureLaunchedAmmunitionCollision(SpawnedAmmunition);
        const FVector BaseDirection = LaunchTransform.TransformVectorNoScale(
            -FVector::ForwardVector).GetSafeNormal();
        const FVector LaunchDirection = SingijeonLaunchSpread::Apply(
            BaseDirection,
            VolleyRandomStream,
            VolleyHorizontalSpreadHalfAngle,
            VolleyVerticalSpreadHalfAngle);
        ISingijeonAmmunitionInterface::Execute_OnLaunched(
            SpawnedAmmunition,
            LaunchDirection,
            LaunchSpeed);
        PlayArrowLaunchSound(LaunchTransform.GetLocation());
    }
    return true;
}

void ASingijeonHwachaActor::PlayArrowLaunchSound(const FVector& LaunchLocation) const
{
    if (!ArrowLaunchSound || !GetWorld())
    {
        return;
    }

    const float PitchMin = FMath::Min(ArrowLaunchSoundPitchMin, ArrowLaunchSoundPitchMax);
    const float PitchMax = FMath::Max(ArrowLaunchSoundPitchMin, ArrowLaunchSoundPitchMax);
    UGameplayStatics::PlaySoundAtLocation(
        this,
        ArrowLaunchSound,
        LaunchLocation,
        ArrowLaunchSoundVolume,
        FMath::FRandRange(PitchMin, PitchMax));
}

void ASingijeonHwachaActor::ConfigureLaunchedAmmunitionCollision(AActor* Ammunition)
{
    if (!IsValid(Ammunition))
    {
        return;
    }

    Ammunition->SetOwner(this);
    UPrimitiveComponent* AmmunitionPrimitive = Cast<UPrimitiveComponent>(Ammunition->GetRootComponent());
    if (AmmunitionPrimitive)
    {
        AmmunitionPrimitive->IgnoreActorWhenMoving(this, true);
    }

    for (int32 Index = ActiveLaunchedAmmunition.Num() - 1; Index >= 0; --Index)
    {
        AActor* ExistingAmmunition = ActiveLaunchedAmmunition[Index].Get();
        if (!IsValid(ExistingAmmunition))
        {
            ActiveLaunchedAmmunition.RemoveAtSwap(Index, 1, EAllowShrinking::No);
            continue;
        }

        UPrimitiveComponent* ExistingPrimitive = Cast<UPrimitiveComponent>(
            ExistingAmmunition->GetRootComponent());
        if (AmmunitionPrimitive)
        {
            AmmunitionPrimitive->IgnoreActorWhenMoving(ExistingAmmunition, true);
        }
        if (ExistingPrimitive)
        {
            ExistingPrimitive->IgnoreActorWhenMoving(Ammunition, true);
        }
    }
    ActiveLaunchedAmmunition.Add(Ammunition);
}

void ASingijeonHwachaActor::ResetHwacha()
{
    GetWorldTimerManager().ClearTimer(LaunchTimerHandle);
    GetWorldTimerManager().ClearTimer(PendingVolleyTimerHandle);
    PendingLaunchSlots.Reset();
    CurrentLaunchInterval = 0.0f;
    ActiveLaunchedAmmunition.Reset();
    bPendingLaunchAfterFuse = false;
    ClearAutoFilledAmmunition();
    bAutoFillInProgress = false;
    bLoadInteractionReported = false;
    bAimInteractionComplete = false;
    SetFuseIgnitionEffectActive(false);
    bHasAimStartTransform = false;
    TwoHandCarry->ClearConstrainedWorldZ();
    SetHandleHighlightsVisible(false);
    SetMoveTargetVisible(false);
    SetFuseGuideVisible(false);
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
    TwoHandCarry->SetCarryEnabled(bHasEnoughAmmo && !bAimInteractionComplete);
    if (bHasEnoughAmmo && !bHasAimStartTransform)
    {
        AimStartTransform = GetActorTransform();
        bHasAimStartTransform = true;
        bAimInteractionComplete = false;
        TwoHandCarry->SetConstrainedWorldZ(AimStartTransform.GetLocation().Z);
    }
    else if (!bHasEnoughAmmo)
    {
        bHasAimStartTransform = false;
        bAimInteractionComplete = false;
        TwoHandCarry->ClearConstrainedWorldZ();
    }
    SetHandleHighlightsVisible(
        bEnableAimGuideHighlight && bHasEnoughAmmo && !bAimInteractionComplete &&
        !TwoHandCarry->IsBeingCarried());
    SetMoveTargetVisible(
        bShowMoveTargetMarker && bHasEnoughAmmo && !bAimInteractionComplete);
    SetFuseGuideVisible(
        bHasEnoughAmmo && bAimInteractionComplete &&
        HwachaState != ESingijeonHwachaState::Igniting);
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
