#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NokroRepairTargetActor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNokroTargetStateChanged);

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

	/** Only the current repair point is highlighted and eligible for placement. */
	UFUNCTION(BlueprintCallable, Category="Nokro|Repair")
	void SetTargetActive(bool bActive);

	UFUNCTION(BlueprintPure, Category="Nokro|Repair")
	bool IsRepaired() const { return bRepaired; }

	UFUNCTION(BlueprintPure, Category="Nokro|Repair")
	bool IsTargetActive() const { return bTargetActive; }

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

	/** Designer hooks for replacing the placeholder highlight/placement particles without changing flow code. */
	UPROPERTY(BlueprintAssignable, Category="Nokro|Events")
	FOnNokroTargetStateChanged OnTargetActivated;

	UPROPERTY(BlueprintAssignable, Category="Nokro|Events")
	FOnNokroTargetStateChanged OnRepairCompleted;

	/** Implement in BP_NokroRepairTarget to swap the target-highlight particle. */
	UFUNCTION(BlueprintImplementableEvent, Category="Nokro|FX", meta=(DisplayName="On Target Activated FX"))
	void BP_OnTargetActivatedFX();

	/** Implement in BP_NokroRepairTarget to swap the stone-placement particle. */
	UFUNCTION(BlueprintImplementableEvent, Category="Nokro|FX", meta=(DisplayName="On Repair Completed FX"))
	void BP_OnRepairCompletedFX();

private:
	void RefreshVisualState();

	UPROPERTY(VisibleInstanceOnly, Category="Nokro|Repair")
	bool bRepaired = false;

	UPROPERTY(VisibleInstanceOnly, Category="Nokro|Repair")
	bool bTargetActive = false;
};
