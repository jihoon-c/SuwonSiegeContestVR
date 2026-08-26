#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "InteractionHighlightComponent.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UNiagaraComponent;
class UNiagaraSystem;
class UMeshComponent;

/**
 * Marks a part of an Actor as "interact with this now".
 *
 * Two layers of feedback, both optional:
 *  - an overlay material on the interaction meshes, so the object itself glows in its own shape
 *  - a looping Niagara system at this component, so the spot reads from across the battlement
 *
 * Core owns this because every experience needs the same affordance; it must never know which
 * Game Feature switched it on.
 */
UCLASS(ClassGroup = (VRInteraction), meta = (BlueprintSpawnableComponent))
class SUWONSIEGECONTESTVR_API UInteractionHighlightComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UInteractionHighlightComponent();

	UFUNCTION(BlueprintCallable, Category = "VR|Interaction|Highlight")
	void SetHighlightActive(bool bActive);

	UFUNCTION(BlueprintPure, Category = "VR|Interaction|Highlight")
	bool IsHighlightActive() const { return bHighlightActive; }

	/** Recolours a live highlight, for example amber for "hold" versus blue for "grab". */
	UFUNCTION(BlueprintCallable, Category = "VR|Interaction|Highlight")
	void SetHighlightColor(FLinearColor NewColor);

	/**
	 * Makes an active highlight breathe instead of glowing flat.
	 * A steady glow reads as part of the material at VR distances; a pulse reads as "look here".
	 */
	UFUNCTION(BlueprintCallable, Category = "VR|Interaction|Highlight")
	void SetHighlightPulse(bool bEnable, float PulsesPerSecond = 1.2f);

	UFUNCTION(BlueprintPure, Category = "VR|Interaction|Highlight")
	bool IsHighlightPulsing() const { return bPulseHighlight; }

	/** Replaces the looping Niagara system, or clears it when the caller wants overlay-only feedback. */
	void SetHighlightEffectAsset(TSoftObjectPtr<UNiagaraSystem> NewEffect);

	/** Base overlay brightness the pulse oscillates around. */
	void SetHighlightIntensity(float NewIntensity);

	/** Adds a mesh that is not found automatically, such as one created by a Blueprint child. */
	UFUNCTION(BlueprintCallable, Category = "VR|Interaction|Highlight")
	void AddHighlightMesh(UMeshComponent* MeshComponent);

	/**
	 * Sets up a highlight from native construction code.
	 * Grip points pass false: the glow belongs on the spot to hold, not on the whole machine.
	 */
	void ConfigureHighlight(bool bInHighlightOwnerMeshes, FLinearColor InHighlightColor);

	/** Colour used by grab prompts across the project. */
	static FLinearColor GetDefaultGrabColor() { return FLinearColor(0.05f, 0.45f, 1.0f); }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void CollectOwnerMeshes();
	void ApplyOverlayMaterial(bool bActive);
	void ApplyHighlightEffect(bool bActive);
	/** Tick only exists to drive the pulse, so it is switched off whenever the pulse is not running. */
	void UpdatePulseTickEnabled();
	void ApplyPulseIntensity(float Alpha);

	/** Soft so a missing art asset degrades to "no glow" instead of failing construction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Interaction|Highlight")
	TSoftObjectPtr<UMaterialInterface> HighlightMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Interaction|Highlight")
	TSoftObjectPtr<UNiagaraSystem> HighlightEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Interaction|Highlight")
	FLinearColor HighlightColor = FLinearColor(0.05f, 0.45f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Interaction|Highlight", meta = (ClampMin = "0.0"))
	float HighlightIntensity = 4.0f;

	/** Off by default: existing grab prompts must keep their steady glow. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Interaction|Highlight|Pulse")
	bool bPulseHighlight = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Interaction|Highlight|Pulse", meta = (ClampMin = "0.01"))
	float PulsesPerSecond = 1.2f;

	/** Intensity multiplier at the dim end of the pulse. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Interaction|Highlight|Pulse", meta = (ClampMin = "0.0"))
	float PulseMinIntensityScale = 0.15f;

	/** Intensity multiplier at the bright end of the pulse. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Interaction|Highlight|Pulse", meta = (ClampMin = "0.0"))
	float PulseMaxIntensityScale = 1.6f;

	/** Off for grip points, where the glow belongs on a marker rather than on the whole machine. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR|Interaction|Highlight")
	bool bHighlightOwnerMeshes = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Interaction|Highlight")
	FVector HighlightEffectScale = FVector(1.0f);

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMeshComponent>> HighlightMeshes;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> HighlightMaterialInstance;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> HighlightEffectComponent;

	bool bHighlightActive = false;
	float PulseTimeSeconds = 0.0f;
};
