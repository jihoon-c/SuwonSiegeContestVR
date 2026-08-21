#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FirePitActor.generated.h"

class UScenarioInteractableComponent;
class USphereComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFirePitIgnitedTorch, AActor*, TorchActor);

UCLASS(Blueprintable)
class GF_SINGIJEON_API AFirePitActor : public AActor
{
    GENERATED_BODY()

public:
    AFirePitActor();

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "Singijeon|Fire Pit")
    bool TryIgniteTorch(AActor* Candidate);

    UPROPERTY(BlueprintAssignable, Category = "Singijeon|Fire Pit")
    FOnFirePitIgnitedTorch OnTorchIgnited;

protected:
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
};
