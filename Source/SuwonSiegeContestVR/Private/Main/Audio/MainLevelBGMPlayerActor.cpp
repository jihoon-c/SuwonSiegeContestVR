#include "Main/Audio/MainLevelBGMPlayerActor.h"

#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

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

	if (!Music)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainLevelBGMPlayerActor %s has no Music asset assigned."), *GetName());
		return;
	}

	// A placed instance can retain an older, disabled bPlayOnBeginPlay value.
	// The Main BGM actor's contract is instead simple: whenever Music is assigned,
	// start it with the level. Starting next tick also avoids racing the VR audio
	// device during level initialization.
	InitialStartRetryCount = 0;
	GetWorldTimerManager().SetTimerForNextTick(this, &ThisClass::StartBGMWhenAudioReady);
}

void AMainLevelBGMPlayerActor::StartBGMWhenAudioReady()
{
	if (!Music || !BGMComponent)
	{
		return;
	}

	BGMComponent->Activate(true);
	PlayBGM();

	// On some VR startup paths the AudioComponent accepts the request before its
	// audio device is ready. Retry a few times only while it reports not playing.
	if (!BGMComponent->IsPlaying() && InitialStartRetryCount++ < 3)
	{
		GetWorldTimerManager().SetTimer(
			InitialStartRetryTimer,
			this,
			&ThisClass::StartBGMWhenAudioReady,
			0.25f,
			false);
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
