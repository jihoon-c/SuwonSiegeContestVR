#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SingijeonAmmunitionInterface.generated.h"

class USceneComponent;

UINTERFACE(BlueprintType)
class GF_SINGIJEON_API USingijeonAmmunitionInterface : public UInterface
{
    GENERATED_BODY()
};

class GF_SINGIJEON_API ISingijeonAmmunitionInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Singijeon|Ammunition")
    bool CanBeLoaded() const;

    /** Called before the slot changes attachment. Override in Blueprint to release an active VR grab. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Singijeon|Ammunition")
    bool PrepareForLoading();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Singijeon|Ammunition")
    void OnLoaded(USceneComponent* Slot);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Singijeon|Ammunition")
    void OnUnloaded();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Singijeon|Ammunition")
    void OnLaunched(FVector Direction, float Speed);
};
