#include "Enemy/SingijeonEnemyWaveActor.h"

#include "Animation/AnimationAsset.h"
#include "Components/BoxComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Shared/Characters/EnemySoldierActor.h"
#include "Shared/Combat/HealthComponent.h"
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

    ForegroundEnemyClass = AEnemySoldierActor::StaticClass();

    static ConstructorHelpers::FObjectFinder<UStaticMesh> ProxyMeshFinder(
        TEXT("/Game/NiagaraExamples/Gallery/StaticMesh/SM_MannequinTarget.SM_MannequinTarget"));
    if (ProxyMeshFinder.Succeeded())
    {
        ProxyMesh = ProxyMeshFinder.Object;
        ProxyInstances->SetStaticMesh(ProxyMesh);
    }

    static ConstructorHelpers::FObjectFinder<UAnimationAsset> RunAnimationFinder(
        TEXT("/Game/NiagaraExamples/Gallery/SkeletalMesh/Mannequins/Anims/Rifle/Jog/MF_Rifle_Jog_Fwd.MF_Rifle_Jog_Fwd"));
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
    if (ProxyInstances)
    {
        ProxyInstances->SetStaticMesh(ProxyMesh);
        ProxyInstances->SetCullDistances(ProxyStartCullDistance, ProxyEndCullDistance);
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

    WaveDistance += FMath::Max(0.0f, DeltaSeconds) * ChargeSpeed;
    UpdateInteractiveEnemies();

    ProxyUpdateAccumulator += FMath::Max(0.0f, DeltaSeconds);
    if (ProxyUpdateAccumulator >= ProxyUpdateInterval)
    {
        ProxyUpdateAccumulator = FMath::Fmod(ProxyUpdateAccumulator, ProxyUpdateInterval);
        UpdateProxyEnemies(true);
    }

    for (const FEnemySlot& Slot : EnemySlots)
    {
        if (Slot.bAlive && WaveDistance * Slot.SpeedScale - Slot.LongitudinalOffset >= RouteLength)
        {
            SetWaveState(ESingijeonEnemyWaveState::ReachedTarget);
            SetActorTickEnabled(false);
            OnWaveReachedTarget.Broadcast();
            return;
        }
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
    return ProxyInstances ? ProxyInstances->GetInstanceCount() : 0;
}

void ASingijeonEnemyWaveActor::HandleHwachaLoadCountChanged(const int32 LoadedCount, const int32)
{
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
}

void ASingijeonEnemyWaveActor::UnbindHwacha()
{
    if (BoundHwacha)
    {
        BoundHwacha->OnLoadCountChanged.RemoveDynamic(this, &ThisClass::HandleHwachaLoadCountChanged);
        BoundHwacha->OnVolleyLaunched.RemoveDynamic(this, &ThisClass::HandleHwachaVolleyLaunched);
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

    FRandomStream Random(741953);
    for (int32 Index = 0; Index < EnemyCount; ++Index)
    {
        FEnemySlot& Slot = EnemySlots.AddDefaulted_GetRef();
        Slot.Platoon = FMath::Min(Index / PlatoonSize, SafePlatoonCount - 1);
        const int32 InPlatoonIndex = Index - Slot.Platoon * PlatoonSize;
        const int32 Row = InPlatoonIndex / Columns;
        const int32 Column = InPlatoonIndex % Columns;
        const float CenteredColumn = static_cast<float>(Column) - static_cast<float>(Columns - 1) * 0.5f;
        Slot.LongitudinalOffset = Slot.Platoon * PlatoonSpacing + Row * RowSpacing + Random.FRandRange(-20.0f, 20.0f);
        Slot.LateralOffset = CenteredColumn * LateralSpacing + Random.FRandRange(-30.0f, 30.0f);
        Slot.SpeedScale = Random.FRandRange(0.97f, 1.03f);
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
    ProxyInstances->SetStaticMesh(ProxyMesh);
    ProxyInstances->SetCullDistances(ProxyStartCullDistance, ProxyEndCullDistance);

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
            if (UHealthComponent* Health = Enemy->GetHealthComponent())
            {
                Health->OnHealthDepleted.AddUniqueDynamic(this, &ThisClass::HandleInteractiveEnemyDepleted);
            }
            if (ForegroundRunAnimation && Enemy->GetMesh())
            {
                Enemy->GetMesh()->PlayAnimation(ForegroundRunAnimation, true);
            }
            Enemy->SetSoldierActive(false);
        }
        else if (ProxyMesh)
        {
            Slot.ProxyInstanceIndex = ProxyInstances->AddInstance(GetSlotTransform(Slot), true);
            ProxyInstances->SetCustomDataValue(Slot.ProxyInstanceIndex, 0, static_cast<float>(Index % 8) / 8.0f, false);
            ProxyInstances->SetCustomDataValue(Slot.ProxyInstanceIndex, 1, Slot.SpeedScale, false);
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
            if (UHealthComponent* Health = Enemy->GetHealthComponent())
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

void ASingijeonEnemyWaveActor::UpdateProxyEnemies(const bool bMarkRenderStateDirty)
{
    if (!ProxyInstances || !ProxyMesh || ProxyInstances->GetInstanceCount() == 0)
    {
        return;
    }
    TArray<FTransform> Transforms;
    Transforms.SetNum(ProxyInstances->GetInstanceCount());
    for (const FEnemySlot& Slot : EnemySlots)
    {
        if (Slot.ProxyInstanceIndex == INDEX_NONE || !Transforms.IsValidIndex(Slot.ProxyInstanceIndex))
        {
            continue;
        }
        FTransform Transform = GetSlotTransform(Slot);
        if (!Slot.bAlive)
        {
            Transform.SetScale3D(FVector::ZeroVector);
        }
        Transforms[Slot.ProxyInstanceIndex] = Transform;
    }
    ProxyInstances->BatchUpdateInstancesTransforms(
        0, Transforms, true, bMarkRenderStateDirty, true);
}

FTransform ASingijeonEnemyWaveActor::GetSlotTransform(const FEnemySlot& Slot) const
{
    FVector Location;
    FVector Direction;
    SampleRoute(WaveDistance * Slot.SpeedScale - Slot.LongitudinalOffset, Location, Direction);
    const FVector Right = FVector::CrossProduct(FVector::UpVector, Direction).GetSafeNormal();
    Location += Right * Slot.LateralOffset;
    Location.Z += AgentGroundOffset;
    const FRotator Rotation = Direction.Rotation();
    const FVector Scale = Slot.ProxyInstanceIndex != INDEX_NONE ? ProxyScale : FVector::OneVector;
    return FTransform(Rotation, Location, Scale);
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
        ProxyInstances->SetVisibility(bActive, true);
        ProxyInstances->SetHiddenInGame(!bActive, true);
    }
    for (FEnemySlot& Slot : EnemySlots)
    {
        if (AEnemySoldierActor* Enemy = Slot.InteractiveActor.Get())
        {
            Enemy->SetSoldierActive(bActive && Slot.bAlive);
        }
    }
}
