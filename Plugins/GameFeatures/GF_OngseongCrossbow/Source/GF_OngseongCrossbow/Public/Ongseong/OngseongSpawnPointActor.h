#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OngseongSpawnPointActor.generated.h"

UENUM(BlueprintType)
enum class EOngseongSpawnPointRole : uint8
{
	EnemyInitial UMETA(DisplayName = "Enemy Initial Spawn"),
	SoldierRespawn UMETA(DisplayName = "Soldier Respawn"),
	Ram UMETA(DisplayName = "Battering Ram Spawn")
};

/**
 * Placeable authoring marker for the three spawn stages used by the ongseong defense.
 * Set SpawnPointRole per Blueprint instance; managers can reference the marker explicitly or
 * discover the first marker of the required role in the loaded level.
 */
UCLASS(Blueprintable)
class GF_ONGSEONGCROSSBOW_API AOngseongSpawnPointActor : public AActor
{
	GENERATED_BODY()

public:
	AOngseongSpawnPointActor();
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintPure, Category = "Ongseong|Spawning")
	EOngseongSpawnPointRole GetSpawnPointRole() const { return SpawnPointRole; }

	UFUNCTION(BlueprintPure, Category = "Ongseong|Spawning")
	FTransform GetSpawnTransform() const { return GetActorTransform(); }

	UFUNCTION(BlueprintPure, Category = "Ongseong|Spawning")
	bool MatchesRole(EOngseongSpawnPointRole ExpectedRole) const { return SpawnPointRole == ExpectedRole; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UArrowComponent> DirectionArrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UTextRenderComponent> RoleLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ongseong|Spawning")
	EOngseongSpawnPointRole SpawnPointRole = EOngseongSpawnPointRole::EnemyInitial;

private:
	void UpdateEditorPreview();
};
