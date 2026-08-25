#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Singijeon/SingijeonAmmunitionInterface.h"
#include "SingijeonProjectileActor.generated.h"

class UProjectileMovementComponent;
class UPrimitiveComponent;
class UScenarioInteractableComponent;
class UStaticMeshComponent;
class UMaterialInterface;
class UNiagaraComponent;
class UNiagaraSystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSingijeonProjectileImpact, AActor*, HitActor);

UCLASS(Blueprintable)
class GF_SINGIJEON_API ASingijeonProjectileActor : public AActor, public ISingijeonAmmunitionInterface
{
    GENERATED_BODY()

public:
    ASingijeonProjectileActor();

    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void Tick(float DeltaSeconds) override;
    virtual FVector GetVelocity() const override;

    virtual bool CanBeLoaded_Implementation() const override;
    virtual bool PrepareForLoading_Implementation() override;
    virtual void OnLoaded_Implementation(USceneComponent* Slot) override;
    virtual void OnUnloaded_Implementation() override;
    virtual void OnLaunched_Implementation(FVector Direction, float Speed) override;
    virtual void RefreshLoadedVisual_Implementation() override;

    /** Re-applies the authored material after XR Grab highlight/release state changes. */
    UFUNCTION(BlueprintCallable, Category = "Singijeon|Visual")
    void ApplyProjectileMaterial();

    /** World direction in which the authored arrowhead points. */
    UFUNCTION(BlueprintPure, Category = "Singijeon|Launch")
    FVector GetArrowTipDirection() const;

    /** Applies the projectile's standard damage contract once after launch. */
    UFUNCTION(BlueprintCallable, Category = "Singijeon|Combat")
    bool ApplyImpactDamage(AActor* OtherActor);

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Singijeon|Combat",
        meta = (ClampMin = "0.0"))
    float ImpactDamage = 100.0f;

    UPROPERTY(BlueprintAssignable, Category = "Singijeon|Combat")
    FOnSingijeonProjectileImpact OnProjectileImpact;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> ProjectileMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

    /** Optional smoke/tracer effect. It stays inactive until this arrow is launched. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UNiagaraComponent> FlightTrailEffect;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UScenarioInteractableComponent> GrabScenarioInteractor;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Singijeon|Visual")
    TObjectPtr<UMaterialInterface> ProjectileMaterialOverride;

    /** Niagara System used while the projectile is flying. Leave empty for no trail. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Flight Trail")
    TObjectPtr<UNiagaraSystem> FlightTrailSystem;

    /** Trail emitter offset in ProjectileMesh local space. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Flight Trail")
    FVector FlightTrailRelativeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Flight Trail")
    FRotator FlightTrailRelativeRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Flight Trail")
    FVector FlightTrailRelativeScale = FVector::OneVector;

    /** Arrowhead direction in mesh-local space. The imported Singijeon mesh points toward -X. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Singijeon|Launch")
    FVector ArrowTipLocalAxis = FVector(-1.0f, 0.0f, 0.0f);

    /** Removes launched arrows after their visible flight so a full volley cannot accumulate forever. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Singijeon|Launch",
        meta = (ClampMin = "0.0", Units = "s"))
    float LaunchedLifeSpan = 12.0f;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Singijeon")
    bool bIsLoaded = false;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Singijeon")
    bool bWasLaunched = false;

private:
    void ApplyFlightTrailSettings();
    void StopFlightTrail();

    UFUNCTION()
    void HandleProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

    bool bDamageApplied = false;
};
