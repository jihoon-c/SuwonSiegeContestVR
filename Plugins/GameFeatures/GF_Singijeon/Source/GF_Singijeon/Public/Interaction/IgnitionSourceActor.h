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

    virtual bool IsIgnitionActive_Implementation() const override;

    UFUNCTION(BlueprintCallable, Category = "Ignition")
    void SetIgnitionActive(bool bNewActive);

    UPROPERTY(BlueprintAssignable, Category = "Ignition")
    FOnIgnitionSourceStateChanged OnIgnitionSourceStateChanged;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> SourceMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> IgnitionArea;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UScenarioInteractableComponent> GrabScenarioInteractor;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ignition")
    bool bIgnitionActive = true;
};
