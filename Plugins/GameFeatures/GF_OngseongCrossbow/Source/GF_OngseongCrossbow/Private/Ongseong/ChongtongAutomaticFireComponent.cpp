#include "Ongseong/ChongtongAutomaticFireComponent.h"

#include "Ongseong/ChongtongCannonActor.h"
#include "TimerManager.h"

UChongtongAutomaticFireComponent::UChongtongAutomaticFireComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UChongtongAutomaticFireComponent::BeginPlay()
{
	Super::BeginPlay();
	if (bAutomaticFireEnabled)
	{
		StartAutomaticFire();
	}
}

void UChongtongAutomaticFireComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopAutomaticFire();
	Super::EndPlay(EndPlayReason);
}

void UChongtongAutomaticFireComponent::ConfigureAutomaticFire(const bool bEnabled, const float InFireInterval)
{
	FireInterval = FMath::Max(0.1f, InFireInterval);
	bAutomaticFireEnabled = bEnabled;

	if (HasBegunPlay())
	{
		if (bAutomaticFireEnabled)
		{
			StartAutomaticFire();
		}
		else
		{
			StopAutomaticFire();
		}
	}
}

void UChongtongAutomaticFireComponent::StartAutomaticFire()
{
	if (!GetWorld() || !Cast<AChongtongCannonActor>(GetOwner()))
	{
		return;
	}

	bAutomaticFireEnabled = true;
	GetWorld()->GetTimerManager().SetTimer(
		FireTimerHandle,
		this,
		&UChongtongAutomaticFireComponent::FireScheduledShot,
		FMath::Max(0.1f, FireInterval + FMath::FRandRange(-FireIntervalJitter, FireIntervalJitter)),
		false);
}

void UChongtongAutomaticFireComponent::StopAutomaticFire()
{
	bAutomaticFireEnabled = false;
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
	}
}

void UChongtongAutomaticFireComponent::FireScheduledShot()
{
	if (AChongtongCannonActor* Cannon = Cast<AChongtongCannonActor>(GetOwner()))
	{
		Cannon->TryFire();
	}
	if (bAutomaticFireEnabled && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			FireTimerHandle,
			this,
			&UChongtongAutomaticFireComponent::FireScheduledShot,
			FMath::Max(0.1f, FireInterval + FMath::FRandRange(-FireIntervalJitter, FireIntervalJitter)),
			false);
	}
}
