#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Ongseong/ChongtongInteractionTypes.h"
#include "ChongtongLoadingItemActor.generated.h"

class UInteractionHighlightComponent;
class UStaticMeshComponent;
class UStaticMesh;

/** Loading prop whose Blueprint-authored mesh and component transform are preserved. */
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
	/** Releases the prop from the hand and makes it kinematic for the cannon-driven ram animation. */
	void BeginAutomatedUse();

	/** Glows while this is the item the loading sequence is waiting for. */
	UFUNCTION(BlueprintCallable, Category="Ongseong|Chongtong|Loading")
	void SetLoadingPromptActive(bool bActive);
	UFUNCTION(BlueprintPure, Category="Ongseong|Chongtong|Loading")
	bool IsLoadingPromptActive() const;

protected:
	void ApplyNativePlaceholderAppearance();
	void RespawnAtHome();

	/**
	 * Edit this inherited component directly in a Blueprint child. Native placeholder
	 * appearance is only applied to instances of this exact C++ class, never Blueprint children.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Ongseong|Chongtong|Loading|Appearance")
	TObjectPtr<UStaticMeshComponent> Mesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ongseong|Chongtong|Loading")
	TObjectPtr<UInteractionHighlightComponent> LoadingPrompt;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ongseong|Chongtong|Loading|Appearance")
	TObjectPtr<UStaticMesh> PowderMesh;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ongseong|Chongtong|Loading|Appearance")
	TObjectPtr<UStaticMesh> RammerMesh;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ongseong|Chongtong|Loading|Appearance")
	TObjectPtr<UStaticMesh> CannonballMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Chongtong|Loading")
	EChongtongLoadingItemType ItemType = EChongtongLoadingItemType::Powder;
	FTransform HomeTransform;
	bool bHomeSimulatingPhysics = false;
};
