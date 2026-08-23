#include "Enemy/SingijeonEnemyWaveActor.h"

#include "Animation/AnimationAsset.h"
#include "Animation/AnimSequenceTransformProviderData.h"
#include "Components/BoxComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/InstancedSkinnedMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Shared/Characters/EnemySoldierActor.h"
#include "Shared/Combat/LegacyHealthComponent.h"
#include "Singijeon/SingijeonHwachaActor.h"
#include "UObject/ConstructorHelpers.h"

ASingijeonEnemyWaveActor::ASingijeonEnemyWaveActor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

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

    ForegroundEnemyClass = AEnemySoldierActor::StaticClass();

    static ConstructorHelpers::FObjectFinder<USkeletalMesh> ProxySkeletalMeshFinder(
        TEXT("/GF_Singijeon/Gameplay/Enemy/Samurai/SKM_Low_Poly_Samurai_VR.SKM_Low_Poly_Samurai_VR"));
    if (ProxySkeletalMeshFinder.Succeeded())
    {
        ProxySkeletalMesh = ProxySkeletalMeshFinder.Object;
        CharacterInstances->SetSkinnedAssetAndUpdate(ProxySkeletalMesh);
    }

    static ConstructorHelpers::FObjectFinder<UAnimSequenceTransformProviderData> AnimationProviderFinder(
        TEXT("/GF_Singijeon/Gameplay/Enemy/Samurai/DA_SingijeonSamuraiRifleRun_GPU.DA_SingijeonSamuraiRifleRun_GPU"));
    if (AnimationProviderFinder.Succeeded())
    {
        ProxyAnimationProvider = AnimationProviderFinder.Object;
        CharacterInstances->SetTransformProvider(ProxyAnimationProvider);
    }

    static ConstructorHelpers::FObjectFinder<UAnimationAsset> RunAnimationFinder(
        TEXT("/GF_Singijeon/Gameplay/Enemy/Samurai/MF_Rifle_Jog_Fwd_Samurai.MF_Rifle_Jog_Fwd_Samurai"));
    if (RunAnimationFinder.Succeeded())
    {
        ForegroundRunAnimation = RunAnimationFinder.Object;
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
    MinRunAnimationRate = FMath::Clamp(MinRunAnimationRate, 0.1f, 3.0f);
    MaxRunAnimationRate = FMath::Clamp(MaxRunAnimationRate, MinRunAnimationRate, 3.0f);
    LoadedApproachLimit = FMath::Clamp(LoadedApproachLimit, 0.0f, 1.0f);
    AimedApproachLimit = FMath::Clamp(AimedApproachLimit, LoadedApproachLimit, 1.0f);
    IgnitingApproachLimit = FMath::Clamp(IgnitingApproachLimit, AimedApproachLimit, 1.0f);
    FiringApproachLimit = FMath::Clamp(FiringApproachLimit, IgnitingApproachLimit, 1.0f);
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
}

void ASingijeonEnemyWaveActor::BeginPlay()
{
    Super::BeginPlay();
    FindAndBindHwacha();
    PrepareWave();
    if (bStartOnBeginPlay)
    {
        StartWave();
    }
}

void ASingijeonEnemyWaveActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UnbindHwacha();
    DestroyVisualRepresentations();
    Super::EndPlay(EndPlayReason);
}

void ASingijeonEnemyWaveActor::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
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
    const FVector End = IsValid(TargetActor)
        ? TargetActor->GetActorLocation()
        : (DefaultTargetPoint ? DefaultTargetPoint->GetComponentLocation() : Start + GetActorForwardVector() * 10000.0f);

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
    AliveEnemyCount = EnemySlots.Num();
    UpdateInteractiveEnemies();
    UpdateProxyEnemies(true);
    SetVisualsActive(false);
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
    SetVisualsActive(false);
    SetWaveState(ESingijeonEnemyWaveState::Ready);
    CurrentApproachLimitFraction = 1.0f;
    ApproachPhase = ESingijeonEnemyApproachPhase::Unrestricted;
    if (BoundHwacha && bLimitApproachByHwachaProcedure)
    {
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
        }
        else
        {
            SetProcedureApproachPhase(ESingijeonEnemyApproachPhase::Waiting);
        }
    }
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
    return CharacterInstances ? CharacterInstances->GetInstanceCount() : 0;
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
    if (!BoundHwacha || WaveState != ESingijeonEnemyWaveState::Charging)
    {
        return;
    }
    ResolveVolley(BoundHwacha->GetActorLocation(), BoundHwacha->GetActorForwardVector());
    if (AliveEnemyCount > 0)
    {
        SetProcedureApproachPhase(ESingijeonEnemyApproachPhase::Unrestricted);
    }
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
    TargetActor = Hwacha;
    Hwacha->OnLoadCountChanged.AddUniqueDynamic(this, &ThisClass::HandleHwachaLoadCountChanged);
    Hwacha->OnVolleyLaunched.AddUniqueDynamic(this, &ThisClass::HandleHwachaVolleyLaunched);
    Hwacha->OnAimCompleted.AddUniqueDynamic(this, &ThisClass::HandleHwachaAimCompleted);
    Hwacha->OnHwachaStateChanged.AddUniqueDynamic(this, &ThisClass::HandleHwachaStateChanged);
    if (bLimitApproachByHwachaProcedure)
    {
        SetProcedureApproachPhase(ESingijeonEnemyApproachPhase::Waiting);
    }
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
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            AEnemySoldierActor* Enemy = GetWorld()->SpawnActor<AEnemySoldierActor>(
                ForegroundEnemyClass, GetSlotTransform(Slot), Params);
            if (!Enemy)
            {
                return false;
            }
            Slot.InteractiveActor = Enemy;
            if (ULegacyHealthComponent* Health = Enemy->GetHealthComponent())
            {
                Health->OnHealthDepleted.AddUniqueDynamic(this, &ThisClass::HandleInteractiveEnemyDepleted);
            }
            if (ForegroundRunAnimation && Enemy->GetMesh())
            {
                Enemy->GetMesh()->SetSkeletalMesh(ProxySkeletalMesh);
                Enemy->GetMesh()->PlayAnimation(ForegroundRunAnimation, true);
                Enemy->GetMesh()->SetPlayRate(Slot.SpeedScale);
            }
            if (USkeletalMeshComponent* EnemyMesh = Enemy->GetMesh())
            {
                // These foreground actors are visual/shot targets only; their
                // shadows are disproportionately expensive in standalone VR.
                EnemyMesh->SetCastShadow(false);
                EnemyMesh->bCastDynamicShadow = false;
                EnemyMesh->VisibilityBasedAnimTickOption =
                    EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
                EnemyMesh->OverrideMinLOD(ProxyMinLOD);
            }
            Enemy->SetSoldierActive(false);
        }
        else if (ProxySkeletalMesh)
        {
            // Store the stable dense index before building the transform so the
            // skeletal-pivot ground offset and proxy scale are selected.
            Slot.ProxyInstanceIndex = CharacterInstances->GetInstanceCount();
            CharacterInstances->AddInstance(GetSlotTransform(Slot), Slot.AnimationIndex, true);
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

void ASingijeonEnemyWaveActor::UpdateProxyEnemies(const bool /*bMarkRenderStateDirty*/)
{
    if (!CharacterInstances || !ProxySkeletalMesh || CharacterInstances->GetInstanceCount() == 0)
    {
        return;
    }
    for (const FEnemySlot& Slot : EnemySlots)
    {
        if (Slot.ProxyInstanceIndex == INDEX_NONE ||
            Slot.ProxyInstanceIndex >= CharacterInstances->GetInstanceCount())
        {
            continue;
        }
        FTransform Transform = GetSlotTransform(Slot);
        if (!Slot.bAlive)
        {
            Transform.SetScale3D(FVector::ZeroVector);
        }
        CharacterInstances->SetInstanceTransform(
            CharacterInstances->GetInstanceId(Slot.ProxyInstanceIndex), Transform, true);
    }
}

FTransform ASingijeonEnemyWaveActor::GetSlotTransform(const FEnemySlot& Slot) const
{
    FVector Location;
    FVector Direction;
    SampleRoute(GetSlotRouteDistance(Slot), Location, Direction);
    const FVector Right = FVector::CrossProduct(FVector::UpVector, Direction).GetSafeNormal();
    Location += Right * Slot.LateralOffset;
    const bool bGpuCharacter = Slot.ProxyInstanceIndex != INDEX_NONE;
    Location.Z += bGpuCharacter ? ProxyGroundOffset : AgentGroundOffset;
    FRotator Rotation = Direction.Rotation();
    Rotation.Yaw += Slot.YawOffset;
    const FVector Scale = bGpuCharacter
        ? ProxyScale * Slot.UniformScale
        : FVector(Slot.UniformScale);
    return FTransform(Rotation, Location, Scale);
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
        return;
    }
    const ESingijeonEnemyWaveState OldState = WaveState;
    WaveState = NewState;
    OnWaveStateChanged.Broadcast(OldState, NewState);
}

void ASingijeonEnemyWaveActor::SetVisualsActive(const bool bActive)
{
    if (ProxyInstances)
    {
        ProxyInstances->SetVisibility(false, true);
        ProxyInstances->SetHiddenInGame(true, true);
    }
    if (CharacterInstances)
    {
        CharacterInstances->SetVisibility(bActive, true);
        CharacterInstances->SetHiddenInGame(!bActive, true);
    }
    for (FEnemySlot& Slot : EnemySlots)
    {
        if (AEnemySoldierActor* Enemy = Slot.InteractiveActor.Get())
        {
            Enemy->SetSoldierActive(bActive && Slot.bAlive);
        }
    }
}
