#include "Singijeon/SingijeonHwachaBatteryActor.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Singijeon/SingijeonLaunchSpread.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

ASingijeonHwachaBatteryActor::ASingijeonHwachaBatteryActor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    HwachaInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("HwachaInstances"));
    HwachaInstances->SetupAttachment(SceneRoot);
    ConfigureInstanceComponent(HwachaInstances, true);

    LoadedArrowInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("LoadedArrowInstances"));
    LoadedArrowInstances->SetupAttachment(SceneRoot);
    ConfigureInstanceComponent(LoadedArrowInstances, false);

    FlyingArrowInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("FlyingArrowInstances"));
    FlyingArrowInstances->SetupAttachment(SceneRoot);
    ConfigureInstanceComponent(FlyingArrowInstances, false);

    RackEditorGuide = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowRackEditorGuide"));
    RackEditorGuide->SetupAttachment(SceneRoot);
    RackEditorGuide->SetArrowColor(FColor::Cyan);
    RackEditorGuide->ArrowSize = 1.5f;
    RackEditorGuide->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RackEditorGuide->SetHiddenInGame(true);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> HwachaMeshFinder(
        TEXT("/GF_Singijeon/Asset/Hwacha/hwacha/StaticMeshes/hwacha.hwacha"));
    HwachaMesh = HwachaMeshFinder.Succeeded() ? HwachaMeshFinder.Object : nullptr;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> ArrowMeshFinder(
        TEXT("/GF_Singijeon/Asset/Arrow/arrowb/StaticMeshes/arrowb.arrowb"));
    ArrowMesh = ArrowMeshFinder.Succeeded() ? ArrowMeshFinder.Object : nullptr;

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> ArrowMaterialFinder(
        TEXT("/GF_Singijeon/Asset/Arrow/arrowb/Materials/M_SingijeonArrow_Runtime.M_SingijeonArrow_Runtime"));
    ArrowMaterial = ArrowMaterialFinder.Succeeded() ? ArrowMaterialFinder.Object : nullptr;

    static ConstructorHelpers::FObjectFinder<USoundBase> LaunchSoundFinder(
        TEXT("/GF_Singijeon/Asset/Sound/Effect/SingijeonLaunch.SingijeonLaunch"));
    LaunchSound = LaunchSoundFinder.Succeeded() ? LaunchSoundFinder.Object : nullptr;

}

void ASingijeonHwachaBatteryActor::ConfigureInstanceComponent(
    UInstancedStaticMeshComponent* Component, const bool bCastShadow) const
{
    if (!Component)
    {
        return;
    }
    Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Component->SetGenerateOverlapEvents(false);
    Component->SetCanEverAffectNavigation(false);
    Component->SetCastShadow(bCastShadow);
    Component->SetReceivesDecals(false);
}

void ASingijeonHwachaBatteryActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    RebuildInstances();
}

void ASingijeonHwachaBatteryActor::BeginPlay()
{
    Super::BeginPlay();
    RebuildInstances();
    ResolveSourceHwacha();
    if (SourceHwacha)
    {
        SourceHwacha->OnHwachaStateChanged.AddUniqueDynamic(
            this, &ASingijeonHwachaBatteryActor::HandleSourceHwachaStateChanged);
        if (SourceHwacha->GetHwachaState() == ESingijeonHwachaState::Fired)
        {
            StartSynchronizedVolley();
        }
    }
}

void ASingijeonHwachaBatteryActor::ResolveSourceHwacha()
{
    if (IsValid(SourceHwacha))
    {
        return;
    }

    float BestDistanceSquared = TNumericLimits<float>::Max();
    for (TActorIterator<ASingijeonHwachaActor> It(GetWorld()); It; ++It)
    {
        const float DistanceSquared = FVector::DistSquared(GetActorLocation(), It->GetActorLocation());
        if (DistanceSquared < BestDistanceSquared)
        {
            SourceHwacha = *It;
            BestDistanceSquared = DistanceSquared;
        }
    }
}

void ASingijeonHwachaBatteryActor::RebuildInstances()
{
    if (!HwachaInstances || !LoadedArrowInstances || !FlyingArrowInstances)
    {
        return;
    }

    HwachaInstances->SetStaticMesh(HwachaMesh);
    LoadedArrowInstances->SetStaticMesh(ArrowMesh);
    FlyingArrowInstances->SetStaticMesh(ArrowMesh);
    HwachaInstances->SetCullDistances(StartCullDistance, EndCullDistance);
    LoadedArrowInstances->SetCullDistances(StartCullDistance, EndCullDistance);
    FlyingArrowInstances->SetCullDistances(StartCullDistance, EndCullDistance);
    ApplyArrowMaterial(LoadedArrowInstances);
    ApplyArrowMaterial(FlyingArrowInstances);
    RackEditorGuide->SetRelativeTransform(RackTransform);

    HwachaInstances->ClearInstances();
    LoadedArrowInstances->ClearInstances();
    FlyingArrowInstances->ClearInstances();
    FlyingArrows.Reset();

    HwachaInstances->AddInstance(FTransform::Identity);
    for (int32 Row = 0; Row < FMath::Max(1, ArrowRows); ++Row)
    {
        for (int32 Column = 0; Column < FMath::Max(1, ArrowColumns); ++Column)
        {
            FTransform ArrowTransform = RackTransform;
            ArrowTransform.AddToTranslation(FVector(
                0.0f,
                Column * ArrowColumnSpacing,
                Row * ArrowRowSpacing));
            LoadedArrowInstances->AddInstance(ArrowTransform);
        }
    }
}

void ASingijeonHwachaBatteryActor::ApplyArrowMaterial(
    UInstancedStaticMeshComponent* Component) const
{
    if (!Component || !ArrowMaterial)
    {
        return;
    }
    const int32 MaterialCount = FMath::Max(1, Component->GetNumMaterials());
    for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
    {
        Component->SetMaterial(MaterialIndex, ArrowMaterial);
    }
}

void ASingijeonHwachaBatteryActor::HandleSourceHwachaStateChanged(
    const ESingijeonHwachaState OldState, const ESingijeonHwachaState NewState)
{
    if (NewState == ESingijeonHwachaState::Fired && OldState != ESingijeonHwachaState::Fired)
    {
        StartSynchronizedVolley();
    }
    else if (NewState == ESingijeonHwachaState::Empty)
    {
        ResetBattery();
    }
}

void ASingijeonHwachaBatteryActor::StartSynchronizedVolley()
{
    if (bVolleyActive || !LoadedArrowInstances || LoadedArrowInstances->GetInstanceCount() <= 0)
    {
        return;
    }

    bVolleyActive = true;
    VolleyElapsed = 0.0f;
    NextLaunchTime = 0.0f;
    VisualUpdateAccumulator = 0.0f;
    LaunchedArrowCount = 0;
    LaunchRandomStream.Initialize(FMath::Rand());
    const int32 ArrowCount = LoadedArrowInstances->GetInstanceCount();
    LaunchInterval = ArrowCount > 1 ? FMath::Max(0.0f, VolleyDuration) / (ArrowCount - 1) : 0.0f;
    SetActorTickEnabled(true);
    LaunchOneArrow();
    NextLaunchTime = LaunchInterval;
}

void ASingijeonHwachaBatteryActor::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bVolleyActive)
    {
        VolleyElapsed += DeltaSeconds;
        while (LoadedArrowInstances->GetInstanceCount() > 0 &&
            (LaunchInterval <= KINDA_SMALL_NUMBER || VolleyElapsed + KINDA_SMALL_NUMBER >= NextLaunchTime))
        {
            LaunchOneArrow();
            NextLaunchTime += LaunchInterval;
        }
        if (LoadedArrowInstances->GetInstanceCount() <= 0)
        {
            bVolleyActive = false;
        }
    }

    VisualUpdateAccumulator += DeltaSeconds;
    const float UpdateStep = FMath::Max(VisualUpdateInterval, 0.016f);
    if (VisualUpdateAccumulator >= UpdateStep && FlyingArrows.Num() > 0)
    {
        const float AppliedStep = VisualUpdateAccumulator;
        VisualUpdateAccumulator = 0.0f;
        UpdateFlyingArrows(AppliedStep);
    }
    StopTickIfIdle();
}

void ASingijeonHwachaBatteryActor::LaunchOneArrow()
{
    const int32 LoadedCount = LoadedArrowInstances ? LoadedArrowInstances->GetInstanceCount() : 0;
    if (LoadedCount <= 0)
    {
        return;
    }

    const int32 InstanceIndex = LaunchRandomStream.RandRange(0, LoadedCount - 1);
    FTransform WorldTransform;
    if (!LoadedArrowInstances->GetInstanceTransform(InstanceIndex, WorldTransform, true))
    {
        return;
    }
    LoadedArrowInstances->RemoveInstance(InstanceIndex);

    const FVector BaseDirection = WorldTransform.TransformVectorNoScale(
        -FVector::ForwardVector).GetSafeNormal();
    const FVector Direction = SingijeonLaunchSpread::Apply(
        BaseDirection,
        LaunchRandomStream,
        VolleyHorizontalSpreadHalfAngle,
        VolleyVerticalSpreadHalfAngle);
    const FQuat TipAlignment = FQuat::FindBetweenNormals(BaseDirection, Direction);
    WorldTransform.SetRotation(TipAlignment * WorldTransform.GetRotation());
    FFlyingArrowState& State = FlyingArrows.AddDefaulted_GetRef();
    State.Location = WorldTransform.GetLocation();
    State.Velocity = Direction * LaunchSpeed;
    State.RemainingLifetime = FlyingArrowLifetime;
    FlyingArrowInstances->AddInstance(WorldTransform, true);

    ++LaunchedArrowCount;
    if (LaunchSound && SoundEveryNthArrow > 0 && (LaunchedArrowCount % SoundEveryNthArrow) == 1)
    {
        UGameplayStatics::PlaySoundAtLocation(this, LaunchSound, State.Location, 0.7f,
            LaunchRandomStream.FRandRange(0.94f, 1.06f));
    }
}

void ASingijeonHwachaBatteryActor::UpdateFlyingArrows(const float StepSeconds)
{
    bool bTransformsChanged = false;
    for (int32 Index = FlyingArrows.Num() - 1; Index >= 0; --Index)
    {
        FFlyingArrowState& State = FlyingArrows[Index];
        State.RemainingLifetime -= StepSeconds;
        if (State.RemainingLifetime <= 0.0f)
        {
            FlyingArrowInstances->RemoveInstance(Index);
            FlyingArrows.RemoveAtSwap(Index, 1, EAllowShrinking::No);
            bTransformsChanged = true;
            continue;
        }

        State.Location += State.Velocity * StepSeconds + FVector(0.0f, 0.0f, 0.5f * GravityZ * StepSeconds * StepSeconds);
        State.Velocity.Z += GravityZ * StepSeconds;
        const FRotator Rotation = FRotationMatrix::MakeFromX(-State.Velocity.GetSafeNormal()).Rotator();
        FlyingArrowInstances->UpdateInstanceTransform(
            Index, FTransform(Rotation, State.Location), true, false, false);
        bTransformsChanged = true;
    }
    if (bTransformsChanged)
    {
        FlyingArrowInstances->MarkRenderStateDirty();
    }
}

void ASingijeonHwachaBatteryActor::ResetBattery()
{
    bVolleyActive = false;
    RebuildInstances();
    StopTickIfIdle();
}

void ASingijeonHwachaBatteryActor::StopTickIfIdle()
{
    if (!bVolleyActive && FlyingArrows.IsEmpty())
    {
        SetActorTickEnabled(false);
    }
}

int32 ASingijeonHwachaBatteryActor::GetCartCount() const
{
    return HwachaInstances ? HwachaInstances->GetInstanceCount() : 0;
}

int32 ASingijeonHwachaBatteryActor::GetLoadedArrowCount() const
{
    return LoadedArrowInstances ? LoadedArrowInstances->GetInstanceCount() : 0;
}

int32 ASingijeonHwachaBatteryActor::GetFlyingArrowCount() const
{
    return FlyingArrowInstances ? FlyingArrowInstances->GetInstanceCount() : 0;
}
