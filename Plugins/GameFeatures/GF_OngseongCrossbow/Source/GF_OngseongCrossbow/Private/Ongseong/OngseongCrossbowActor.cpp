#include "Ongseong/OngseongCrossbowActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Gameplay/Combat/CombatTypes.h"
#include "Gameplay/Combat/CombatFactionComponent.h"
#include "Gameplay/Combat/GameplayProjectileActor.h"
#include "Gameplay/Pooling/ActorPool.h"
#include "Gameplay/UI/VRHUDComponent.h"
#include "Ongseong/OngseongBoltProjectileActor.h"
#include "Ongseong/OngseongCrossbowGripComponent.h"
#include "TimerManager.h"

AOngseongCrossbowActor::AOngseongCrossbowActor()
{
	PrimaryActorTick.bCanEverTick = false;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(Root);
	AimPivot = CreateDefaultSubobject<USceneComponent>(TEXT("AimPivot"));
	AimPivot->SetupAttachment(BaseMesh);
	AimPivot->SetRelativeLocation(FVector(0.0f, 0.0f, 110.0f));
	CrossbowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrossbowMesh"));
	CrossbowMesh->SetupAttachment(AimPivot);
	Muzzle = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle"));
	Muzzle->SetupAttachment(AimPivot);
	Muzzle->SetRelativeLocation(FVector(140.0f, 0.0f, 0.0f));
	Grip = CreateDefaultSubobject<UOngseongCrossbowGripComponent>(TEXT("Grip"));
	Grip->SetupAttachment(AimPivot);
	Grip->SetRelativeLocation(FVector(-35.0f, 0.0f, -20.0f));
	Grip->SetAimTarget(AimPivot);
	FactionComponent = CreateDefaultSubobject<UCombatFactionComponent>(TEXT("FactionComponent"));
	FactionComponent->SetFaction(ECombatFaction::Player);
	BoltClass = AOngseongBoltProjectileActor::StaticClass();
}

void AOngseongCrossbowActor::BeginPlay()
{
	Super::BeginPlay();
	RemainingAmmo = MaxAmmo == 0 ? 0 : FMath::Clamp(InitialAmmo, 0, MaxAmmo);
	SetCrossbowState(MaxAmmo == 0 || RemainingAmmo > 0 ? EOngseongCrossbowState::Loaded : EOngseongCrossbowState::Empty);
}

void AOngseongCrossbowActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(ReloadTimerHandle);
	Super::EndPlay(EndPlayReason);
}

bool AOngseongCrossbowActor::TryFire()
{
	if (CrossbowState != EOngseongCrossbowState::Loaded || !Grip->IsTwoHandAiming()) return false;
	const FVector Direction = Muzzle->GetForwardVector().GetSafeNormal();
	AGameplayProjectileActor* Projectile = SpawnBolt(Direction);
	if (!Projectile) return false;

	if (MaxAmmo > 0) --RemainingAmmo;
	OnFired.Broadcast(Projectile, RemainingAmmo);
	if (RemainingAmmo <= 0 && MaxAmmo > 0)
	{
		SetCrossbowState(EOngseongCrossbowState::Empty);
		if (UVRHUDComponent* HUD = ResolveHUD()) HUD->ShowPrompt(FText::FromString(TEXT("쇠뇌 화살을 보충하십시오")));
	}
	else
	{
		BeginReload();
	}
	return true;
}

bool AOngseongCrossbowActor::BeginReload()
{
	if (CrossbowState == EOngseongCrossbowState::Reloading || (MaxAmmo > 0 && RemainingAmmo <= 0)) return false;
	SetCrossbowState(EOngseongCrossbowState::Reloading);
	if (UVRHUDComponent* HUD = ResolveHUD()) HUD->ShowPrompt(FText::FromString(TEXT("쇠뇌 재장전 중")));
	if (ReloadDuration <= 0.0f) CompleteReload();
	else GetWorldTimerManager().SetTimer(ReloadTimerHandle, this, &AOngseongCrossbowActor::CompleteReload, ReloadDuration, false);
	return true;
}

void AOngseongCrossbowActor::CompleteReload()
{
	GetWorldTimerManager().ClearTimer(ReloadTimerHandle);
	if (MaxAmmo > 0 && RemainingAmmo <= 0)
	{
		SetCrossbowState(EOngseongCrossbowState::Empty);
		return;
	}
	SetCrossbowState(EOngseongCrossbowState::Loaded);
	if (UVRHUDComponent* HUD = ResolveHUD()) HUD->ShowPrompt(FText::FromString(TEXT("양손으로 조준하고 방아쇠를 당기십시오")));
}

void AOngseongCrossbowActor::RefillAmmo(const int32 Amount)
{
	if (MaxAmmo == 0) return;
	RemainingAmmo = FMath::Clamp(RemainingAmmo + FMath::Max(0, Amount), 0, MaxAmmo);
	if (CrossbowState == EOngseongCrossbowState::Empty && RemainingAmmo > 0) BeginReload();
}

void AOngseongCrossbowActor::HandleGripStateChanged(const bool bTwoHandAiming)
{
	if (UVRHUDComponent* HUD = ResolveHUD())
	{
		if (!bTwoHandAiming) HUD->ClearPrompt();
		else if (CrossbowState == EOngseongCrossbowState::Loaded) HUD->ShowPrompt(FText::FromString(TEXT("쇠뇌를 조준하고 방아쇠를 당기십시오")));
	}
}

void AOngseongCrossbowActor::SetCrossbowState(const EOngseongCrossbowState NewState)
{
	if (CrossbowState == NewState) return;
	CrossbowState = NewState;
	OnStateChanged.Broadcast(NewState);
}

AGameplayProjectileActor* AOngseongCrossbowActor::SpawnBolt(const FVector& Direction)
{
	if (!BoltClass || !GetWorld() || Direction.IsNearlyZero()) return nullptr;
	const FVector SpawnLocation = Muzzle->GetComponentLocation() + Direction * ProjectileSpawnClearance;
	AGameplayProjectileActor* Projectile = nullptr;
	if (BoltPool)
	{
		AActor* Acquired = BoltPool->AcquireActor(FTransform(Direction.Rotation(), SpawnLocation));
		Projectile = Cast<AGameplayProjectileActor>(Acquired);
		if (!Projectile && Acquired) BoltPool->ReleaseActor(Acquired);
	}
	else
	{
		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Projectile = GetWorld()->SpawnActor<AGameplayProjectileActor>(BoltClass, SpawnLocation, Direction.Rotation(), Params);
	}
	if (!Projectile) return nullptr;
	FCombatDamageSpec Spec;
	Spec.Amount = BoltDamage;
	Spec.InstigatorActor = this;
	Spec.DamageCauser = Projectile;
	Projectile->LaunchProjectile(Direction, BoltSpeed, Spec);
	return Projectile;
}

UVRHUDComponent* AOngseongCrossbowActor::ResolveHUD()
{
	if (VRHUD) return VRHUD;
	if (APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
	{
		if (APawn* Pawn = PC->GetPawn()) VRHUD = Pawn->FindComponentByClass<UVRHUDComponent>();
	}
	return VRHUD;
}
