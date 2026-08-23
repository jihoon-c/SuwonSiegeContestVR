#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Ongseong/ChongtongInteractionTypes.h"
#include "ChongtongLoadingItemActor.generated.h"

class UStaticMeshComponent;
class UStaticMesh;

/** Placeholder loading prop. Meshes are Engine primitives and can be replaced in a Blueprint child. */
UCLASS(Blueprintable)
class GF_ONGSEONGCROSSBOW_API AChongtongLoadingItemActor : public AActor
{
	GENERATED_BODY()
public:
	AChongtongLoadingItemActor();
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable, Category="Ongseong|Chongtong|Loading")
	void ConfigureItem(EChongtongLoadingItemType NewType);
	UFUNCTION(BlueprintCallable, Category="Ongseong|Chongtong|Loading")
	void ConsumeAndRespawn(float DelaySeconds = 0.35f);
	UFUNCTION(BlueprintPure, Category="Ongseong|Chongtong|Loading")
	EChongtongLoadingItemType GetItemType() const { return ItemType; }
	UFUNCTION(BlueprintPure, Category="Ongseong|Chongtong|Loading")
	float GetDistanceToPoint(FVector WorldPoint) const;

protected:
	void ApplyPlaceholderAppearance();
	void RespawnAtHome();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> Mesh;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ongseong|Chongtong|Loading|Appearance")
	TObjectPtr<UStaticMesh> PowderMesh;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ongseong|Chongtong|Loading|Appearance")
	TObjectPtr<UStaticMesh> RammerMesh;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ongseong|Chongtong|Loading|Appearance")
	TObjectPtr<UStaticMesh> CannonballMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Chongtong|Loading")
	EChongtongLoadingItemType ItemType = EChongtongLoadingItemType::Powder;
	FTransform HomeTransform;
};
