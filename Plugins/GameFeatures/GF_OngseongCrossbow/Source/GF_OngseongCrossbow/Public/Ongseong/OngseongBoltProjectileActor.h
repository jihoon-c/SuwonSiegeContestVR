#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Combat/GameplayProjectileActor.h"
#include "OngseongBoltProjectileActor.generated.h"

class UInteractionHighlightComponent;
class UStaticMeshComponent;

/** Lightweight physical arrow fired by the Ongseong enemy archers. */
UCLASS(Blueprintable)
class GF_ONGSEONGCROSSBOW_API AOngseongBoltProjectileActor : public AGameplayProjectileActor
{
	GENERATED_BODY()

public:
	AOngseongBoltProjectileActor();

	virtual void BeginPlay() override;
	virtual void OnAcquiredFromPool_Implementation() override;
	virtual void OnReleasedToPool_Implementation() override;

protected:
	void ApplyVisibilityHighlight(bool bActive);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> BoltMesh;

	/**
	 * An arrow is a few centimetres of untextured geometry crossing the courtyard in a
	 * fraction of a second. Without a rim overlay players report that nothing was fired at all.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UInteractionHighlightComponent> VisibilityHighlight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Bolt|Visual")
	bool bShowVisibilityHighlight = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Bolt|Visual")
	FLinearColor VisibilityHighlightColor = FLinearColor(1.0f, 0.05f, 0.02f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Bolt|Visual")
	bool bPulseVisibilityHighlight = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Bolt|Visual", meta=(ClampMin="0.01"))
	float VisibilityHighlightPulsesPerSecond = 4.0f;

	/** The arrow art is authored small; scale it up here rather than in every Blueprint child. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Bolt|Visual")
	FVector BoltMeshScale = FVector(1.0f);

	/**
	 * Yaw/pitch correction so the arrow art points along the velocity axis.
	 * The collision root already follows velocity along +X; the arrow mesh is authored pointing
	 * along +Y, so it flew sideways. -90 yaw turns it left onto the travel axis.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Bolt|Visual")
	FRotator BoltMeshRotation = FRotator(0.0f, -90.0f, 0.0f);
};
