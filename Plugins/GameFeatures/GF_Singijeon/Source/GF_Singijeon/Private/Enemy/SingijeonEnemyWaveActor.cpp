#include "Enemy/SingijeonEnemyWaveActor.h"

#include "Animation/AnimationAsset.h"
#include "Animation/AnimSequenceTransformProviderData.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/InstancedSkinnedMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Shared/Characters/EnemySoldierActor.h"
#include "Shared/Combat/LegacyHealthComponent.h"
#include "Singijeon/SingijeonHwachaActor.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"

ASingijeonEnemyWaveActor::ASingijeonEnemyWaveActor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    MarchAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MarchAudioComponent"));
    MarchAudioComponent->SetupAttachment(SceneRoot);
    MarchAudioComponent->bAutoActivate = false;
    MarchAudioComponent->bAllowSpatialization = false;
    MarchAudioComponent->OnAudioFinished.AddDynamic(this, &ThisClass::HandleMarchAudioFinished);

    SpawnVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnVolume"));
    SpawnVolume->SetupAttachment(SceneRoot);
    SpawnVolume->SetBoxExtent(FVector(500.0f, 700.0f, 100.0f));
    SpawnVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SpawnVolume->SetHiddenInGame(true);

    DefaultTargetPoint = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultTargetPoint"));
    DefaultTargetPoint->SetupAttachment(SceneRoot);
    DefaultTargetPoint->SetRelativeLocation(FVector(10000.0f, 0.0f, 0.0f));

    ProxyInstances = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("ProxyInstances"));
    ProxyInstances->SetupAttachment(SceneRoot);
    ProxyInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ProxyInstances->SetGenerateOverlapEvents(false);
    ProxyInstances->SetCanEverAffectNavigation(false);
    ProxyInstances->CastShadow = false;
    ProxyInstances->bCastDynamicShadow = false;
    ProxyInstances->bCastStaticShadow = false;
    ProxyInstances->bReceivesDecals = false;
    ProxyInstances->NumCustomDataFloats = 2;
    ProxyInstances->SetVisibility(false, true);
    ProxyInstances->SetHiddenInGame(true, true);

    CharacterInstances = CreateDefaultSubobject<UInstancedSkinnedMeshComponent>(TEXT("CharacterInstances"));
    CharacterInstances->SetupAttachment(SceneRoot);
    CharacterInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CharacterInstances->SetGenerateOverlapEvents(false);
    CharacterInstances->SetCanEverAffectNavigation(false);
    CharacterInstances->CastShadow = false;
    CharacterInstances->bCastDynamicShadow = false;
    CharacterInstances->bCastStaticShadow = false;
    CharacterInstances->bReceivesDecals = false;
    CharacterInstances->SetHasPerInstancePrevTransforms(false);
    CharacterInstances->SetVisibility(false, true);
    CharacterInstances->SetHiddenInGame(true, true);

    ForegroundEnemyClass = AEnemySoldierActor::StaticClass();

    static ConstructorHelpers::FObjectFinder<USkeletalMesh> ProxySkeletalMeshFinder(
        TEXT("/Game/NiagaraExamples/Gallery/SkeletalMesh/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
    if (ProxySkeletalMeshFinder.Succeeded())
    {
        ProxySkeletalMesh = ProxySkeletalMeshFinder.Object;
        CharacterInstances->SetSkinnedAssetAndUpdate(ProxySkeletalMesh);
    }

    static ConstructorHelpers::FObjectFinder<UAnimationAsset> RunAnimationFinder(
        TEXT("/Game/NiagaraExamples/Gallery/SkeletalMesh/Mannequins/Anims/Rifle/Jog/MF_Rifle_Jog_Fwd.MF_Rifle_Jog_Fwd"));
    if (RunAnimationFinder.Succeeded())
    {
        RuntimeRunAnimationFallback = RunAnimationFinder.Object;
        ForegroundRunAnimation = RunAnimationFinder.Object;
    }

    static ConstructorHelpers::FObjectFinder<UAnimSequenceTransformProviderData> AnimationProviderFinder(
        TEXT("/GF_Singijeon/Gameplay/Enemy/Samurai/DA_SingijeonSamuraiRifleRun_GPU.DA_SingijeonSamuraiRifleRun_GPU"));
    if (AnimationProviderFinder.Succeeded())
    {
        ProxyAnimationProvider = AnimationProviderFinder.Object;
        CharacterInstances->SetTransformProvider(ProxyAnimationProvider);
    }

    static ConstructorHelpers::FObjectFinder<USoundBase> MarchSoundFinder(
        TEXT("/GF_Singijeon/Asset/Sound/Effect/Troop_march_2.Troop_march_2"));
    if (MarchSoundFinder.Succeeded())
    {
        MarchSound = MarchSoundFinder.Object;
        MarchAudioComponent->SetSound(MarchSound);
    }

}

void ASingijeonEnemyWaveActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    EnemyCount = FMath::Clamp(EnemyCount, 1, 80);
    MaxInteractiveEnemies = FMath::Clamp(MaxInteractiveEnemies, 0, FMath::Min(16, EnemyCount));
    PlatoonCount = FMath::Clamp(PlatoonCount, 1, FMath::Min(8, EnemyCount));
    FormationColumnsPerPlatoon = FMath::Max(1, FormationColumnsPerPlatoon);
    ProxyUpdateInterval = FMath::Max(0.016f, ProxyUpdateInterval);
    ProxyMinLOD = FMath::Clamp(ProxyMinLOD, 0, 4);
    DesiredEnemyHeight = FMath::Clamp(DesiredEnemyHeight, 100.0f, 240.0f);
    SharedPoseLeaderCount = FMath::Clamp(SharedPoseLeaderCount, 1, 16);
    MinRunAnimationRate = FMath::Clamp(MinRunAnimationRate, 0.1f, 3.0f);
    MaxRunAnimationRate = FMath::Clamp(MaxRunAnimationRate, MinRunAnimationRate, 3.0f);
    LoadedApproachLimit = FMath::Clamp(LoadedApproachLimit, 0.0f, 1.0f);
    AimedApproachLimit = FMath::Clamp(AimedApproachLimit, LoadedApproachLimit, 1.0f);
    IgnitingApproachLimit = FMath::Clamp(IgnitingApproachLimit, AimedApproachLimit, 1.0f);
    FiringApproachLimit = FMath::Clamp(FiringApproachLimit, IgnitingApproachLimit, 1.0f);
    PanicDuration = FMath::Clamp(PanicDuration, 0.0f, 10.0f);
    PanicLateralDistance = FMath::Max(0.0f, PanicLateralDistance);
    PanicRetreatDistance = FMath::Max(0.0f, PanicRetreatDistance);
    PanicAnimationRateMultiplier = FMath::Clamp(PanicAnimationRateMultiplier, 0.1f, 3.0f);
    if (ProxyInstances)
    {
        // Serialized actors may still contain the old target-disc mesh. Keeping
        // the native subobject empty removes it without forcing a level migration.
        ProxyMesh = nullptr;
        ProxyInstances->SetStaticMesh(nullptr);
        ProxyInstances->ClearInstances();
        ProxyInstances->SetVisibility(false, true);
        ProxyInstances->SetHiddenInGame(true, true);
    }
    if (CharacterInstances)
    {
        CharacterInstances->SetSkinnedAssetAndUpdate(ProxySkeletalMesh);
        CharacterInstances->SetTransformProvider(ProxyAnimationProvider);
        CharacterInstances->SetCullDistances(ProxyStartCullDistance, ProxyEndCullDistance);
        CharacterInstances->SetAnimationMinScreenSize(ProxyAnimationMinScreenSize);
        CharacterInstances->OverrideMinLOD(ProxyMinLOD);
    }
#if WITH_EDITOR
    RefreshEditorEnemyPreview();
#endif
}

void ASingijeonEnemyWaveActor::BeginPlay()
{
    Super::BeginPlay();
#if WITH_EDITOR
    DestroyEditorEnemyPreview();
#endif
    FindAndBindHwacha();
    PrepareWave();
    RefreshProcedureStateFromHwacha();
    if (bStartOnBeginPlay)
    {
        StartWave();
    }
}

void ASingijeonEnemyWaveActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopMarchAudio();
#if WITH_EDITOR
    DestroyEditorEnemyPreview();
#endif
    UnbindHwacha();
    DestroyVisualRepresentations();
    Super::EndPlay(EndPlayReason);
}

void ASingijeonEnemyWaveActor::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (WaveState == ESingijeonEnemyWaveState::Panicking)
    {
        PanicElapsed += FMath::Max(0.0f, DeltaSeconds);
        UpdateInteractiveEnemies();
        ProxyUpdateAccumulator += FMath::Max(0.0f, DeltaSeconds);
        if (ProxyUpdateAccumulator >= ProxyUpdateInterval)
        {
            ProxyUpdateAccumulator = FMath::Fmod(ProxyUpdateAccumulator, ProxyUpdateInterval);
            UpdateProxyEnemies(true);
        }
        if (PanicElapsed >= PanicDuration && bVolleyResolutionPending)
        {
            CompletePanicResolution();
        }
        return;
    }
    if (WaveState != ESingijeonEnemyWaveState::Charging)
    {
        SetActorTickEnabled(false);
        return;
    }

    const float MaximumDistance = RouteLength * CurrentApproachLimitFraction;
    if (WaveDistance < MaximumDistance)
    {
        WaveDistance = FMath::Min(
            WaveDistance + FMath::Max(0.0f, DeltaSeconds) * ChargeSpeed,
            MaximumDistance);
    }
    UpdateInteractiveEnemies();

    ProxyUpdateAccumulator += FMath::Max(0.0f, DeltaSeconds);
    if (ProxyUpdateAccumulator >= ProxyUpdateInterval)
    {
        ProxyUpdateAccumulator = FMath::Fmod(ProxyUpdateAccumulator, ProxyUpdateInterval);
        UpdateProxyEnemies(true);
    }

    for (const FEnemySlot& Slot : EnemySlots)
    {
        if (Slot.bAlive && GetSlotRouteDistance(Slot) >= RouteLength)
        {
            SetWaveState(ESingijeonEnemyWaveState::ReachedTarget);
            SetActorTickEnabled(false);
            OnWaveReachedTarget.Broadcast();
            return;
        }
    }

    if (CurrentApproachLimitFraction < 1.0f &&
        WaveDistance >= MaximumDistance - KINDA_SMALL_NUMBER)
    {
        // Keep all visual state, but stop manager Tick while the player performs
        // the next Hwacha step. SetProcedureApproachPhase wakes it again.
        SetActorTickEnabled(false);
    }
}

#if WITH_EDITOR
void ASingijeonEnemyWaveActor::RefreshEditorEnemyPreview()
{
    DestroyEditorEnemyPreview();
    UWorld* World = GetWorld();
    if (!GIsEditor || !World || World->IsGameWorld() || IsTemplate() ||
        !bShowEnemyPreviewInEditor || !ProxySkeletalMesh || !SceneRoot)
    {
        return;
    }

    if (!RebuildRoute())
    {
        return;
    }
    BuildFormationSlots();
    EditorPreviewMeshes.Reserve(EnemySlots.Num());
    for (int32 Index = 0; Index < EnemySlots.Num(); ++Index)
    {
        FEnemySlot PreviewSlot = EnemySlots[Index];
        PreviewSlot.ReliableProxyIndex = Index;
        const FName PreviewName = MakeUniqueObjectName(
            this,
            USkeletalMeshComponent::StaticClass(),
            *FString::Printf(TEXT("EnemyPreview_%02d"), Index + 1));
        USkeletalMeshComponent* Preview = NewObject<USkeletalMeshComponent>(
            this, PreviewName, RF_Transient | RF_TextExportTransient);
        if (!Preview)
        {
            continue;
        }

        Preview->CreationMethod = EComponentCreationMethod::Instance;
        Preview->SetupAttachment(SceneRoot);
        Preview->SetSkeletalMesh(ProxySkeletalMesh);
        Preview->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Preview->SetGenerateOverlapEvents(false);
        Preview->SetCanEverAffectNavigation(false);
        Preview->SetCastShadow(false);
        Preview->bCastDynamicShadow = false;
        Preview->bCastStaticShadow = false;
        Preview->bReceivesDecals = false;
        Preview->SetComponentTickEnabled(false);
        Preview->OverrideMinLOD(ProxyMinLOD);
        Preview->bIsEditorOnly = true;
        // Editor-only objects are stripped from cooked builds, so keep the
        // visibility flag on for both viewport drawing and editor automation.
        Preview->SetHiddenInGame(false, true);
        AddInstanceComponent(Preview);
        Preview->RegisterComponent();
        Preview->SetWorldTransform(GetSlotTransform(PreviewSlot));
        Preview->SetVisibility(true, true);
        EditorPreviewMeshes.Add(Preview);
    }
}

void ASingijeonEnemyWaveActor::DestroyEditorEnemyPreview()
{
    TInlineComponentArray<USkeletalMeshComponent*> SkeletalComponents(this);
    for (USkeletalMeshComponent* Preview : SkeletalComponents)
    {
        if (IsValid(Preview) &&
            (EditorPreviewMeshes.Contains(Preview) ||
             Preview->GetName().StartsWith(TEXT("EnemyPreview_"))))
        {
            RemoveInstanceComponent(Preview);
            Preview->DestroyComponent();
        }
    }
    EditorPreviewMeshes.Reset();
}
#endif

void ASingijeonEnemyWaveActor::SetProcedureApproachPhase(
    const ESingijeonEnemyApproachPhase NewPhase)
{
    ApproachPhase = bLimitApproachByHwachaProcedure
        ? NewPhase
        : ESingijeonEnemyApproachPhase::Unrestricted;

    float DesiredLimit = 1.0f;
    switch (ApproachPhase)
    {
    case ESingijeonEnemyApproachPhase::Waiting:
        DesiredLimit = 0.0f;
        break;
    case ESingijeonEnemyApproachPhase::Loaded:
        DesiredLimit = LoadedApproachLimit;
        break;
    case ESingijeonEnemyApproachPhase::Aimed:
        DesiredLimit = AimedApproachLimit;
        break;
    case ESingijeonEnemyApproachPhase::Igniting:
        DesiredLimit = IgnitingApproachLimit;
        break;
    case ESingijeonEnemyApproachPhase::Firing:
        DesiredLimit = FiringApproachLimit;
        break;
    case ESingijeonEnemyApproachPhase::Unrestricted:
    default:
        DesiredLimit = 1.0f;
        break;
    }

    DesiredLimit = FMath::Clamp(DesiredLimit, 0.0f, 1.0f);
    if (WaveState == ESingijeonEnemyWaveState::Charging)
    {
        // Procedure cancellation never teleports a formation backwards.
        CurrentApproachLimitFraction = FMath::Max(CurrentApproachLimitFraction, DesiredLimit);
        if (RouteLength * CurrentApproachLimitFraction > WaveDistance + KINDA_SMALL_NUMBER)
        {
            SetActorTickEnabled(true);
        }
    }
    else
    {
        CurrentApproachLimitFraction = DesiredLimit;
    }
}

bool ASingijeonEnemyWaveActor::RebuildRoute()
{
    RoutePoints.Reset();
    RouteCumulativeDistances.Reset();
    RouteLength = 0.0f;

    const FVector Start = SpawnVolume ? SpawnVolume->GetComponentLocation() : GetActorLocation();
    const FVector End = GetDestinationLocation();

    if (GetWorld())
    {
        if (UNavigationPath* Path = UNavigationSystemV1::FindPathToLocationSynchronously(
            GetWorld(), Start, End, this))
        {
            if (Path->IsValid() && Path->PathPoints.Num() >= 2)
            {
                RoutePoints = Path->PathPoints;
            }
        }
    }
    if (RoutePoints.Num() < 2)
    {
        RoutePoints = {Start, End};
    }

    RouteCumulativeDistances.Reserve(RoutePoints.Num());
    RouteCumulativeDistances.Add(0.0f);
    for (int32 Index = 1; Index < RoutePoints.Num(); ++Index)
    {
        RouteLength += FVector::Distance(RoutePoints[Index - 1], RoutePoints[Index]);
        RouteCumulativeDistances.Add(RouteLength);
    }
    return RouteLength > KINDA_SMALL_NUMBER;
}

bool ASingijeonEnemyWaveActor::PrepareWave()
{
#if WITH_EDITOR
    if (GetWorld() && !GetWorld()->IsGameWorld())
    {
        DestroyEditorEnemyPreview();
    }
#endif
	// A serialized level flag or a previously hidden wave must never suppress
	// dynamically-created child meshes for the entire play session.
	SetActorHiddenInGame(false);
	if (SceneRoot)
	{
		SceneRoot->SetVisibility(true, false);
		SceneRoot->SetHiddenInGame(false, false);
	}
    StopWave(true);
    DestroyVisualRepresentations();
    if (!RebuildRoute())
    {
        return false;
    }

    BuildFormationSlots();
    if (!SpawnVisualRepresentations())
    {
        DestroyVisualRepresentations();
        return false;
    }
    WaveDistance = 0.0f;
    ProxyUpdateAccumulator = 0.0f;
    PanicElapsed = 0.0f;
    bVolleyResolutionPending = false;
    AliveEnemyCount = EnemySlots.Num();
    UpdateInteractiveEnemies();
    UpdateProxyEnemies(true);
    SetVisualsActive(bShowEnemiesWhileReady);
    SetWaveState(ESingijeonEnemyWaveState::Ready);
    return true;
}

void ASingijeonEnemyWaveActor::StartWave()
{
    if (EnemySlots.Num() != EnemyCount && !PrepareWave())
    {
        return;
    }
    if (WaveState == ESingijeonEnemyWaveState::Defeated ||
        WaveState == ESingijeonEnemyWaveState::ReachedTarget)
    {
        ResetWave();
    }
    SetVisualsActive(true);
    SetWaveState(ESingijeonEnemyWaveState::Charging);
    SetActorTickEnabled(true);
}

void ASingijeonEnemyWaveActor::StopWave(const bool bHideEnemies)
{
    SetActorTickEnabled(false);
    StopMarchAudio();
    if (bHideEnemies)
    {
        SetVisualsActive(false);
    }
}

void ASingijeonEnemyWaveActor::ResetWave()
{
    WaveDistance = 0.0f;
    ProxyUpdateAccumulator = 0.0f;
    AliveEnemyCount = EnemySlots.Num();
    PanicElapsed = 0.0f;
    bVolleyResolutionPending = false;
    SetPanicAnimationRates(false);
    for (FEnemySlot& Slot : EnemySlots)
    {
        Slot.bAlive = true;
        if (AEnemySoldierActor* Enemy = Slot.InteractiveActor.Get())
        {
            Enemy->SetSoldierActive(false);
        }
    }
    UpdateInteractiveEnemies();
    UpdateProxyEnemies(true);
    SetVisualsActive(bShowEnemiesWhileReady);
    SetWaveState(ESingijeonEnemyWaveState::Ready);
    CurrentApproachLimitFraction = 1.0f;
    ApproachPhase = ESingijeonEnemyApproachPhase::Unrestricted;
    RefreshProcedureStateFromHwacha();
}

FVector ASingijeonEnemyWaveActor::GetDestinationLocation() const
{
    if (IsValid(DestinationActor))
    {
        return DestinationActor->GetActorLocation();
    }
    if (DefaultTargetPoint)
    {
        return DefaultTargetPoint->GetComponentLocation();
    }
    return GetActorLocation() + GetActorForwardVector() * 10000.0f;
}

void ASingijeonEnemyWaveActor::BeginPanic(
    const FVector VolleyOrigin,
    const FVector VolleyDirection)
{
    if (AliveEnemyCount <= 0 ||
        (WaveState != ESingijeonEnemyWaveState::Charging &&
         WaveState != ESingijeonEnemyWaveState::Ready))
    {
        return;
    }
    PendingVolleyOrigin = VolleyOrigin;
    PendingVolleyDirection = VolleyDirection.GetSafeNormal();
    PanicElapsed = 0.0f;
    bVolleyResolutionPending = true;
    SetVisualsActive(true);
    SetPanicAnimationRates(true);
    SetWaveState(ESingijeonEnemyWaveState::Panicking);
    if (PanicDuration <= KINDA_SMALL_NUMBER)
    {
        CompletePanicResolution();
        return;
    }
    SetActorTickEnabled(true);
}

void ASingijeonEnemyWaveActor::CompletePanicResolution()
{
    bVolleyResolutionPending = false;
    ResolveVolley(PendingVolleyOrigin, PendingVolleyDirection);
    if (AliveEnemyCount <= 0)
    {
        SetPanicAnimationRates(false);
        SetActorTickEnabled(false);
        return;
    }
    SetPanicAnimationRates(false);
    SetProcedureApproachPhase(ESingijeonEnemyApproachPhase::Unrestricted);
    SetWaveState(ESingijeonEnemyWaveState::Charging);
    SetActorTickEnabled(true);
}

int32 ASingijeonEnemyWaveActor::ResolveVolley(const FVector Origin, const FVector Direction)
{
    if (AliveEnemyCount <= 0)
    {
        return 0;
    }

    struct FCandidate
    {
        int32 SlotIndex = INDEX_NONE;
        float AimScore = 0.0f;
    };
    TArray<FCandidate> Candidates;
    Candidates.Reserve(AliveEnemyCount);
    const FVector AimDirection = Direction.GetSafeNormal();
    const float MinimumDot = FMath::Cos(FMath::DegreesToRadians(VolleyHalfAngleDegrees));

    for (int32 Index = 0; Index < EnemySlots.Num(); ++Index)
    {
        const FEnemySlot& Slot = EnemySlots[Index];
        if (!Slot.bAlive)
        {
            continue;
        }
        const FVector ToEnemy = GetSlotTransform(Slot).GetLocation() - Origin;
        const float Distance = ToEnemy.Size();
        const float Dot = FVector::DotProduct(AimDirection, ToEnemy.GetSafeNormal());
        const bool bInsidePrimaryCone = Distance <= VolleyMaxDistance && Dot >= MinimumDot;
        // Cone targets sort first. Remaining enemies provide a deterministic fallback so
        // the 90-arrow educational volley cannot miss every visual proxy due to mesh axes.
        const float Score = (bInsidePrimaryCone ? 1000.0f : 0.0f) + Dot * 100.0f - Distance * 0.001f;
        Candidates.Add({Index, Score});
    }
    Candidates.Sort([](const FCandidate& Left, const FCandidate& Right)
    {
        return Left.AimScore > Right.AimScore;
    });

    const int32 DesiredCasualties = FMath::Clamp(
        FMath::CeilToInt(static_cast<float>(AliveEnemyCount) * VolleyCasualtyFraction),
        0,
        Candidates.Num());
    int32 Casualties = 0;
    for (int32 CandidateIndex = 0; CandidateIndex < DesiredCasualties; ++CandidateIndex)
    {
        Casualties += DefeatSlot(Candidates[CandidateIndex].SlotIndex) ? 1 : 0;
    }

    UpdateProxyEnemies(true);
    OnVolleyResolved.Broadcast(Casualties, AliveEnemyCount);
    return Casualties;
}

int32 ASingijeonEnemyWaveActor::GetInteractiveEnemyCount() const
{
    int32 Count = 0;
    for (const FEnemySlot& Slot : EnemySlots)
    {
        Count += Slot.InteractiveActor.IsValid() ? 1 : 0;
    }
    return Count;
}

int32 ASingijeonEnemyWaveActor::GetProxyEnemyCount() const
{
    const int32 GpuCount = CharacterInstances ? CharacterInstances->GetInstanceCount() : 0;
    return GpuCount + ReliableProxyMeshes.Num();
}

void ASingijeonEnemyWaveActor::HandleHwachaLoadCountChanged(const int32 LoadedCount, const int32)
{
    if (bLimitApproachByHwachaProcedure && LoadedCount > 0)
    {
        SetProcedureApproachPhase(ESingijeonEnemyApproachPhase::Loaded);
    }
    if (bStartWhenHwachaLoaded && LoadedCount > 0 && WaveState == ESingijeonEnemyWaveState::Ready)
    {
        StartWave();
    }
}

void ASingijeonEnemyWaveActor::HandleHwachaVolleyLaunched()
{
    if (!BoundHwacha ||
        (WaveState != ESingijeonEnemyWaveState::Charging &&
         WaveState != ESingijeonEnemyWaveState::Ready))
    {
        return;
    }
    BeginPanic(BoundHwacha->GetActorLocation(), BoundHwacha->GetActorForwardVector());
}

void ASingijeonEnemyWaveActor::HandleHwachaAimCompleted()
{
    SetProcedureApproachPhase(ESingijeonEnemyApproachPhase::Aimed);
}

void ASingijeonEnemyWaveActor::HandleHwachaStateChanged(
    const ESingijeonHwachaState,
    const ESingijeonHwachaState NewState)
{
    switch (NewState)
    {
    case ESingijeonHwachaState::Empty:
        SetProcedureApproachPhase(ESingijeonEnemyApproachPhase::Waiting);
        break;
    case ESingijeonHwachaState::Loaded:
        SetProcedureApproachPhase(BoundHwacha && BoundHwacha->IsAimInteractionComplete()
            ? ESingijeonEnemyApproachPhase::Aimed
            : ESingijeonEnemyApproachPhase::Loaded);
        break;
    case ESingijeonHwachaState::Igniting:
        SetProcedureApproachPhase(ESingijeonEnemyApproachPhase::Igniting);
        break;
    case ESingijeonHwachaState::Fired:
        SetProcedureApproachPhase(ESingijeonEnemyApproachPhase::Firing);
        break;
    }
}

void ASingijeonEnemyWaveActor::HandleInteractiveEnemyDepleted(AActor* OwnerActor)
{
    for (int32 Index = 0; Index < EnemySlots.Num(); ++Index)
    {
        if (EnemySlots[Index].InteractiveActor.Get() == OwnerActor)
        {
            DefeatSlot(Index);
            return;
        }
    }
}

void ASingijeonEnemyWaveActor::FindAndBindHwacha()
{
    if (!GetWorld())
    {
        return;
    }
    ASingijeonHwachaActor* Hwacha = Cast<ASingijeonHwachaActor>(TargetActor);
    if (!Hwacha && bAutoFindHwacha)
    {
        for (TActorIterator<ASingijeonHwachaActor> It(GetWorld()); It; ++It)
        {
            Hwacha = *It;
            break;
        }
    }
    if (!Hwacha)
    {
        return;
    }
    BoundHwacha = Hwacha;
    Hwacha->OnLoadCountChanged.AddUniqueDynamic(this, &ThisClass::HandleHwachaLoadCountChanged);
    Hwacha->OnVolleyLaunched.AddUniqueDynamic(this, &ThisClass::HandleHwachaVolleyLaunched);
    Hwacha->OnAimCompleted.AddUniqueDynamic(this, &ThisClass::HandleHwachaAimCompleted);
    Hwacha->OnHwachaStateChanged.AddUniqueDynamic(this, &ThisClass::HandleHwachaStateChanged);
    RefreshProcedureStateFromHwacha();
}

void ASingijeonEnemyWaveActor::UnbindHwacha()
{
    if (BoundHwacha)
    {
        BoundHwacha->OnLoadCountChanged.RemoveDynamic(this, &ThisClass::HandleHwachaLoadCountChanged);
        BoundHwacha->OnVolleyLaunched.RemoveDynamic(this, &ThisClass::HandleHwachaVolleyLaunched);
        BoundHwacha->OnAimCompleted.RemoveDynamic(this, &ThisClass::HandleHwachaAimCompleted);
        BoundHwacha->OnHwachaStateChanged.RemoveDynamic(this, &ThisClass::HandleHwachaStateChanged);
    }
    BoundHwacha = nullptr;
}

void ASingijeonEnemyWaveActor::RefreshProcedureStateFromHwacha()
{
    if (!bLimitApproachByHwachaProcedure)
    {
        SetProcedureApproachPhase(ESingijeonEnemyApproachPhase::Unrestricted);
        return;
    }
    if (!BoundHwacha)
    {
        SetProcedureApproachPhase(ESingijeonEnemyApproachPhase::Waiting);
        return;
    }
    if (BoundHwacha->GetHwachaState() == ESingijeonHwachaState::Fired)
    {
        SetProcedureApproachPhase(ESingijeonEnemyApproachPhase::Firing);
    }
    else if (BoundHwacha->GetHwachaState() == ESingijeonHwachaState::Igniting)
    {
        SetProcedureApproachPhase(ESingijeonEnemyApproachPhase::Igniting);
    }
    else if (BoundHwacha->IsAimInteractionComplete())
    {
        SetProcedureApproachPhase(ESingijeonEnemyApproachPhase::Aimed);
    }
    else if (BoundHwacha->GetLoadedAmmunitionCount() > 0)
    {
        SetProcedureApproachPhase(ESingijeonEnemyApproachPhase::Loaded);
        if (bStartWhenHwachaLoaded && WaveState == ESingijeonEnemyWaveState::Ready)
        {
            StartWave();
        }
    }
    else
    {
        SetProcedureApproachPhase(ESingijeonEnemyApproachPhase::Waiting);
    }
}

void ASingijeonEnemyWaveActor::BuildFormationSlots()
{
    EnemySlots.Reset();
    EnemySlots.Reserve(EnemyCount);
    const int32 SafePlatoonCount = FMath::Max(1, PlatoonCount);
    const int32 PlatoonSize = FMath::CeilToInt(static_cast<float>(EnemyCount) / SafePlatoonCount);
    const int32 Columns = FMath::Max(1, FormationColumnsPerPlatoon);

    FRandomStream Random(FormationRandomSeed);
    TArray<float> PlatoonLateralOffsets;
    PlatoonLateralOffsets.SetNum(SafePlatoonCount);
    for (float& PlatoonOffset : PlatoonLateralOffsets)
    {
        PlatoonOffset = Random.FRandRange(-LateralSpacing * 0.65f, LateralSpacing * 0.65f);
    }

    const float SafeLateralJitter = FMath::Min(LateralJitter, LateralSpacing * 0.42f);
    const float SafeLongitudinalJitter = FMath::Min(LongitudinalJitter, RowSpacing * 0.42f);
    for (int32 Index = 0; Index < EnemyCount; ++Index)
    {
        FEnemySlot& Slot = EnemySlots.AddDefaulted_GetRef();
        Slot.Platoon = FMath::Min(Index / PlatoonSize, SafePlatoonCount - 1);
        const int32 InPlatoonIndex = Index - Slot.Platoon * PlatoonSize;
        const int32 Row = InPlatoonIndex / Columns;
        const int32 Column = InPlatoonIndex % Columns;
        const float CenteredColumn = static_cast<float>(Column) - static_cast<float>(Columns - 1) * 0.5f;
        Slot.LongitudinalOffset = Slot.Platoon * PlatoonSpacing + Row * RowSpacing +
            Random.FRandRange(-SafeLongitudinalJitter, SafeLongitudinalJitter);
        Slot.LateralOffset = CenteredColumn * LateralSpacing + PlatoonLateralOffsets[Slot.Platoon] +
            Random.FRandRange(-SafeLateralJitter, SafeLateralJitter);
        Slot.SpeedScale = Random.FRandRange(MinRunAnimationRate, MaxRunAnimationRate);
        Slot.YawOffset = Random.FRandRange(-YawJitterDegrees, YawJitterDegrees);
        Slot.UniformScale = Random.FRandRange(1.0f - ScaleVariation, 1.0f + ScaleVariation);
        Slot.PanicLateralScale = Random.FRandRange(0.55f, 1.0f) * (Random.RandBool() ? 1.0f : -1.0f);
        Slot.PanicRetreatScale = Random.FRandRange(0.65f, 1.15f);
        Slot.PanicPhase = Random.FRandRange(0.0f, 2.0f * PI);
        Slot.AnimationIndex = Random.RandRange(0, 11);
        Slot.bAlive = true;
    }
}

bool ASingijeonEnemyWaveActor::SpawnVisualRepresentations()
{
    if (!GetWorld())
    {
        return false;
    }
    ProxyInstances->ClearInstances();
    ProxyInstances->SetStaticMesh(nullptr);
    ProxyInstances->SetVisibility(false, true);
    ProxyInstances->SetHiddenInGame(true, true);

    CharacterInstances->ClearInstances();
    CharacterInstances->SetSkinnedAssetAndUpdate(ProxySkeletalMesh);
    CharacterInstances->SetTransformProvider(ProxyAnimationProvider);
    CharacterInstances->SetCullDistances(ProxyStartCullDistance, ProxyEndCullDistance);
    CharacterInstances->SetAnimationMinScreenSize(ProxyAnimationMinScreenSize);
    CharacterInstances->OverrideMinLOD(ProxyMinLOD);
    CharacterInstances->SetVisibility(false, true);
    CharacterInstances->SetHiddenInGame(true, true);

    const int32 InteractiveCount = ForegroundEnemyClass
        ? FMath::Min(MaxInteractiveEnemies, EnemySlots.Num())
        : 0;
    for (int32 Index = 0; Index < EnemySlots.Num(); ++Index)
    {
        FEnemySlot& Slot = EnemySlots[Index];
        if (Index < InteractiveCount)
        {
            FActorSpawnParameters Params;
            Params.Owner = this;
            Params.OverrideLevel = GetLevel();
            Params.Name = MakeUniqueObjectName(
                GetLevel(), ForegroundEnemyClass.Get(),
                *FString::Printf(TEXT("SingijeonEnemy_%02d"), Index + 1));
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            AEnemySoldierActor* Enemy = GetWorld()->SpawnActor<AEnemySoldierActor>(
                ForegroundEnemyClass, GetSlotTransform(Slot), Params);
            if (!Enemy)
            {
                return false;
            }
#if WITH_EDITOR
            Enemy->SetActorLabel(FString::Printf(TEXT("Singijeon Enemy %02d"), Index + 1));
#endif
            Slot.InteractiveActor = Enemy;
            if (ULegacyHealthComponent* Health = Enemy->GetHealthComponent())
            {
                Health->OnHealthDepleted.AddUniqueDynamic(this, &ThisClass::HandleInteractiveEnemyDepleted);
            }
            if (USkeletalMeshComponent* EnemyMesh = Enemy->GetMesh())
            {
                if (ProxySkeletalMesh)
                {
                    EnemyMesh->SetSkeletalMesh(ProxySkeletalMesh);
                }
                if (UAnimationAsset* RunAnimation = GetRunAnimation())
                {
                    EnemyMesh->PlayAnimation(RunAnimation, true);
                    EnemyMesh->SetPlayRate(Slot.SpeedScale);
                    const float Phase = static_cast<float>(Slot.AnimationIndex % 12) / 12.0f;
                    EnemyMesh->SetPosition(RunAnimation->GetPlayLength() * Phase, false);
                }
                // These foreground actors are visual/shot targets only; their
                // shadows are disproportionately expensive in standalone VR.
                EnemyMesh->SetCastShadow(false);
                EnemyMesh->bCastDynamicShadow = false;
                EnemyMesh->VisibilityBasedAnimTickOption =
                    EVisibilityBasedAnimTickOption::AlwaysTickPose;
                EnemyMesh->OverrideMinLOD(ProxyMinLOD);
				EnemyMesh->SetRelativeScale3D(
					ProxyScale * GetEnemyHeightNormalizationScale());
				EnemyMesh->SetBoundsScale(2.0f);
				EnemyMesh->bNeverDistanceCull = true;
            }
            Enemy->SetSoldierActive(false);
        }
        else if (ProxySkeletalMesh && bUseGpuInstancedCrowd && ProxyAnimationProvider)
        {
            // Store the stable dense index before building the transform so the
            // skeletal-pivot ground offset and proxy scale are selected.
            Slot.ProxyInstanceIndex = CharacterInstances->GetInstanceCount();
            CharacterInstances->AddInstance(GetSlotTransform(Slot), Slot.AnimationIndex, true);
        }
        else if (ProxySkeletalMesh)
        {
            Slot.ReliableProxyIndex = ReliableProxyMeshes.Num();
            if (!CreateReliableProxyMesh(Slot, Slot.ReliableProxyIndex))
            {
                return false;
            }
        }
    }
    return true;
}

void ASingijeonEnemyWaveActor::DestroyVisualRepresentations()
{
    for (FEnemySlot& Slot : EnemySlots)
    {
        if (AEnemySoldierActor* Enemy = Slot.InteractiveActor.Get())
        {
            if (ULegacyHealthComponent* Health = Enemy->GetHealthComponent())
            {
                Health->OnHealthDepleted.RemoveDynamic(this, &ThisClass::HandleInteractiveEnemyDepleted);
            }
            Enemy->Destroy();
        }
        Slot.InteractiveActor.Reset();
    }
    EnemySlots.Reset();
    if (ProxyInstances)
    {
        ProxyInstances->ClearInstances();
    }
    if (CharacterInstances)
    {
        CharacterInstances->ClearInstances();
    }
    // Components from older hot-reload instances were owned directly by the
    // Wave. New Quest-safe proxies are owned by their SkeletalMeshActor and
    // are destroyed with that Actor below.
    for (USkeletalMeshComponent* ProxyMeshComponent : ReliableProxyMeshes)
    {
        if (IsValid(ProxyMeshComponent) && ProxyMeshComponent->GetOwner() == this)
        {
            RemoveInstanceComponent(ProxyMeshComponent);
            ProxyMeshComponent->DestroyComponent();
        }
    }
    for (AEnemySoldierActor* ProxyActor : ReliableProxyActors)
    {
        if (IsValid(ProxyActor))
        {
            ProxyActor->Destroy();
        }
    }
    ReliableProxyMeshes.Reset();
    ReliableProxyActors.Reset();
    SharedPoseLeaders.Reset();
    AliveEnemyCount = 0;
}

void ASingijeonEnemyWaveActor::UpdateInteractiveEnemies()
{
    for (const FEnemySlot& Slot : EnemySlots)
    {
        if (AEnemySoldierActor* Enemy = Slot.InteractiveActor.Get())
        {
            if (Slot.bAlive)
            {
                Enemy->SetActorTransform(GetSlotTransform(Slot), false, nullptr, ETeleportType::TeleportPhysics);
            }
        }
    }
}

USkeletalMeshComponent* ASingijeonEnemyWaveActor::CreateReliableProxyMesh(
    const FEnemySlot& Slot,
    const int32 ProxyOrdinal)
{
    if (!ProxySkeletalMesh || !GetWorld())
    {
        return nullptr;
    }

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.Owner = this;
    SpawnParameters.OverrideLevel = GetLevel();
    const int32 SlotIndex = FMath::Clamp(MaxInteractiveEnemies, 0, EnemyCount) + ProxyOrdinal;
    SpawnParameters.Name = MakeUniqueObjectName(
        GetLevel(), AEnemySoldierActor::StaticClass(),
        *FString::Printf(TEXT("SingijeonEnemy_%02d"), SlotIndex + 1));
    SpawnParameters.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnParameters.ObjectFlags |= RF_Transient;
    // SkeletalMeshActor proxies can disappear from the OpenXR frame after the
    // editor-only formation preview is destroyed.  Use the same lightweight
    // EnemySoldier actor path as the three foreground targets instead.
    AEnemySoldierActor* ProxyActor = GetWorld()->SpawnActor<AEnemySoldierActor>(
        AEnemySoldierActor::StaticClass(), GetSlotTransform(Slot), SpawnParameters);
    if (!ProxyActor)
    {
        return nullptr;
    }
#if WITH_EDITOR
    ProxyActor->SetActorLabel(FString::Printf(TEXT("Singijeon Enemy %02d"), SlotIndex + 1));
#endif
    ProxyActor->SetActorTickEnabled(false);
    if (UCharacterMovementComponent* Movement = ProxyActor->GetCharacterMovement())
    {
        Movement->SetComponentTickEnabled(false);
    }
    USkeletalMeshComponent* Proxy = ProxyActor->GetMesh();
    if (!Proxy)
    {
        ProxyActor->Destroy();
        return nullptr;
    }
    Proxy->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Proxy->SetGenerateOverlapEvents(false);
    Proxy->SetSkeletalMesh(ProxySkeletalMesh);
    Proxy->SetRelativeLocation(FVector::ZeroVector);
    Proxy->SetRelativeRotation(FRotator::ZeroRotator);
    Proxy->SetCanEverAffectNavigation(false);
    Proxy->SetCastShadow(false);
    Proxy->bCastDynamicShadow = false;
    Proxy->bCastStaticShadow = false;
    Proxy->bReceivesDecals = false;
	Proxy->SetRenderInMainPass(true);
	Proxy->SetRenderInDepthPass(true);
	Proxy->bNeverDistanceCull = true;
	Proxy->SetBoundsScale(2.0f);
    Proxy->bEnableUpdateRateOptimizations = true;
    Proxy->OverrideMinLOD(ProxyMinLOD);
    Proxy->SetComponentTickInterval(1.0f / 30.0f);
    Proxy->SetVisibility(true, true);
    Proxy->SetHiddenInGame(false, true);
    ProxyActor->SetSoldierActive(true);
    ProxyActor->SetActorEnableCollision(false);

    const int32 ProxyCount = FMath::Max(1, EnemyCount - MaxInteractiveEnemies);
    const int32 LeaderCount = FMath::Clamp(SharedPoseLeaderCount, 1, ProxyCount);
    if (ProxyOrdinal < LeaderCount)
    {
        Proxy->VisibilityBasedAnimTickOption =
            EVisibilityBasedAnimTickOption::AlwaysTickPose;
        if (UAnimationAsset* RunAnimation = GetRunAnimation())
        {
            Proxy->PlayAnimation(RunAnimation, true);
            Proxy->SetPlayRate(Slot.SpeedScale);
            const float Phase = static_cast<float>(Slot.AnimationIndex % 12) / 12.0f;
            Proxy->SetPosition(RunAnimation->GetPlayLength() * Phase, false);
        }
        SharedPoseLeaders.Add(Proxy);
    }
    else if (!SharedPoseLeaders.IsEmpty())
    {
        Proxy->VisibilityBasedAnimTickOption =
            EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
        Proxy->SetLeaderPoseComponent(
            SharedPoseLeaders[ProxyOrdinal % SharedPoseLeaders.Num()]);
    }

	Proxy->UpdateBounds();
	Proxy->MarkRenderTransformDirty();
	Proxy->MarkRenderStateDirty();

    ReliableProxyActors.Add(ProxyActor);
    ReliableProxyMeshes.Add(Proxy);
    return Proxy;
}

void ASingijeonEnemyWaveActor::UpdateProxyEnemies(const bool /*bMarkRenderStateDirty*/)
{
    if (!ProxySkeletalMesh)
    {
        return;
    }
    for (const FEnemySlot& Slot : EnemySlots)
    {
        if (Slot.ProxyInstanceIndex != INDEX_NONE && CharacterInstances &&
            Slot.ProxyInstanceIndex < CharacterInstances->GetInstanceCount())
        {
            FTransform Transform = GetSlotTransform(Slot);
            if (!Slot.bAlive)
            {
                Transform.SetScale3D(FVector::ZeroVector);
            }
            CharacterInstances->SetInstanceTransform(
                CharacterInstances->GetInstanceId(Slot.ProxyInstanceIndex), Transform, true);
        }
        else if (ReliableProxyMeshes.IsValidIndex(Slot.ReliableProxyIndex))
        {
            USkeletalMeshComponent* Proxy = ReliableProxyMeshes[Slot.ReliableProxyIndex];
            if (IsValid(Proxy))
            {
                if (ReliableProxyActors.IsValidIndex(Slot.ReliableProxyIndex) &&
                    IsValid(ReliableProxyActors[Slot.ReliableProxyIndex]))
                {
                    ReliableProxyActors[Slot.ReliableProxyIndex]->SetActorTransform(
                        GetSlotTransform(Slot), false, nullptr, ETeleportType::TeleportPhysics);
                }
                else
                {
                    Proxy->SetWorldTransform(GetSlotTransform(Slot), false, nullptr,
                        ETeleportType::TeleportPhysics);
                }
                Proxy->SetVisibility(Slot.bAlive, true);
                Proxy->SetHiddenInGame(!Slot.bAlive, true);
                Proxy->SetComponentTickEnabled(Slot.bAlive);
            }
        }
    }
}

FTransform ASingijeonEnemyWaveActor::GetSlotTransform(const FEnemySlot& Slot) const
{
    FVector Location;
    FVector Direction;
    SampleRoute(GetSlotRouteDistance(Slot), Location, Direction);
    const FVector Right = FVector::CrossProduct(FVector::UpVector, Direction).GetSafeNormal();
    Location += Right * Slot.LateralOffset;
    if (WaveState == ESingijeonEnemyWaveState::Panicking && PanicDuration > KINDA_SMALL_NUMBER)
    {
        const float PanicAlpha = FMath::Clamp(PanicElapsed / PanicDuration, 0.0f, 1.0f);
        const float Ease = FMath::InterpEaseOut(0.0f, 1.0f, PanicAlpha, 2.0f);
        const float Weave = FMath::Sin(PanicElapsed * 5.0f + Slot.PanicPhase) *
            PanicLateralDistance * 0.12f * (1.0f - PanicAlpha);
        Location += Right * (Slot.PanicLateralScale * PanicLateralDistance * Ease + Weave);
        Location -= Direction * Slot.PanicRetreatScale * PanicRetreatDistance * Ease;
    }
    const bool bVisualProxy = Slot.ProxyInstanceIndex != INDEX_NONE ||
        Slot.ReliableProxyIndex != INDEX_NONE;
    Location.Z += bVisualProxy ? ProxyGroundOffset : AgentGroundOffset;
    FRotator Rotation = Direction.Rotation();
    Rotation.Yaw += Slot.YawOffset;
    if (WaveState == ESingijeonEnemyWaveState::Panicking)
    {
        Rotation.Yaw += Slot.PanicLateralScale * 70.0f +
            FMath::Sin(PanicElapsed * 6.0f + Slot.PanicPhase) * 22.0f;
    }
    const FVector Scale = bVisualProxy
		? ProxyScale * GetEnemyHeightNormalizationScale() * Slot.UniformScale
        : FVector(Slot.UniformScale);
    return FTransform(Rotation, Location, Scale);
}

UAnimationAsset* ASingijeonEnemyWaveActor::GetRunAnimation() const
{
    return ForegroundRunAnimation
        ? ForegroundRunAnimation.Get()
        : RuntimeRunAnimationFallback.Get();
}

void ASingijeonEnemyWaveActor::SetPanicAnimationRates(const bool bPanic)
{
    const float Multiplier = bPanic ? PanicAnimationRateMultiplier : 1.0f;
    for (const FEnemySlot& Slot : EnemySlots)
    {
        if (AEnemySoldierActor* Enemy = Slot.InteractiveActor.Get())
        {
            if (USkeletalMeshComponent* Mesh = Enemy->GetMesh())
            {
                Mesh->SetPlayRate(Slot.SpeedScale * Multiplier);
            }
        }
    }
    for (USkeletalMeshComponent* Leader : SharedPoseLeaders)
    {
        if (IsValid(Leader))
        {
            Leader->SetPlayRate(Multiplier);
        }
    }
}

float ASingijeonEnemyWaveActor::GetEnemyHeightNormalizationScale() const
{
	if (!ProxySkeletalMesh)
	{
		return 1.0f;
	}
	const float ImportedHeight = ProxySkeletalMesh->GetBounds().BoxExtent.Z * 2.0f;
	return ImportedHeight > KINDA_SMALL_NUMBER
		? DesiredEnemyHeight / ImportedHeight
		: 1.0f;
}

float ASingijeonEnemyWaveActor::GetSlotRouteDistance(const FEnemySlot& Slot) const
{
    const float UnrestrictedDistance = WaveDistance * Slot.SpeedScale - Slot.LongitudinalOffset;
    if (CurrentApproachLimitFraction >= 1.0f)
    {
        return UnrestrictedDistance;
    }

    // The front rank obeys the exact procedure gate while rear rows retain
    // their authored longitudinal spacing instead of stacking into one line.
    const float FormationPreservingLimit =
        RouteLength * CurrentApproachLimitFraction - FMath::Max(0.0f, Slot.LongitudinalOffset);
    return FMath::Min(UnrestrictedDistance, FormationPreservingLimit);
}

void ASingijeonEnemyWaveActor::SampleRoute(
    const float Distance,
    FVector& OutLocation,
    FVector& OutDirection) const
{
    if (RoutePoints.Num() < 2)
    {
        OutLocation = GetActorLocation();
        OutDirection = GetActorForwardVector();
        return;
    }

    if (Distance <= 0.0f)
    {
        OutDirection = (RoutePoints[1] - RoutePoints[0]).GetSafeNormal();
        OutLocation = RoutePoints[0] + OutDirection * Distance;
        return;
    }
    if (Distance >= RouteLength)
    {
        const int32 Last = RoutePoints.Num() - 1;
        OutDirection = (RoutePoints[Last] - RoutePoints[Last - 1]).GetSafeNormal();
        OutLocation = RoutePoints[Last];
        return;
    }

    int32 Segment = 1;
    while (Segment < RouteCumulativeDistances.Num() &&
        RouteCumulativeDistances[Segment] < Distance)
    {
        ++Segment;
    }
    Segment = FMath::Clamp(Segment, 1, RoutePoints.Num() - 1);
    const float SegmentStartDistance = RouteCumulativeDistances[Segment - 1];
    const float SegmentLength = RouteCumulativeDistances[Segment] - SegmentStartDistance;
    const float Alpha = SegmentLength > KINDA_SMALL_NUMBER
        ? (Distance - SegmentStartDistance) / SegmentLength
        : 0.0f;
    OutLocation = FMath::Lerp(RoutePoints[Segment - 1], RoutePoints[Segment], Alpha);
    OutDirection = (RoutePoints[Segment] - RoutePoints[Segment - 1]).GetSafeNormal();
}

bool ASingijeonEnemyWaveActor::DefeatSlot(const int32 SlotIndex)
{
    if (!EnemySlots.IsValidIndex(SlotIndex) || !EnemySlots[SlotIndex].bAlive)
    {
        return false;
    }
    FEnemySlot& Slot = EnemySlots[SlotIndex];
    Slot.bAlive = false;
    AliveEnemyCount = FMath::Max(0, AliveEnemyCount - 1);
    if (AEnemySoldierActor* Enemy = Slot.InteractiveActor.Get())
    {
        Enemy->SetSoldierActive(false);
        if (USkeletalMeshComponent* EnemyMesh = Enemy->GetMesh())
        {
            EnemyMesh->SetComponentTickEnabled(false);
        }
    }

    if (AliveEnemyCount == 0)
    {
        SetWaveState(ESingijeonEnemyWaveState::Defeated);
        SetActorTickEnabled(false);
        OnWaveDefeated.Broadcast();
    }
    return true;
}

void ASingijeonEnemyWaveActor::SetWaveState(const ESingijeonEnemyWaveState NewState)
{
    if (WaveState == NewState)
    {
        if (NewState == ESingijeonEnemyWaveState::Charging)
        {
            StartMarchAudio();
        }
        return;
    }
    const ESingijeonEnemyWaveState OldState = WaveState;
    WaveState = NewState;
    if (NewState == ESingijeonEnemyWaveState::Charging)
    {
        StartMarchAudio();
    }
    else
    {
        StopMarchAudio();
    }
    OnWaveStateChanged.Broadcast(OldState, NewState);
}

bool ASingijeonEnemyWaveActor::IsPrimaryMarchAudioWave() const
{
    if (!GetWorld())
    {
        return false;
    }

    const ASingijeonEnemyWaveActor* PrimaryWave = nullptr;
    for (TActorIterator<ASingijeonEnemyWaveActor> It(GetWorld()); It; ++It)
    {
        if (!IsValid(*It) || It->IsTemplate())
        {
            continue;
        }
        if (!PrimaryWave || It->GetPathName() < PrimaryWave->GetPathName())
        {
            PrimaryWave = *It;
        }
    }
    return PrimaryWave == this;
}

void ASingijeonEnemyWaveActor::StartMarchAudio()
{
    if (!MarchAudioComponent || !MarchSound || !IsPrimaryMarchAudioWave())
    {
        bMarchAudioRequested = false;
        return;
    }

    bMarchAudioRequested = true;
    MarchAudioComponent->SetSound(MarchSound);
    MarchAudioComponent->SetVolumeMultiplier(MarchSoundVolume);
    if (!MarchAudioComponent->IsPlaying())
    {
        MarchAudioComponent->Play();
    }
}

void ASingijeonEnemyWaveActor::StopMarchAudio()
{
    bMarchAudioRequested = false;
    if (MarchAudioComponent && MarchAudioComponent->IsPlaying())
    {
        MarchAudioComponent->Stop();
    }
}

void ASingijeonEnemyWaveActor::HandleMarchAudioFinished()
{
    if (bMarchAudioRequested && WaveState == ESingijeonEnemyWaveState::Charging &&
        MarchAudioComponent && MarchSound && IsPrimaryMarchAudioWave())
    {
        MarchAudioComponent->Play();
    }
}

void ASingijeonEnemyWaveActor::SetVisualsActive(const bool bActive)
{
	if (bActive)
	{
		SetActorHiddenInGame(false);
		if (SceneRoot)
		{
			SceneRoot->SetVisibility(true, false);
			SceneRoot->SetHiddenInGame(false, false);
		}
	}
    if (ProxyInstances)
    {
        ProxyInstances->SetVisibility(false, true);
        ProxyInstances->SetHiddenInGame(true, true);
    }
    if (CharacterInstances)
    {
        const bool bGpuVisible = bActive && bUseGpuInstancedCrowd;
        CharacterInstances->SetVisibility(bGpuVisible, true);
        CharacterInstances->SetHiddenInGame(!bGpuVisible, true);
        CharacterInstances->SetComponentTickEnabled(bGpuVisible);
    }
    for (FEnemySlot& Slot : EnemySlots)
    {
        if (AEnemySoldierActor* Enemy = Slot.InteractiveActor.Get())
        {
            const bool bEnemyVisible = bActive && Slot.bAlive;
            Enemy->SetSoldierActive(bEnemyVisible);
            if (USkeletalMeshComponent* EnemyMesh = Enemy->GetMesh())
            {
                EnemyMesh->SetComponentTickEnabled(bEnemyVisible);
            }
        }
        if (ReliableProxyMeshes.IsValidIndex(Slot.ReliableProxyIndex))
        {
            if (USkeletalMeshComponent* Proxy = ReliableProxyMeshes[Slot.ReliableProxyIndex])
            {
                const bool bProxyVisible = bActive && Slot.bAlive;
                if (ReliableProxyActors.IsValidIndex(Slot.ReliableProxyIndex) &&
                    IsValid(ReliableProxyActors[Slot.ReliableProxyIndex]))
                {
                    ReliableProxyActors[Slot.ReliableProxyIndex]->SetActorHiddenInGame(
                        !bProxyVisible);
                }
                Proxy->SetVisibility(bProxyVisible, true);
                Proxy->SetHiddenInGame(!bProxyVisible, true);
                Proxy->SetComponentTickEnabled(bProxyVisible);
				if (bProxyVisible)
				{
					Proxy->UpdateBounds();
					Proxy->MarkRenderTransformDirty();
					Proxy->MarkRenderStateDirty();
				}
            }
        }
    }
}
