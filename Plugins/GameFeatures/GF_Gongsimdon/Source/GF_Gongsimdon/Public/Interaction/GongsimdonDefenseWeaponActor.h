#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GongsimdonDefenseWeaponActor.generated.h"

class UMotionControllerComponent;
class USceneComponent;
class UScenarioInteractableComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnGongsimdonDefenseShot, bool, bHitEnemy, AActor*, HitActor);

/** Stationary VR trigger weapon used by the playable Gongsimdon combat step. */
UCLASS(Blueprintable)
class GF_GONGSIMDON_API AGongsimdonDefenseWeaponActor : public AActor
{
	GENERATED_BODY()

public:
	AGongsimdonDefenseWeaponActor();

	UFUNCTION(BlueprintCallable, Category = "Gongsimdon|Combat")
	bool HandleVRGrabbed(USceneComponent* GrabComponent, UMotionControllerComponent* MotionController);

	UFUNCTION(BlueprintCallable, Category = "Gongsimdon|Combat")
	bool FireFromTransform(FVector Origin, FVector Direction, AActor* InstigatorActor = nullptr);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gongsimdon|Combat",
		meta = (ClampMin = "100.0", Units = "cm"))
	float ShotRange = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gongsimdon|Combat",
		meta = (ClampMin = "0.0", Units = "cm"))
	float AimAssistRadius = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gongsimdon|Combat",
		meta = (ClampMin = "0.1"))
	float ShotDamage = 10.0f;

	UPROPERTY(BlueprintAssignable, Category = "Gongsimdon|Combat")
	FOnGongsimdonDefenseShot OnDefenseShot;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gongsimdon|Combat")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	/** Lets the common Scenario guide resolve this nearby physical weapon for the Combat step. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gongsimdon|Combat")
	TObjectPtr<UScenarioInteractableComponent> CombatGuideInteractor;
};
