#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/IgnitionSourceInterface.h"
#include "IgnitionSourceActor.generated.h"

class USceneComponent;
class UScenarioInteractableComponent;
class USphereComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIgnitionSourceStateChanged, bool, bIsActive);

UCLASS(Blueprintable)
class GF_SINGIJEON_API AIgnitionSourceActor : public AActor, public IIgnitionSourceInterface
{
    GENERATED_BODY()

public:
    AIgnitionSourceActor();

    virtual void BeginPlay() override;
    virtual bool IsIgnitionActive_Implementation() const override;

    UFUNCTION(BlueprintCallable, Category = "Ignition")
    void SetIgnitionActive(bool bNewActive);

    /** Allocates and warms Niagara once during level start, before the VR interaction. */
    UFUNCTION(BlueprintCallable, Category = "Ignition|Performance")
    void PrepareIgnitionVisuals();

    UFUNCTION(BlueprintPure, Category = "Ignition|Performance")
    bool AreIgnitionVisualsPrepared() const { return bIgnitionVisualsPrepared; }

    UPROPERTY(BlueprintAssignable, Category = "Ignition")
    FOnIgnitionSourceStateChanged OnIgnitionSourceStateChanged;

protected:
    void RefreshIgnitionVisuals();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> SourceMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> IgnitionArea;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UScenarioInteractableComponent> GrabScenarioInteractor;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ignition")
    bool bIgnitionActive = false;

    /** Small hidden warmup avoids allocating the torch Niagara system on overlap. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ignition|Performance",
        meta = (ClampMin = "0", ClampMax = "4"))
    int32 IgnitionEffectWarmupTicks = 1;

    /** Torch fire is unnecessary beyond this range in the VR scene. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ignition|Performance",
        meta = (ClampMin = "0.0", Units = "cm"))
    float IgnitionEffectCullDistance = 1500.0f;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ignition|Performance")
    bool bIgnitionVisualsPrepared = false;
};
