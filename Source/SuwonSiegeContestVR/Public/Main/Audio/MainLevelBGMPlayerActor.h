#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MainLevelBGMPlayerActor.generated.h"

class UAudioComponent;
class USoundBase;

/**
 * Level-placeable 2D BGM player. Select the actor in the level to swap its
 * sound, volume, fade, and looping behaviour without editing gameplay code.
 */
UCLASS(Blueprintable)
class SUWONSIEGECONTESTVR_API AMainLevelBGMPlayerActor : public AActor
{
	GENERATED_BODY()

public:
	AMainLevelBGMPlayerActor();

	/** Starts the configured BGM. Safe to call again after StopBGM. */
	UFUNCTION(BlueprintCallable, Category = "Main BGM")
	void PlayBGM();

	/** Stops the BGM using Fade Out Duration. */
	UFUNCTION(BlueprintCallable, Category = "Main BGM")
	void StopBGM();

	/** Applies a new volume immediately, so it can also be driven by an options menu. */
	UFUNCTION(BlueprintCallable, Category = "Main BGM")
	void SetBGMVolume(float NewVolume);

	UFUNCTION(BlueprintPure, Category = "Main BGM")
	bool IsBGMPlaying() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Main BGM")
	TObjectPtr<UAudioComponent> BGMComponent;

	/** Sound Wave or Sound Cue to play. Change this directly in the placed actor's Details panel. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main BGM|Sound")
	TObjectPtr<USoundBase> Music;

	/** Enables automatic looping at the component level. Prefer a looping Sound Cue if the music needs a seamless loop. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main BGM|Sound")
	bool bLoop = true;

	/** Master volume for this level BGM. 1.0 is the source asset's original volume. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main BGM|Mix", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float Volume = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main BGM|Mix", meta = (ClampMin = "0.0", Units = "s"))
	float FadeInDuration = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main BGM|Mix", meta = (ClampMin = "0.0", Units = "s"))
	float FadeOutDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main BGM|Playback")
	bool bPlayOnBeginPlay = true;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleBGMFinished();

private:
	bool bStopRequested = false;
};
