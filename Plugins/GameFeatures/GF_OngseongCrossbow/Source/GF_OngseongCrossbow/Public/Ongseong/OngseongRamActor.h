#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Combat/DamageReceiverInterface.h"
#include "Gameplay/Combat/CombatTypes.h"
#include "Gameplay/Pooling/PoolableActorInterface.h"
#include "OngseongRamActor.generated.h"

class UCombatFactionComponent;
class UHealthComponent;
class UInteractionHighlightComponent;
class USceneComponent;
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
class GF_ONGSEONGCROSSBOW_API AOngseongRamActor : public AActor, public IDamageReceiverInterface, public IPoolableActorInterface
{
	GENERATED_BODY()

public:
	AOngseongRamActor();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual bool ReceiveCombatDamage_Implementation(const FCombatDamageSpec& DamageSpec) override;
	virtual void OnAcquiredFromPool_Implementation() override;
	virtual void OnReleasedToPool_Implementation() override;

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

	/**
	 * Neutral transform parent for all authored ram visuals.  Keeping the moving Actor root separate
	 * means Blueprint authors can offset, rotate, and scale the ram art without changing its pathing.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ongseong|Ram|Components")
	TObjectPtr<USceneComponent> VisualRoot;

	/** Main body mesh. Edit its Transform and add further children in BP_OngseongRam's Components panel. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ongseong|Ram|Components")
	TObjectPtr<UStaticMeshComponent> RamMesh;
	/** Pulsing red rim overlay so the objective reads clearly at range. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ongseong|Ram|Components")
	TObjectPtr<UInteractionHighlightComponent> VisibilityHighlight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Ram|Visual")
	bool bShowVisibilityHighlight = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Ram|Visual")
	FLinearColor VisibilityHighlightColor = FLinearColor(1.0f, 0.035f, 0.015f, 1.0f);

	/** The rim breathes instead of glowing flat, so the ram reads as the thing to shoot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Ram|Visual")
	bool bPulseVisibilityHighlight = true;

	/** Full bright-to-dim-to-bright cycles per second. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Ram|Visual", meta=(ClampMin="0.01"))
	float VisibilityHighlightPulsesPerSecond = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ongseong|Ram")
	TObjectPtr<UHealthComponent> HealthComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ongseong|Ram")
	TObjectPtr<UCombatFactionComponent> FactionComponent;
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Ongseong|Ram")
	TObjectPtr<AActor> GateTarget;
	/**
	 * Deliberately slow first approach: the ram is the objective target and must be killable in time.
	 * 42 cm/s covers the 7,450 cm from the spawn point to the staging point in about three minutes,
	 * which is the intended length of the defense.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Ram", meta=(ClampMin="0.0"))
	float MoveSpeed = 42.0f;
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
	/**
	 * The ram mesh is authored facing its own +Y, but movement rotation faces +X, so the ram drove
	 * sideways. Yaw correction applied on top of the travel direction; flip the sign if the final
	 * art is authored the other way round.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Ram", meta=(ClampMin="-180.0", ClampMax="180.0"))
	float MeshYawOffset = -90.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Ram", meta=(ClampMin="0.0"))
	float AttackDamage = 75.0f;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Ongseong|Ram")
	EOngseongRamState RamState = EOngseongRamState::Inactive;
	FVector StagingLocation = FVector::ZeroVector;
};
