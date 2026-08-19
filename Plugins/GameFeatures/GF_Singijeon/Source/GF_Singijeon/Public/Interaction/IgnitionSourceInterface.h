#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IgnitionSourceInterface.generated.h"

UINTERFACE(BlueprintType)
class GF_SINGIJEON_API UIgnitionSourceInterface : public UInterface
{
    GENERATED_BODY()
};

class GF_SINGIJEON_API IIgnitionSourceInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ignition")
    bool IsIgnitionActive() const;
};
