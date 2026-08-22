#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NokroNarrationComponent.generated.h"

class UDataTable;
class UNarrationSequenceComponent;

USTRUCT(BlueprintType)
struct GF_GEOJUNGGI_API FNokroNarrationEventBinding
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Nokro|Narration")
	FName EventName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Nokro|Narration")
	FName NarrationRow;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Nokro|Narration")
	bool bPlayOnce = true;
};

/** Feature-local event adapter that queues rows on the player's shared narration component. */
UCLASS(ClassGroup=(Nokro), meta=(BlueprintSpawnableComponent))
class GF_GEOJUNGGI_API UNokroNarrationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNokroNarrationComponent();

	UFUNCTION(BlueprintCallable, Category="Nokro|Narration")
	bool InitializeNarration();

	UFUNCTION(BlueprintCallable, Category="Nokro|Narration")
	void ReportScenarioEvent(FName EventName);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Nokro|Narration")
	TSoftObjectPtr<UDataTable> NarrationTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Nokro|Narration")
	TArray<FNokroNarrationEventBinding> EventBindings;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void TryPlayNext();

	UFUNCTION()
	void HandleSequenceFinished();

	UPROPERTY(Transient)
	TObjectPtr<UNarrationSequenceComponent> NarrationSequence;
	TArray<FName> PendingRows;
	TSet<FName> PlayedOnceEvents;
	bool bOwnsCurrentNarration = false;
};
