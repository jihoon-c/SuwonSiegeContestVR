#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FirePitActor.generated.h"

class UScenarioInteractableComponent;
class USoundBase;
class USphereComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFirePitIgnitedTorch, AActor*, TorchActor);

UCLASS(Blueprintable)
class GF_SINGIJEON_API AFirePitActor : public AActor
{
    GENERATED_BODY()

public:
    AFirePitActor();

    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "Singijeon|Fire Pit")
    bool TryIgniteTorch(AActor* Candidate);

    UPROPERTY(BlueprintAssignable, Category = "Singijeon|Fire Pit")
    FOnFirePitIgnitedTorch OnTorchIgnited;

protected:
    /** Reapplies the attached FireEffect transform after Blueprint/level construction. */
    void SnapFireEffectToBowl();

    UFUNCTION()
    void HandleIgnitionOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> FirePitMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> IgnitionArea;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UScenarioInteractableComponent> IgniteTorchScenarioInteractor;

    /** Sound played once after a scenario-approved torch ignition. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Singijeon|Fire Pit|Audio")
    TObjectPtr<USoundBase> TorchIgnitionSound;

    /** Local offset that accounts for the imported Fire Pit mesh pivot. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Singijeon|Fire Pit|Visual")
    FVector FireEffectRelativeLocation = FVector(0.0f, 0.0f, 280.0f);
};
