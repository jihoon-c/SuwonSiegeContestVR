#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Combat/DamageReceiverInterface.h"
#include "Ongseong/ChongtongInteractionTypes.h"
#include "ChongtongCannonActor.generated.h"

class AGameplayProjectileActor;
class AActorPool;
class AAllyCombatCharacter;
class UCombatFactionComponent;
class UHealthComponent;
class UCombatTargetingComponent;
class UCombatThreatComponent;
class USceneComponent;
class UStaticMeshComponent;
class UChongtongAimGripComponent;
class UInteractionHighlightComponent;
class UChongtongAutomaticFireComponent;
class AChongtongLoadingItemActor;
class UTextRenderComponent;
class UPointLightComponent;
class UNiagaraSystem;
class USoundBase;
class UOngseongNarrationComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChongtongFired, AActor*, Target, AGameplayProjectileActor*, Projectile);

/** Defensive fixed cannon. Allied automatic fire answers enemy archers only; see bEngageEnemyArchersOnly. */
UCLASS(Blueprintable)
class GF_ONGSEONGCROSSBOW_API AChongtongCannonActor : public AActor, public IDamageReceiverInterface
{
	GENERATED_BODY()

public:
	AChongtongCannonActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual bool ReceiveCombatDamage_Implementation(const FCombatDamageSpec& DamageSpec) override;

	/** Visibility is judged from the barrel, not the actor origin. See the .cpp for why. */
	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;

	UFUNCTION(BlueprintCallable, Category = "Ongseong|Chongtong")
	void SetGateTarget(AActor* NewGateTarget);

	UFUNCTION(BlueprintCallable, Category = "Ongseong|Chongtong")
	bool TryFire();

	/** Player shot: requires a complete loading cycle, two grips and both triggers. */
	UFUNCTION(BlueprintCallable, Category = "Ongseong|Chongtong|Player")
	bool TryFirePlayer();

	UFUNCTION(BlueprintCallable, Category = "Ongseong|Chongtong|Player")
	void BeginPlayerAim();
	UFUNCTION(BlueprintCallable, Category = "Ongseong|Chongtong|Player")
	void EndPlayerAim();

	UFUNCTION(BlueprintCallable, Category = "Ongseong|Chongtong|Loading")
	bool TryLoadItem(EChongtongLoadingItemType ItemType);
	UFUNCTION(BlueprintCallable, Category = "Ongseong|Chongtong|Loading")
	bool RegisterRammerStroke();
	UFUNCTION(BlueprintPure, Category = "Ongseong|Chongtong|Loading")
	EChongtongLoadingState GetLoadingState() const { return LoadingState; }
	UFUNCTION(BlueprintPure, Category = "Ongseong|Chongtong|Loading")
	int32 GetCompletedShots() const { return CompletedShots; }
	UFUNCTION(BlueprintPure, Category = "Ongseong|Chongtong|Loading")
	int32 GetRequiredRammerStrokes() const { return RequiredRammerStrokes; }
	UFUNCTION(BlueprintPure, Category = "Ongseong|Chongtong|Loading")
	int32 GetRequiredShotsToComplete() const { return RequiredShotsToComplete; }

	UFUNCTION(BlueprintPure, Category = "Ongseong|Chongtong")
	AActor* SelectTarget() const;

	/**
	 * Attack slots: how many enemies may close in on this emplacement at once.
	 * A full cannon is ignored by the next archer, which walks on to another one or to the ram.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ongseong|Chongtong|Slots")
	bool TryReserveAttackerSlot(AActor* Attacker);

	UFUNCTION(BlueprintCallable, Category = "Ongseong|Chongtong|Slots")
	void ReleaseAttackerSlot(AActor* Attacker);

	UFUNCTION(BlueprintPure, Category = "Ongseong|Chongtong|Slots")
	bool HasFreeAttackerSlot() const;

	UFUNCTION(BlueprintPure, Category = "Ongseong|Chongtong|Slots")
	int32 GetReservedAttackerCount() const;

	/** Spawns the configured allied operator and locks it to the cannon's seat. */
	UFUNCTION(BlueprintCallable, Category = "Ongseong|Chongtong|Operator")
	bool SpawnMountedOperator();

	UFUNCTION(BlueprintCallable, Category = "Ongseong|Chongtong|Operator")
	void RemoveMountedOperator();

	UFUNCTION(BlueprintPure, Category = "Ongseong|Chongtong|Operator")
	AAllyCombatCharacter* GetMountedOperator() const { return MountedOperator; }

	UPROPERTY(BlueprintAssignable, Category = "Ongseong|Chongtong")
	FOnChongtongFired OnFired;
	UPROPERTY(BlueprintAssignable, Category="Ongseong|Chongtong|Loading")
	FOnChongtongLoadingStateChanged OnLoadingStateChanged;
	UPROPERTY(BlueprintAssignable, Category="Ongseong|Chongtong|Loading")
	FOnChongtongRammingProgress OnRammingProgress;
	UPROPERTY(BlueprintAssignable, Category="Ongseong|Chongtong|Loading")
	FOnChongtongExperienceCompleted OnExperienceCompleted;

protected:
	void UpdateLoadingInteractions();
	void UpdateInteractionPrompts();
	bool IsItemRequiredNow(EChongtongLoadingItemType ItemType) const;
	void SetLoadingState(EChongtongLoadingState NewState);
	void UpdateStatusSignal();
	void EnterReadyStation();
	void ExitReadyStation();
	void SpawnPlaceholderLoadingItems();
	AGameplayProjectileActor* SpawnProjectile(const FVector& Direction, float Speed);
	/** Points the barrel so its muzzle axis follows WorldDirection, then returns that axis. */
	FVector AimBarrelAtDirection(const FVector& WorldDirection);
	/** Solves the launch velocity that drops a shell on TargetLocation under the shell's own gravity. */
	bool SolveFiringArc(const FVector& TargetLocation, FVector& OutLaunchVelocity) const;
	void PruneAttackerSlots() const;
	void PlayFeedback(UNiagaraSystem* Effect, USoundBase* Sound, const FVector& Location, float Pitch = 1.0f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	/** Hwacha carriage/support mesh. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> HwachaBaseMesh;

	/** Chongtong barrel mesh mounted on the carriage. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> BarrelPivot;

	/** Visual mesh only; its import correction scale must not scale interaction sockets. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ChongtongMesh;

	/** Attachment point for the friendly AI gunner. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> OperatorSeat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Muzzle;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> LoadingSocket;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> PlayerCameraAnchor;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UChongtongAimGripComponent> AimGrip;
	/** Shows the player where to put both hands once the cannon is loaded. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UInteractionHighlightComponent> AimPrompt;
	/** Optional ally AI firing behavior; configured per Blueprint variant. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UChongtongAutomaticFireComponent> AutomaticFire;
	/** Event-driven instructor narration for this experience. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UOngseongNarrationComponent> Narration;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTextRenderComponent> StatusText;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPointLightComponent> StatusLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCombatFactionComponent> FactionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCombatThreatComponent> ThreatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCombatTargetingComponent> TargetingComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong")
	TObjectPtr<AActor> GateTarget;

	/** Blueprint class for the allied gunner. Leave empty only when a level supplies one manually. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong|Operator")
	TSubclassOf<AAllyCombatCharacter> OperatorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong|Operator")
	bool bSpawnOperatorOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong|Operator")
	FTransform OperatorRelativeTransform;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ongseong|Chongtong|Operator")
	TObjectPtr<AAllyCombatCharacter> MountedOperator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong")
	TSubclassOf<AGameplayProjectileActor> ProjectileClass;

	/** Optional projectile pool. When unset, the cannon falls back to SpawnActor/Destroy. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong")
	TObjectPtr<AActorPool> ProjectilePool;

	/** Caps how many cannon effects can be audible at once on standalone hardware. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chongtong|Feedback")
	TObjectPtr<class USoundConcurrency> CombatSoundConcurrency;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong", meta = (ClampMin = "0.1"))
	float FireInterval = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong", meta = (ClampMin = "0.0"))
	float FireRange = 12000.0f;

	/**
	 * Automatic fire answers enemy archers and nothing else: the ram is the player's objective and
	 * ally emplacements must not clear it, and the melee line is left to the defenders on the wall.
	 * Targets are identified by their ranged-combat component, never by their Actor class.
	 * Player-aimed shots are unaffected - they hit whatever the barrel points at.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong")
	bool bEngageEnemyArchersOnly = true;

	/** Muzzle velocity for a player shot. Allied shots solve their own arc speed instead. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong", meta = (ClampMin = "0.0"))
	float ProjectileSpeed = 2800.0f;

	/** 0 = flattest arc that still reaches, 1 = highest lob. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FiringArc = 0.45f;

	/** Enemies allowed to engage this emplacement at once. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong|Slots", meta = (ClampMin = "0"))
	int32 MaxAttackerSlots = 2;

	/** Weak by design and deliberately not a UPROPERTY: a slot must never keep an attacker alive. */
	mutable TArray<TWeakObjectPtr<AActor>> AttackerSlots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong", meta = (ClampMin = "0.0"))
	float ProjectileDamage = 40.0f;

	/** Starts the projectile beyond the carriage/barrel collision envelope. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong", meta = (ClampMin = "0.0"))
	float ProjectileSpawnClearance = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Chongtong|Player")
	bool bEnableAutomaticFire = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Chongtong|Loading", meta=(ClampMin="1"))
	int32 RequiredRammerStrokes = 3;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Chongtong|Loading", meta=(ClampMin="1"))
	int32 RequiredShotsToComplete = 5;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Chongtong|Loading", meta=(ClampMin="1.0"))
	float LoadingAcceptanceRadius = 30.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Chongtong|Loading", meta=(ClampMin="1.0"))
	float RammerWithdrawRadius = 65.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Chongtong|Loading")
	bool bSpawnPlaceholderProps = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Chongtong|Loading")
	TSubclassOf<AChongtongLoadingItemActor> PowderItemClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Chongtong|Loading")
	TSubclassOf<AChongtongLoadingItemActor> RammerItemClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Chongtong|Loading")
	TSubclassOf<AChongtongLoadingItemActor> CannonballItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Chongtong|Feedback")
	TObjectPtr<UNiagaraSystem> LoadSuccessEffect;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Chongtong|Feedback")
	TObjectPtr<UNiagaraSystem> MuzzleEffect;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Chongtong|Feedback")
	TObjectPtr<USoundBase> InteractionSound;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Chongtong|Feedback")
	TObjectPtr<USoundBase> FireSound;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Ongseong|Chongtong|Loading")
	EChongtongLoadingState LoadingState = EChongtongLoadingState::NeedsPowder;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Ongseong|Chongtong|Loading")
	int32 CompletedRammerStrokes = 0;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Ongseong|Chongtong|Loading")
	int32 CompletedShots = 0;
	UPROPERTY(Transient)
	TArray<TObjectPtr<AChongtongLoadingItemActor>> LoadingItems;
	bool bRammerInserted = false;

	UFUNCTION()
	void HandleDeath(UHealthComponent* DeadHealthComponent, const FCombatDamageSpec& KillingDamage);
};
