#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Singijeon/SingijeonAmmunitionInterface.h"
#include "SingijeonProjectileActor.generated.h"

class UProjectileMovementComponent;
class UScenarioInteractableComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class GF_SINGIJEON_API ASingijeonProjectileActor : public AActor, public ISingijeonAmmunitionInterface
{
    GENERATED_BODY()

public:
    ASingijeonProjectileActor();

    virtual bool CanBeLoaded_Implementation() const override;
    virtual bool PrepareForLoading_Implementation() override;
    virtual void OnLoaded_Implementation(USceneComponent* Slot) override;
    virtual void OnUnloaded_Implementation() override;
    virtual void OnLaunched_Implementation(FVector Direction, float Speed) override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> ProjectileMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UScenarioInteractableComponent> GrabScenarioInteractor;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Singijeon")
    bool bIsLoaded = false;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Singijeon")
    bool bWasLaunched = false;
};
