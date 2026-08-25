#include "Core/Experience/LevelFadeSubsystem.h"

#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void ULevelFadeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::HandlePostLoadMap);
}

void ULevelFadeSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
	Super::Deinitialize();
}

void ULevelFadeSubsystem::FadeOutThen(UWorld* World, TFunction<void()> OpenLevelCallback)
{
	if (!World || FadeOutDuration <= 0.0f)
	{
		OpenLevelCallback();
		return;
	}

	StartCameraFade(World, 0.0f, 1.0f, FadeOutDuration);
	FTimerHandle TravelTimer;
	World->GetTimerManager().SetTimer(
		TravelTimer,
		FTimerDelegate::CreateLambda([Callback = MoveTemp(OpenLevelCallback)]() mutable { Callback(); }),
		FadeOutDuration,
		false);
}

void ULevelFadeSubsystem::HandlePostLoadMap(UWorld* LoadedWorld)
{
	if (!LoadedWorld || !LoadedWorld->IsGameWorld())
	{
		return;
	}

	FTimerHandle FadeInTimer;
	LoadedWorld->GetTimerManager().SetTimer(
		FadeInTimer,
		FTimerDelegate::CreateUObject(this, &ThisClass::FadeInLoadedWorld, TWeakObjectPtr<UWorld>(LoadedWorld)),
		0.05f,
		false);
}

void ULevelFadeSubsystem::FadeInLoadedWorld(TWeakObjectPtr<UWorld> LoadedWorld)
{
	if (LoadedWorld.IsValid())
	{
		StartCameraFade(LoadedWorld.Get(), 1.0f, 0.0f, FadeInDuration);
	}
}

void ULevelFadeSubsystem::StartCameraFade(UWorld* World, float FromAlpha, float ToAlpha, float Duration) const
{
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0))
	{
		if (APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager)
		{
			CameraManager->StartCameraFade(FromAlpha, ToAlpha, Duration, FLinearColor::Black, false, false);
		}
	}
}
