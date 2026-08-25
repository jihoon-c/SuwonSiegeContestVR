#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LevelFadeSubsystem.generated.h"

/** Common full-screen fades for game start and Experience-level travel. */
UCLASS()
class SUWONSIEGECONTESTVR_API ULevelFadeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Fades to black before invoking the supplied level-open callback. */
	void FadeOutThen(UWorld* World, TFunction<void()> OpenLevelCallback);

	UPROPERTY(EditAnywhere, Category = "Level Fade", meta = (ClampMin = "0.0", Units = "s"))
	float FadeInDuration = 0.75f;

	UPROPERTY(EditAnywhere, Category = "Level Fade", meta = (ClampMin = "0.0", Units = "s"))
	float FadeOutDuration = 0.5f;

private:
	void HandlePostLoadMap(UWorld* LoadedWorld);
	void FadeInLoadedWorld(TWeakObjectPtr<UWorld> LoadedWorld);
	void StartCameraFade(UWorld* World, float FromAlpha, float ToAlpha, float Duration) const;
};
