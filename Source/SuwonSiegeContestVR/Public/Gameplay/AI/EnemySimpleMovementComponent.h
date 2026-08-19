#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemySimpleMovementComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemySimpleMoveTargetReached, AActor*, EnemyActor);

/** Low-cost direct movement used only while an enemy is in the far AI LOD. */
UCLASS(ClassGroup = (Gameplay), meta = (BlueprintSpawnableComponent))
class SUWONSIEGECONTESTVR_API UEnemySimpleMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemySimpleMovementComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Gameplay|AI|Simple Movement")
	void SetSimpleMovementEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Gameplay|AI|Simple Movement")
	void SetMoveTargetActor(AActor* NewTargetActor);

	UFUNCTION(BlueprintCallable, Category = "Gameplay|AI|Simple Movement")
	void SetMoveTargetLocation(FVector NewTargetLocation);

	UFUNCTION(BlueprintCallable, Category = "Gameplay|AI|Simple Movement")
	void ClearMoveTarget();

	UPROPERTY(BlueprintAssignable, Category = "Gameplay|AI|Simple Movement")
	FOnEnemySimpleMoveTargetReached OnTargetReached;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay|AI|Simple Movement", meta = (ClampMin = "0.0"))
	float MoveSpeed = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay|AI|Simple Movement", meta = (ClampMin = "1.0"))
	float UpdateInterval = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay|AI|Simple Movement", meta = (ClampMin = "0.0"))
	float AcceptanceRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay|AI|Simple Movement")
	bool bSweepMovement = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gameplay|AI|Simple Movement")
	bool bSimpleMovementEnabled = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gameplay|AI|Simple Movement")
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gameplay|AI|Simple Movement")
	FVector TargetLocation = FVector::ZeroVector;

	bool bHasLocationTarget = false;
	bool bTargetReached = false;
};
