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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSingijeonProjectileImpact, AActor*, HitActor);

UCLASS(Blueprintable)
class GF_SINGIJEON_API ASingijeonProjectileActor : public AActor, public ISingijeonAmmunitionInterface
{
    GENERATED_BODY()

public:
    ASingijeonProjectileActor();

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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UScenarioInteractableComponent> GrabScenarioInteractor;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Singijeon|Visual")
    TObjectPtr<UMaterialInterface> ProjectileMaterialOverride;

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
    UFUNCTION()
    void HandleProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

    bool bDamageApplied = false;
};
