#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SingijeonEnemyWaveActor.generated.h"

class AEnemySoldierActor;
class ASingijeonHwachaActor;
class UAnimationAsset;
class UBoxComponent;
class UHierarchicalInstancedStaticMeshComponent;
class USceneComponent;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Enemy Wave")
    TObjectPtr<AActor> TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Enemy Wave")
    TSubclassOf<AEnemySoldierActor> ForegroundEnemyClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Enemy Wave", meta = (ClampMin = "1", ClampMax = "80"))
    int32 EnemyCount = 45;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Enemy Wave", meta = (ClampMin = "0", ClampMax = "16"))
    int32 MaxInteractiveEnemies = 10;

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

    /** HISM transforms update less often than foreground soldiers. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Performance", meta = (ClampMin = "0.016", ClampMax = "0.5"))
    float ProxyUpdateInterval = 0.0667f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Performance", meta = (ClampMin = "0"))
    int32 ProxyStartCullDistance = 2500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Performance", meta = (ClampMin = "0"))
    int32 ProxyEndCullDistance = 18000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Visual")
    TObjectPtr<UStaticMesh> ProxyMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Visual")
    TObjectPtr<UAnimationAsset> ForegroundRunAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Visual")
    FVector ProxyScale = FVector(0.9f);

    /** Raises capsule-centered soldiers above NavMesh ground. Adjust for replacement assets. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Visual")
    float AgentGroundOffset = 92.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Integration")
    bool bAutoFindHwacha = true;

    /** Starts on the first loaded-arrow event, leaving level narration time intact. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Integration")
    bool bStartWhenHwachaLoaded = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Integration")
    bool bStartOnBeginPlay = false;

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
    void HandleInteractiveEnemyDepleted(AActor* OwnerActor);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UBoxComponent> SpawnVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> DefaultTargetPoint;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UHierarchicalInstancedStaticMeshComponent> ProxyInstances;

private:
    struct FEnemySlot
    {
        int32 Platoon = 0;
        float LongitudinalOffset = 0.0f;
        float LateralOffset = 0.0f;
        float SpeedScale = 1.0f;
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
    int32 AliveEnemyCount = 0;
    ESingijeonEnemyWaveState WaveState = ESingijeonEnemyWaveState::Hidden;
};
