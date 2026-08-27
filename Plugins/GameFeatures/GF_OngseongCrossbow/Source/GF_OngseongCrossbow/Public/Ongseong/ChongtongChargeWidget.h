#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChongtongChargeWidget.generated.h"

class UProgressBar;
class UTextBlock;

/** Small world-space power meter shown behind the player-operated chongtong. */
UCLASS()
class GF_ONGSEONGCROSSBOW_API UChongtongChargeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category="Ongseong|Chongtong|Charge")
	void SetChargePercent(float Percent);

private:
	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> ChargeBar;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ChargeText;
};
