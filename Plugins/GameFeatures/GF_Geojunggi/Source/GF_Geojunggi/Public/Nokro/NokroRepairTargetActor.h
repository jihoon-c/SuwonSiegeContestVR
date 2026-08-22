#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NokroRepairTargetActor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

/** A yellow damaged-wall marker and its final placed stone. */
UCLASS(Blueprintable)
class GF_GEOJUNGGI_API ANokroRepairTargetActor : public AActor
{
	GENERATED_BODY()

public:
	ANokroRepairTargetActor();

	UFUNCTION(BlueprintPure, Category="Nokro|Repair")
	bool IsStoneWithinTolerance(const FTransform& StoneTransform) const;

	UFUNCTION(BlueprintCallable, Category="Nokro|Repair")
	void CompleteRepair();

	UFUNCTION(BlueprintCallable, Category="Nokro|Repair")
	void ResetRepair();

	UFUNCTION(BlueprintPure, Category="Nokro|Repair")
	bool IsRepaired() const { return bRepaired; }

	UFUNCTION(BlueprintPure, Category="Nokro|Repair")
	FTransform GetPlacementTransform() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Nokro|Components")
	TObjectPtr<UBoxComponent> PlacementVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Nokro|Components")
	TObjectPtr<UStaticMeshComponent> YellowMarker;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Nokro|Components")
	TObjectPtr<UStaticMeshComponent> PlacedStone;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Nokro|Validation", meta=(ClampMin="1.0"))
	float PositionTolerance = 28.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Nokro|Validation", meta=(ClampMin="0.0", ClampMax="180.0"))
	float YawTolerance = 18.0f;

private:
	UPROPERTY(VisibleInstanceOnly, Category="Nokro|Repair")
	bool bRepaired = false;
};
