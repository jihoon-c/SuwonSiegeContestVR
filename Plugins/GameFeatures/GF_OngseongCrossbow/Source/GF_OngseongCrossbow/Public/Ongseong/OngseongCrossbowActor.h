#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OngseongCrossbowActor.generated.h"

class AActorPool;
class AGameplayProjectileActor;
class UCombatFactionComponent;
class UOngseongCrossbowGripComponent;
class USceneComponent;
class UStaticMeshComponent;
class UVRHUDComponent;

UENUM(BlueprintType)
enum class EOngseongCrossbowState : uint8
{
	Loaded,
	Reloading,
	Empty
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnOngseongCrossbowFired, AGameplayProjectileActor*, Projectile, int32, RemainingAmmo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOngseongCrossbowStateChanged, EOngseongCrossbowState, NewState);

/** Mounted two-hand crossbow with physical bolts and timed automatic reloading. */
UCLASS(Blueprintable)
class GF_ONGSEONGCROSSBOW_API AOngseongCrossbowActor : public AActor
{
	GENERATED_BODY()

public:
	AOngseongCrossbowActor();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category="Ongseong|Crossbow")
	bool TryFire();
	UFUNCTION(BlueprintCallable, Category="Ongseong|Crossbow")
	bool BeginReload();
	UFUNCTION(BlueprintCallable, Category="Ongseong|Crossbow")
	void CompleteReload();
	UFUNCTION(BlueprintCallable, Category="Ongseong|Crossbow")
	void RefillAmmo(int32 Amount);
	UFUNCTION(BlueprintPure, Category="Ongseong|Crossbow")
	EOngseongCrossbowState GetCrossbowState() const { return CrossbowState; }
	UFUNCTION(BlueprintPure, Category="Ongseong|Crossbow")
	int32 GetRemainingAmmo() const { return RemainingAmmo; }

	void HandleGripStateChanged(bool bTwoHandAiming);

	UPROPERTY(BlueprintAssignable, Category="Ongseong|Crossbow")
	FOnOngseongCrossbowFired OnFired;
	UPROPERTY(BlueprintAssignable, Category="Ongseong|Crossbow")
	FOnOngseongCrossbowStateChanged OnStateChanged;

protected:
	void SetCrossbowState(EOngseongCrossbowState NewState);
	AGameplayProjectileActor* SpawnBolt(const FVector& Direction);
	UVRHUDComponent* ResolveHUD();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> Root;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> BaseMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> AimPivot;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> CrossbowMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> Muzzle;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UOngseongCrossbowGripComponent> Grip;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UCombatFactionComponent> FactionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Crossbow")
	TSubclassOf<AGameplayProjectileActor> BoltClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Crossbow")
	TObjectPtr<AActorPool> BoltPool;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Crossbow", meta=(ClampMin="1.0"))
	float BoltSpeed = 9000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Crossbow", meta=(ClampMin="0.0"))
	float BoltDamage = 55.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Crossbow", meta=(ClampMin="0.0"))
	float ReloadDuration = 1.25f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Crossbow", meta=(ClampMin="0"))
	int32 InitialAmmo = 12;
	/** Zero means unlimited ammunition. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Crossbow", meta=(ClampMin="0"))
	int32 MaxAmmo = 12;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Crossbow", meta=(ClampMin="0.0"))
	float ProjectileSpawnClearance = 45.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Ongseong|Crossbow")
	EOngseongCrossbowState CrossbowState = EOngseongCrossbowState::Loaded;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Ongseong|Crossbow")
	int32 RemainingAmmo = 12;
	UPROPERTY(Transient)
	TObjectPtr<UVRHUDComponent> VRHUD;
	FTimerHandle ReloadTimerHandle;
};
