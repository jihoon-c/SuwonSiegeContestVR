#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Combat/DamageReceiverInterface.h"
#include "Gameplay/Combat/CombatTypes.h"
#include "OngseongGateActor.generated.h"

class UCombatFactionComponent;
class UHealthComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOngseongGateHealthRatioChanged, float, HealthRatio);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOngseongGateDestroyed);

/** Feature-owned gate objective. Shared components remain the authority for faction and health. */
UCLASS(Blueprintable)
class GF_ONGSEONGCROSSBOW_API AOngseongGateActor : public AActor, public IDamageReceiverInterface
{
	GENERATED_BODY()

public:
	AOngseongGateActor();
	virtual void BeginPlay() override;
	virtual bool ReceiveCombatDamage_Implementation(const FCombatDamageSpec& DamageSpec) override;

	UFUNCTION(BlueprintPure, Category="Ongseong|Gate")
	UHealthComponent* GetHealthComponent() const { return HealthComponent; }

	UFUNCTION(BlueprintCallable, Category="Ongseong|Gate")
	void ResetGate();

	UPROPERTY(BlueprintAssignable, Category="Ongseong|Gate")
	FOnOngseongGateHealthRatioChanged OnGateHealthRatioChanged;
	UPROPERTY(BlueprintAssignable, Category="Ongseong|Gate")
	FOnOngseongGateDestroyed OnGateDestroyed;

protected:
	UFUNCTION()
	void HandleHealthChanged(UHealthComponent* ChangedHealth, float PreviousHealth, float NewHealth, float AppliedDelta);
	UFUNCTION()
	void HandleDeath(UHealthComponent* DeadHealth, const FCombatDamageSpec& KillingDamage);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ongseong|Gate")
	TObjectPtr<UStaticMeshComponent> GateMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ongseong|Gate")
	TObjectPtr<UHealthComponent> HealthComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ongseong|Gate")
	TObjectPtr<UCombatFactionComponent> FactionComponent;
};
