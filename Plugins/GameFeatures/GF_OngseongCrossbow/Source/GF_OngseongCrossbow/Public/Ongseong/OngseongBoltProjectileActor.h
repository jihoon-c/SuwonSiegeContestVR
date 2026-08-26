#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Combat/GameplayProjectileActor.h"
#include "OngseongBoltProjectileActor.generated.h"

class UInteractionHighlightComponent;
class USoundBase;
class USphereComponent;
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

	UFUNCTION()
	void HandleFlybyOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> BoltMesh;

	/**
	 * Wider, overlap-only sensor around the arrow so a near miss (not a direct hit) can still
	 * trigger a whoosh cue. Query-only -- never affects the arrow's flight or damage collision.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USphereComponent> FlybySensor;

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

	/** One-shot whoosh played when the arrow passes near the player without hitting them.
	 * Set a Concurrency limit on this Sound Cue/MetaSound (e.g. max 4 voices) and a short
	 * Attenuation range -- several arrows can be in flight at once on standalone VR. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Bolt|Audio")
	TObjectPtr<USoundBase> FlybyWhooshSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Bolt|Audio", meta=(ClampMin="0.0", ClampMax="2.0"))
	float FlybyWhooshVolume = 0.8f;

	/** Radius of the overlap-only proximity sensor, separate from the small hit-collision sphere. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Bolt|Audio", meta=(ClampMin="0.0"))
	float FlybySensorRadius = 180.0f;

	bool bFlybyTriggered = false;
};
