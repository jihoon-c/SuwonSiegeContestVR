#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ChongtongAutomaticFireComponent.generated.h"

/**
 * Optional automatic-fire behavior for a Chongtong cannon.
 * Keeps the fire schedule out of the shared cannon actor so ally and player
 * Blueprint variants can select their control behavior through composition.
 */
UCLASS(ClassGroup=(Ongseong), meta=(BlueprintSpawnableComponent))
class GF_ONGSEONGCROSSBOW_API UChongtongAutomaticFireComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UChongtongAutomaticFireComponent();

	UFUNCTION(BlueprintCallable, Category="Ongseong|Chongtong|Automatic Fire")
	void ConfigureAutomaticFire(bool bEnabled, float InFireInterval);

	UFUNCTION(BlueprintCallable, Category="Ongseong|Chongtong|Automatic Fire")
	void StartAutomaticFire();

	UFUNCTION(BlueprintCallable, Category="Ongseong|Chongtong|Automatic Fire")
	void StopAutomaticFire();

	UFUNCTION(BlueprintPure, Category="Ongseong|Chongtong|Automatic Fire")
	bool IsAutomaticFireEnabled() const { return bAutomaticFireEnabled; }

	UFUNCTION(BlueprintPure, Category="Ongseong|Chongtong|Automatic Fire")
	float GetFireInterval() const { return FireInterval; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void FireScheduledShot();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Automatic Fire")
	bool bAutomaticFireEnabled = false;

	/** Delay between allied AI shots. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Automatic Fire", meta=(ClampMin="0.1"))
	float FireInterval = 5.0f;

	/** Random delay applied to each allied shot (5 +/- 1 seconds by default). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Automatic Fire", meta=(ClampMin="0.0"))
	float FireIntervalJitter = 1.0f;

	FTimerHandle FireTimerHandle;
};
