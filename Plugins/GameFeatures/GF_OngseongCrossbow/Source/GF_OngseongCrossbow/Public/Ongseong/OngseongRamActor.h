#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Combat/DamageReceiverInterface.h"
#include "Gameplay/Combat/CombatTypes.h"
#include "OngseongRamActor.generated.h"

class UCombatFactionComponent;
class UHealthComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EOngseongRamState : uint8
{
	Inactive,
	Advancing,
	Charging,
	Returning
};

/** Destructible enemy ram that advances with the soldiers, then repeatedly charges the gate. */
UCLASS(Blueprintable)
class GF_ONGSEONGCROSSBOW_API AOngseongRamActor : public AActor, public IDamageReceiverInterface
{
	GENERATED_BODY()

public:
	AOngseongRamActor();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual bool ReceiveCombatDamage_Implementation(const FCombatDamageSpec& DamageSpec) override;

	UFUNCTION(BlueprintCallable, Category="Ongseong|Ram")
	void ActivateRam(AActor* NewGateTarget);
	UFUNCTION(BlueprintCallable, Category="Ongseong|Ram")
	void StopRam();
	UFUNCTION(BlueprintPure, Category="Ongseong|Ram")
	EOngseongRamState GetRamState() const { return RamState; }

protected:
	bool MoveTowards(const FVector& TargetLocation, float Speed, float DeltaSeconds);
	void BeginCharge();
	void ImpactGate();
	UFUNCTION()
	void HandleDeath(UHealthComponent* DeadHealth, const FCombatDamageSpec& KillingDamage);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ongseong|Ram")
	TObjectPtr<UStaticMeshComponent> RamMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ongseong|Ram")
	TObjectPtr<UHealthComponent> HealthComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ongseong|Ram")
	TObjectPtr<UCombatFactionComponent> FactionComponent;
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Ongseong|Ram")
	TObjectPtr<AActor> GateTarget;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Ram", meta=(ClampMin="0.0"))
	float MoveSpeed = 120.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Ram", meta=(ClampMin="0.0"))
	float StagingDistance = 800.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Ram", meta=(ClampMin="0.0"))
	float ImpactDistance = 150.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Ram", meta=(ClampMin="0.0"))
	float ChargeSpeed = 420.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Ram", meta=(ClampMin="0.0"))
	float ReturnSpeed = 180.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Ram", meta=(ClampMin="1.0"))
	float ArrivalTolerance = 15.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Ram", meta=(ClampMin="0.0"))
	float AttackDamage = 75.0f;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Ongseong|Ram")
	EOngseongRamState RamState = EOngseongRamState::Inactive;
	FVector StagingLocation = FVector::ZeroVector;
};
