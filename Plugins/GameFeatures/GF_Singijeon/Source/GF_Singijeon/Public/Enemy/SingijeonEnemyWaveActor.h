#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Singijeon/SingijeonHwachaActor.h"
#include "SingijeonEnemyWaveActor.generated.h"

class AEnemySoldierActor;
class UAnimationAsset;
class UAnimSequenceTransformProviderData;
class UBoxComponent;
class UHierarchicalInstancedStaticMeshComponent;
class UInstancedSkinnedMeshComponent;
class USceneComponent;
class USkeletalMesh;
class UStaticMesh;

UENUM(BlueprintType)
enum class ESingijeonEnemyWaveState : uint8
{
    Hidden,
    Ready,
    Charging,
    ReachedTarget,
    Defeated
};

UENUM(BlueprintType)
enum class ESingijeonEnemyApproachPhase : uint8
{
    Waiting,
    Loaded,
    Aimed,
    Igniting,
    Firing,
    Unrestricted
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnSingijeonEnemyWaveStateChanged,
    ESingijeonEnemyWaveState, OldState,
    ESingijeonEnemyWaveState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnSingijeonVolleyResolved,
    int32, Casualties,
    int32, RemainingEnemies);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSingijeonEnemyWaveEvent);

/**
 * Quest-oriented charge wave. One manager owns the route and movement for the
 * entire formation; no soldier receives an AIController, path following, or Tick.
 */
UCLASS(Blueprintable)
class GF_SINGIJEON_API ASingijeonEnemyWaveActor : public AActor
{
    GENERATED_BODY()

public:
    ASingijeonEnemyWaveActor();

    virtual void Tick(float DeltaSeconds) override;
    virtual void OnConstruction(const FTransform& Transform) override;

    /** Builds one shared Nav path. Falls back to a direct path when NavMesh is unavailable. */
    UFUNCTION(BlueprintCallable, Category = "Singijeon|Enemy Wave")
    bool RebuildRoute();

    /** Creates the logical formation, foreground soldiers, and background proxy instances. */
    UFUNCTION(BlueprintCallable, Category = "Singijeon|Enemy Wave")
    bool PrepareWave();

    UFUNCTION(BlueprintCallable, Category = "Singijeon|Enemy Wave")
    void StartWave();

    UFUNCTION(BlueprintCallable, Category = "Singijeon|Enemy Wave")
    void StopWave(bool bHideEnemies = false);

    UFUNCTION(BlueprintCallable, Category = "Singijeon|Enemy Wave")
    void ResetWave();

    /** Resolves the logical crowd from a Hwacha volley without per-proxy collision. */
    UFUNCTION(BlueprintCallable, Category = "Singijeon|Enemy Wave")
    int32 ResolveVolley(FVector Origin, FVector Direction);

    UFUNCTION(BlueprintPure, Category = "Singijeon|Enemy Wave")
    ESingijeonEnemyWaveState GetWaveState() const { return WaveState; }

    UFUNCTION(BlueprintPure, Category = "Singijeon|Enemy Wave")
    int32 GetAliveEnemyCount() const { return AliveEnemyCount; }

    UFUNCTION(BlueprintPure, Category = "Singijeon|Enemy Wave")
    int32 GetLogicalEnemyCount() const { return EnemySlots.Num(); }

    UFUNCTION(BlueprintPure, Category = "Singijeon|Enemy Wave")
    int32 GetInteractiveEnemyCount() const;

    UFUNCTION(BlueprintPure, Category = "Singijeon|Enemy Wave")
    int32 GetProxyEnemyCount() const;

    UFUNCTION(BlueprintPure, Category = "Singijeon|Enemy Wave")
    float GetRouteLength() const { return RouteLength; }

    UFUNCTION(BlueprintPure, Category = "Singijeon|Enemy Wave")
    float GetWaveDistance() const { return WaveDistance; }

    UFUNCTION(BlueprintPure, Category = "Singijeon|Enemy Wave")
    float GetCurrentApproachLimitFraction() const { return CurrentApproachLimitFraction; }

    UFUNCTION(BlueprintPure, Category = "Singijeon|Enemy Wave")
    ESingijeonEnemyApproachPhase GetApproachPhase() const { return ApproachPhase; }

    UFUNCTION(BlueprintCallable, Category = "Singijeon|Enemy Wave")
    void SetProcedureApproachPhase(ESingijeonEnemyApproachPhase NewPhase);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Enemy Wave")
    TObjectPtr<AActor> TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Enemy Wave")
    TSubclassOf<AEnemySoldierActor> ForegroundEnemyClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Enemy Wave", meta = (ClampMin = "1", ClampMax = "80"))
    int32 EnemyCount = 45;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Enemy Wave", meta = (ClampMin = "0", ClampMax = "16"))
    int32 MaxInteractiveEnemies = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Enemy Wave", meta = (ClampMin = "1", ClampMax = "8"))
    int32 PlatoonCount = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Enemy Wave", meta = (ClampMin = "1", ClampMax = "10"))
    int32 FormationColumnsPerPlatoon = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Enemy Wave", meta = (ClampMin = "50.0"))
    float LateralSpacing = 145.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Enemy Wave", meta = (ClampMin = "50.0"))
    float RowSpacing = 170.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Enemy Wave", meta = (ClampMin = "0.0"))
    float PlatoonSpacing = 550.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Enemy Wave", meta = (ClampMin = "1.0"))
    float ChargeSpeed = 220.0f;

    /** GPU-skinned crowd transforms update less often than foreground soldiers. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Performance", meta = (ClampMin = "0.016", ClampMax = "0.5"))
    float ProxyUpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Performance", meta = (ClampMin = "0"))
    int32 ProxyStartCullDistance = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Performance", meta = (ClampMin = "0"))
    int32 ProxyEndCullDistance = 12000;

    /** Deprecated target-disc proxy. Kept only for serialized level compatibility and never rendered. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Visual", meta = (DeprecatedProperty, DeprecationMessage = "Use ProxySkeletalMesh"))
    TObjectPtr<UStaticMesh> ProxyMesh;

    /** Full character surface used by the GPU-instanced background force. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Visual")
    TObjectPtr<USkeletalMesh> ProxySkeletalMesh;

    /** GPU animation provider containing a small set of phase-shifted run tracks. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Visual")
    TObjectPtr<UAnimSequenceTransformProviderData> ProxyAnimationProvider;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Visual")
    TObjectPtr<UAnimationAsset> ForegroundRunAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Visual")
    FVector ProxyScale = FVector(0.9f);

    /** Skeletal mesh pivot is at its feet, unlike capsule-centered foreground Actors. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Visual")
    float ProxyGroundOffset = 0.0f;

    /** Raises capsule-centered soldiers above NavMesh ground. Adjust for replacement assets. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Visual")
    float AgentGroundOffset = 92.0f;

    /** Seeded randomness keeps editor/gameplay reproduction while breaking the grid silhouette. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Formation Randomization")
    int32 FormationRandomSeed = 741953;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Formation Randomization", meta = (ClampMin = "0.0"))
    float LateralJitter = 52.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Formation Randomization", meta = (ClampMin = "0.0"))
    float LongitudinalJitter = 68.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Formation Randomization", meta = (ClampMin = "0.0", ClampMax = "30.0"))
    float YawJitterDegrees = 11.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Formation Randomization", meta = (ClampMin = "0.0", ClampMax = "0.15"))
    float ScaleVariation = 0.055f;

    /** Stops far-away GPU animation while retaining the full character silhouette and LOD. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Performance", meta = (ClampMin = "0.0", ClampMax = "0.1"))
    float ProxyAnimationMinScreenSize = 0.006f;

    /** LOD 1 is the default VR crowd floor; set 0 to allow full-detail LOD 0. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Performance", meta = (ClampMin = "0", ClampMax = "4"))
    int32 ProxyMinLOD = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Visual", meta = (ClampMin = "0.1", ClampMax = "3.0"))
    float MinRunAnimationRate = 0.86f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Visual", meta = (ClampMin = "0.1", ClampMax = "3.0"))
    float MaxRunAnimationRate = 1.14f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Integration")
    bool bAutoFindHwacha = true;

    /** Starts on the first loaded-arrow event, leaving level narration time intact. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Integration")
    bool bStartWhenHwachaLoaded = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Integration")
    bool bStartOnBeginPlay = false;

    /** Prevents the formation from reaching the Hwacha before its current procedure step. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Procedure Gates")
    bool bLimitApproachByHwachaProcedure = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Procedure Gates", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float LoadedApproachLimit = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Procedure Gates", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AimedApproachLimit = 0.60f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Procedure Gates", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float IgnitingApproachLimit = 0.82f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Procedure Gates", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FiringApproachLimit = 0.95f;

    /** Fraction of remaining logical enemies defeated by one 90-arrow volley. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Combat", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VolleyCasualtyFraction = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Combat", meta = (ClampMin = "1.0", ClampMax = "90.0"))
    float VolleyHalfAngleDegrees = 35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Combat", meta = (ClampMin = "100.0"))
    float VolleyMaxDistance = 20000.0f;

    UPROPERTY(BlueprintAssignable, Category = "Singijeon|Enemy Wave")
    FOnSingijeonEnemyWaveStateChanged OnWaveStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Singijeon|Enemy Wave")
    FOnSingijeonVolleyResolved OnVolleyResolved;

    UPROPERTY(BlueprintAssignable, Category = "Singijeon|Enemy Wave")
    FOnSingijeonEnemyWaveEvent OnWaveDefeated;

    UPROPERTY(BlueprintAssignable, Category = "Singijeon|Enemy Wave")
    FOnSingijeonEnemyWaveEvent OnWaveReachedTarget;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION()
    void HandleHwachaLoadCountChanged(int32 LoadedCount, int32 TotalSlots);

    UFUNCTION()
    void HandleHwachaVolleyLaunched();

    UFUNCTION()
    void HandleHwachaAimCompleted();

    UFUNCTION()
    void HandleHwachaStateChanged(ESingijeonHwachaState OldState, ESingijeonHwachaState NewState);

    UFUNCTION()
    void HandleInteractiveEnemyDepleted(AActor* OwnerActor);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UBoxComponent> SpawnVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> DefaultTargetPoint;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UHierarchicalInstancedStaticMeshComponent> ProxyInstances;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UInstancedSkinnedMeshComponent> CharacterInstances;

private:
    struct FEnemySlot
    {
        int32 Platoon = 0;
        float LongitudinalOffset = 0.0f;
        float LateralOffset = 0.0f;
        float SpeedScale = 1.0f;
        float YawOffset = 0.0f;
        float UniformScale = 1.0f;
        int32 AnimationIndex = 0;
        bool bAlive = true;
        int32 ProxyInstanceIndex = INDEX_NONE;
        TWeakObjectPtr<AEnemySoldierActor> InteractiveActor;
    };

    void FindAndBindHwacha();
    void UnbindHwacha();
    void BuildFormationSlots();
    bool SpawnVisualRepresentations();
    void DestroyVisualRepresentations();
    void UpdateInteractiveEnemies();
    void UpdateProxyEnemies(bool bMarkRenderStateDirty);
    FTransform GetSlotTransform(const FEnemySlot& Slot) const;
    float GetSlotRouteDistance(const FEnemySlot& Slot) const;
    void SampleRoute(float Distance, FVector& OutLocation, FVector& OutDirection) const;
    bool DefeatSlot(int32 SlotIndex);
    void SetWaveState(ESingijeonEnemyWaveState NewState);
    void SetVisualsActive(bool bActive);

    UPROPERTY(Transient)
    TObjectPtr<ASingijeonHwachaActor> BoundHwacha;

    TArray<FEnemySlot> EnemySlots;
    TArray<FVector> RoutePoints;
    TArray<float> RouteCumulativeDistances;
    float RouteLength = 0.0f;
    float WaveDistance = 0.0f;
    float ProxyUpdateAccumulator = 0.0f;
    float CurrentApproachLimitFraction = 1.0f;
    int32 AliveEnemyCount = 0;
    ESingijeonEnemyWaveState WaveState = ESingijeonEnemyWaveState::Hidden;
    ESingijeonEnemyApproachPhase ApproachPhase = ESingijeonEnemyApproachPhase::Unrestricted;
};
