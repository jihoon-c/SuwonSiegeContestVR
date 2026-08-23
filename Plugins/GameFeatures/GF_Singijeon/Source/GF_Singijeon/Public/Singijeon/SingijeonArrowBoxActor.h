#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SingijeonArrowBoxActor.generated.h"

class UInstancedStaticMeshComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class GF_SINGIJEON_API ASingijeonArrowBoxActor : public AActor
{
    GENERATED_BODY()
public:
    ASingijeonArrowBoxActor();
protected:
    virtual void OnConstruction(const FTransform& Transform) override;
    UPROPERTY(VisibleAnywhere, Category="Components") TObjectPtr<UStaticMeshComponent> Bottom;
    UPROPERTY(VisibleAnywhere, Category="Components") TObjectPtr<UStaticMeshComponent> LeftWall;
    UPROPERTY(VisibleAnywhere, Category="Components") TObjectPtr<UStaticMeshComponent> RightWall;
    UPROPERTY(VisibleAnywhere, Category="Components") TObjectPtr<UStaticMeshComponent> FrontWall;
    UPROPERTY(VisibleAnywhere, Category="Components") TObjectPtr<UStaticMeshComponent> BackWall;
    UPROPERTY(VisibleAnywhere, Category="Components") TObjectPtr<UInstancedStaticMeshComponent> ArrowInstances;
};
