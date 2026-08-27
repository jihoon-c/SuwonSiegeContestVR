#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Singijeon/SingijeonHwachaActor.h"
#include "SingijeonHwachaBatteryActor.generated.h"

class UInstancedStaticMeshComponent;
class UArrowComponent;
class UMaterialInterface;
class USceneComponent;
class USoundBase;
class UStaticMesh;

/**
 * One independently placeable visual-only support Hwacha synchronized to the playable Hwacha.
 * The cart and its arrows are rendered through three ISMs; no arrow actors,
 * collision, physics, Niagara, or per-projectile tick are created.
 */
UCLASS(Blueprintable)
class GF_SINGIJEON_API ASingijeonHwachaBatteryActor : public AActor
{
    GENERATED_BODY()

public:
    ASingijeonHwachaBatteryActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void OnConstruction(const FTransform& Transform) override;

    UFUNCTION(BlueprintCallable, Category = "Singijeon|Support Battery")
    void StartSynchronizedVolley();

    UFUNCTION(BlueprintCallable, Category = "Singijeon|Support Battery")
    void ResetBattery();

    UFUNCTION(BlueprintPure, Category = "Singijeon|Support Battery")
    int32 GetCartCount() const;

    UFUNCTION(BlueprintPure, Category = "Singijeon|Support Battery")
    int32 GetLoadedArrowCount() const;

    UFUNCTION(BlueprintPure, Category = "Singijeon|Support Battery")
    int32 GetFlyingArrowCount() const;

protected:
    UFUNCTION()
    void HandleSourceHwachaStateChanged(
        ESingijeonHwachaState OldState, ESingijeonHwachaState NewState);

    void ResolveSourceHwacha();
    void RebuildInstances();
    void ConfigureInstanceComponent(UInstancedStaticMeshComponent* Component, bool bCastShadow) const;
    void ApplyArrowMaterial(UInstancedStaticMeshComponent* Component) const;
    void LaunchOneArrow();
    void UpdateFlyingArrows(float StepSeconds);
    void StopTickIfIdle();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UInstancedStaticMeshComponent> HwachaInstances;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UInstancedStaticMeshComponent> LoadedArrowInstances;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UInstancedStaticMeshComponent> FlyingArrowInstances;

    /** Editor-only placement guide for RackTransform. Hidden during play. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UArrowComponent> RackEditorGuide;

    /** Playable Hwacha whose Fired state starts this visual-only battery. */
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Singijeon|Support Battery")
    TObjectPtr<ASingijeonHwachaActor> SourceHwacha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Support Battery|Visual")
    TObjectPtr<UStaticMesh> HwachaMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Support Battery|Visual")
    TObjectPtr<UStaticMesh> ArrowMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Support Battery|Visual")
    TObjectPtr<UMaterialInterface> ArrowMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Support Battery|Layout", meta = (ClampMin = "1"))
    int32 ArrowRows = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Support Battery|Layout", meta = (ClampMin = "1"))
    int32 ArrowColumns = 11;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Support Battery|Layout", meta = (ClampMin = "0.1", Units = "cm"))
    float ArrowRowSpacing = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Support Battery|Layout", meta = (ClampMin = "0.1", Units = "cm"))
    float ArrowColumnSpacing = 8.0f;

    /** Arrow-grid origin and tilt. Its viewport widget can be dragged before PIE. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Support Battery|Layout",
        meta = (MakeEditWidget = "true", DisplayName = "Arrow Rack Transform"))
    FTransform RackTransform = FTransform(FRotator(25.0f, 0.0f, 0.0f), FVector(25.0f, 10.0f, 80.0f));

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Support Battery|Launch", meta = (ClampMin = "0.0", Units = "s"))
    float VolleyDuration = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Support Battery|Launch", meta = (ClampMin = "1.0", Units = "cm/s"))
    float LaunchSpeed = 3500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Support Battery|Launch|Spread",
        meta = (ClampMin = "0.0", ClampMax = "45.0", Units = "deg"))
    float VolleyHorizontalSpreadHalfAngle = 14.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Support Battery|Launch|Spread",
        meta = (ClampMin = "0.0", ClampMax = "30.0", Units = "deg"))
    float VolleyVerticalSpreadHalfAngle = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Support Battery|Launch", meta = (Units = "cm/s^2"))
    float GravityZ = -980.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Support Battery|Launch", meta = (ClampMin = "0.1", Units = "s"))
    float FlyingArrowLifetime = 6.0f;

    /** All flying instances are updated together at this rate instead of every arrow ticking. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Support Battery|Optimization", meta = (ClampMin = "0.016", Units = "s"))
    float VisualUpdateInterval = 1.0f / 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Support Battery|Optimization", meta = (ClampMin = "0"))
    int32 StartCullDistance = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Support Battery|Optimization", meta = (ClampMin = "0"))
    int32 EndCullDistance = 20000;

    /** Optional sampled support-battery audio. Zero disables it. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Support Battery|Audio")
    TObjectPtr<USoundBase> LaunchSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Support Battery|Audio", meta = (ClampMin = "0"))
    int32 SoundEveryNthArrow = 16;

private:
    struct FFlyingArrowState
    {
        FVector Location = FVector::ZeroVector;
        FVector Velocity = FVector::ZeroVector;
        float RemainingLifetime = 0.0f;
    };

    TArray<FFlyingArrowState> FlyingArrows;
    bool bVolleyActive = false;
    float VolleyElapsed = 0.0f;
    float NextLaunchTime = 0.0f;
    float LaunchInterval = 0.0f;
    float VisualUpdateAccumulator = 0.0f;
    int32 LaunchedArrowCount = 0;
    FRandomStream LaunchRandomStream;
};
