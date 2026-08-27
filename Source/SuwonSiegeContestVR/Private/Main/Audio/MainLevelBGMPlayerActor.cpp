#include "Main/Audio/MainLevelBGMPlayerActor.h"

#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

AMainLevelBGMPlayerActor::AMainLevelBGMPlayerActor()
{
	PrimaryActorTick.bCanEverTick = false;

	BGMComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BGMComponent"));
	SetRootComponent(BGMComponent);
	BGMComponent->bAutoActivate = false;
	BGMComponent->bIsUISound = true;
	BGMComponent->bAllowSpatialization = false;
	BGMComponent->SetVolumeMultiplier(Volume);
	BGMComponent->OnAudioFinished.AddDynamic(this, &ThisClass::HandleBGMFinished);
}

void AMainLevelBGMPlayerActor::BeginPlay()
{
	Super::BeginPlay();
	if (bPlayOnBeginPlay)
	{
		// Explicitly activate first: placed actors can have an auto-activation
		// override saved in the level, while BGM must always start with the level.
		BGMComponent->Activate(true);
		PlayBGM();
	}
}

void AMainLevelBGMPlayerActor::PlayBGM()
{
	if (!Music)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainLevelBGMPlayerActor %s has no Music asset assigned."), *GetName());
		return;
	}

	// Stop can broadcast OnAudioFinished immediately; suppress a stale loop while
	// replacing the source sound.
	bStopRequested = true;
	BGMComponent->Stop();
	BGMComponent->SetSound(Music);
	BGMComponent->SetVolumeMultiplier(FMath::Max(0.0f, Volume));
	bStopRequested = false;
	if (FadeInDuration > 0.0f)
	{
		// FadeIn's target is an additional multiplier. Using 1.0 here avoids
		// applying the editor Volume value twice (for example 0.35 x 0.35).
		BGMComponent->FadeIn(FadeInDuration, 1.0f);
	}
	else
	{
		BGMComponent->Play();
	}
}

void AMainLevelBGMPlayerActor::StopBGM()
{
	bStopRequested = true;
	if (FadeOutDuration > 0.0f)
	{
		BGMComponent->FadeOut(FadeOutDuration, 0.0f);
	}
	else
	{
		BGMComponent->Stop();
	}
}

void AMainLevelBGMPlayerActor::SetBGMVolume(float NewVolume)
{
	Volume = FMath::Max(0.0f, NewVolume);
	BGMComponent->SetVolumeMultiplier(Volume);
}

bool AMainLevelBGMPlayerActor::IsBGMPlaying() const
{
	return BGMComponent && BGMComponent->IsPlaying();
}

void AMainLevelBGMPlayerActor::HandleBGMFinished()
{
	if (!bStopRequested && bLoop && Music)
	{
		// Loop the source at its current volume. A Sound Cue may still provide
		// its own seamless loop; this fallback also supports a plain Sound Wave.
		BGMComponent->Play();
	}
}
