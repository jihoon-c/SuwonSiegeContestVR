#include "Ongseong/ChongtongCannonActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/WidgetComponent.h"
#include "Core/VR/InteractionHighlightComponent.h"
#include "Gameplay/Characters/AllyCombatCharacter.h"
#include "Gameplay/Combat/CombatFactionComponent.h"
#include "Gameplay/Combat/HealthComponent.h"
#include "Gameplay/Combat/CombatTargetingComponent.h"
#include "Gameplay/Combat/CombatThreatComponent.h"
#include "Gameplay/Combat/CombatTypes.h"
#include "Gameplay/Combat/GameplayProjectileActor.h"
#include "Gameplay/Pooling/ActorPool.h"
#include "Ongseong/ChongtongProjectileActor.h"
#include "Ongseong/ChongtongAimGripComponent.h"
#include "Ongseong/ChongtongAutomaticFireComponent.h"
#include "Ongseong/ChongtongLoadingItemActor.h"
#include "Ongseong/ChongtongChargeWidget.h"
#include "Ongseong/OngseongNarrationComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GF_OngseongCrossbow.h"
#include "Gameplay/Combat/CombatFXLibrary.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundAttenuation.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AChongtongCannonActor::AChongtongCannonActor()
{
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	HwachaBaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HwachaBaseMesh"));
	HwachaBaseMesh->SetupAttachment(Root);
	BarrelPivot = CreateDefaultSubobject<USceneComponent>(TEXT("BarrelPivot"));
	BarrelPivot->SetupAttachment(HwachaBaseMesh);
	BarrelPivot->SetRelativeLocation(FVector(610.0f, 150.0f, 100.0f));
	ChongtongMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChongtongMesh"));
	ChongtongMesh->SetupAttachment(BarrelPivot);
	ChongtongMesh->SetRelativeScale3D(FVector(100.0f));
	OperatorSeat = CreateDefaultSubobject<USceneComponent>(TEXT("OperatorSeat"));
	OperatorSeat->SetupAttachment(HwachaBaseMesh);
	Muzzle = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle"));
	Muzzle->SetupAttachment(BarrelPivot);
	Muzzle->SetRelativeLocation(FVector(0.0f, 1345.0f, 0.0f));
	FireDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("FireDirection"));
	FireDirection->SetupAttachment(Muzzle);
	// Existing art points down local +Y. Arrow +X is the explicit, editor-tunable contract.
	FireDirection->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	FireDirection->ArrowSize = 2.0f;
	LoadingSocket = CreateDefaultSubobject<USceneComponent>(TEXT("LoadingSocket"));
	LoadingSocket->SetupAttachment(Muzzle);
	PlayerCameraAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("PlayerCameraAnchor"));
	PlayerCameraAnchor->SetupAttachment(HwachaBaseMesh);
	PlayerCameraAnchor->SetRelativeLocation(FVector(-115.0f, -45.0f, 165.0f));
	PlayerCameraAnchor->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	LoadingAcceptancePoint = CreateDefaultSubobject<USceneComponent>(TEXT("LoadingAcceptancePoint"));
	LoadingAcceptancePoint->SetupAttachment(PlayerCameraAnchor);
	// The player can only reach the rear of the carriage.  Keep the actual loading socket at
	// the muzzle for the rammer animation, but accept hand-held props at this rear-side point.
	LoadingAcceptancePoint->SetRelativeLocation(FVector(140.0f, 0.0f, -55.0f));
	ChargeDisplay = CreateDefaultSubobject<UWidgetComponent>(TEXT("ChargeDisplay"));
	ChargeDisplay->SetupAttachment(HwachaBaseMesh);
	ChargeDisplay->SetRelativeLocation(FVector(-115.0f, -70.0f, 145.0f));
	ChargeDisplay->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	ChargeDisplay->SetRelativeScale3D(FVector(0.12f));
	ChargeDisplay->SetWidgetSpace(EWidgetSpace::World);
	ChargeDisplay->SetDrawSize(FVector2D(480.0f, 110.0f));
	ChargeDisplay->SetPivot(FVector2D(0.5f, 0.5f));
	ChargeDisplay->SetBlendMode(EWidgetBlendMode::Transparent);
	ChargeDisplay->SetTwoSided(true);
	ChargeDisplay->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ChargeDisplay->SetWidgetClass(UChongtongChargeWidget::StaticClass());
	ChargeDisplay->SetVisibility(false);
	PowderSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("PowderSpawnPoint"));
	PowderSpawnPoint->SetupAttachment(PlayerCameraAnchor);
	PowderSpawnPoint->SetRelativeLocation(FVector(65.0f, -38.0f, -65.0f));
	RammerSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("RammerSpawnPoint"));
	RammerSpawnPoint->SetupAttachment(PlayerCameraAnchor);
	RammerSpawnPoint->SetRelativeLocation(FVector(65.0f, 0.0f, -65.0f));
	CannonballSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("CannonballSpawnPoint"));
	CannonballSpawnPoint->SetupAttachment(PlayerCameraAnchor);
	CannonballSpawnPoint->SetRelativeLocation(FVector(65.0f, 38.0f, -65.0f));
	AimGrip = CreateDefaultSubobject<UChongtongAimGripComponent>(TEXT("AimGrip"));
	AimGrip->SetupAttachment(HwachaBaseMesh);
	AimGrip->SetRelativeLocation(FVector(-40.0f, 0.0f, 120.0f));
	AimGrip->ComponentTags.Add(TEXT("VRGrab"));
	AimGrip->SetAimTarget(BarrelPivot);
	AimPrompt = CreateDefaultSubobject<UInteractionHighlightComponent>(TEXT("AimPrompt"));
	AimPrompt->SetupAttachment(AimGrip);
	// Amber, and only on the grip marker: glowing the whole carriage would hide the barrel.
	AimPrompt->ConfigureHighlight(false, FLinearColor(1.0f, 0.55f, 0.05f));
	AutomaticFire = CreateDefaultSubobject<UChongtongAutomaticFireComponent>(TEXT("AutomaticFire"));
	Narration = CreateDefaultSubobject<UOngseongNarrationComponent>(TEXT("OngseongNarration"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> HwachaBaseAsset(TEXT("/GF_OngseongCrossbow/Art/Namhansanseong/Chongtong/Hwacha/SM_Chongtong_Hwacha.SM_Chongtong_Hwacha"));
	if (HwachaBaseAsset.Succeeded())
	{
		HwachaBaseMesh->SetStaticMesh(HwachaBaseAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ChongtongAsset(TEXT("/GF_OngseongCrossbow/Art/Namhansanseong/Chongtong/FourChongtong/SM_Four_Chongtong_Gun.SM_Four_Chongtong_Gun"));
	if (ChongtongAsset.Succeeded())
	{
		ChongtongMesh->SetStaticMesh(ChongtongAsset.Object);
	}

	static ConstructorHelpers::FClassFinder<AAllyCombatCharacter> OperatorBlueprintClass(TEXT("/GF_OngseongCrossbow/Blueprints/BP_ChongtongOperator"));
	if (OperatorBlueprintClass.Succeeded())
	{
		OperatorClass = OperatorBlueprintClass.Class;
	}
	FactionComponent = CreateDefaultSubobject<UCombatFactionComponent>(TEXT("FactionComponent"));
	FactionComponent->SetFaction(ECombatFaction::Ally);
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	ThreatComponent = CreateDefaultSubobject<UCombatThreatComponent>(TEXT("ThreatComponent"));
	TargetingComponent = CreateDefaultSubobject<UCombatTargetingComponent>(TEXT("TargetingComponent"));
	CombatSoundAttenuation = CreateDefaultSubobject<USoundAttenuation>(TEXT("CombatSoundAttenuation"));
	CombatSoundAttenuation->Attenuation.bAttenuate = true;
	CombatSoundAttenuation->Attenuation.AttenuationShape = EAttenuationShape::Sphere;
	CombatSoundAttenuation->Attenuation.AttenuationShapeExtents = FVector(400.0f);
	CombatSoundAttenuation->Attenuation.FalloffDistance = 12000.0f;
	ProjectileClass = AChongtongProjectileActor::StaticClass();
	bSpawnOperatorOnBeginPlay = false;

	// A muzzle flash, not a generic explosion: the shell is what explodes, at the far end.
	static ConstructorHelpers::FObjectFinder<UParticleSystem> FireFX(TEXT("/Game/StarterContent/Particles/P_Explosion.P_Explosion"));
	static ConstructorHelpers::FObjectFinder<USoundBase> TempFireSound(TEXT("/Game/XRFramework/Audio/Fire_Cue.Fire_Cue"));
	MuzzleEffect = FireFX.Object;
	InteractionSound = TempFireSound.Object;
	FireSound = TempFireSound.Object;
}

void AChongtongCannonActor::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogOngseong, Verbose, TEXT("%s ready: actor %s, muzzle %s."),
		*GetName(), *GetActorLocation().ToCompactString(),
		Muzzle ? *Muzzle->GetComponentLocation().ToCompactString() : TEXT("none"));
	HealthComponent->OnDeath.AddUniqueDynamic(this, &AChongtongCannonActor::HandleDeath);
	if (bSpawnOperatorOnBeginPlay)
	{
		SpawnMountedOperator();
	}
	AutomaticFire->ConfigureAutomaticFire(bEnableAutomaticFire, FireInterval);
	// The player starts combat-ready after the narration; physical powder/rammer/ball props are
	// intentionally reserved for non-player variants only.
	if (bSpawnPlaceholderProps && !IsPlayerOperable())
	{
		SpawnPlaceholderLoadingItems();
	}
}

void AChongtongCannonActor::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateAutomatedRamming(DeltaSeconds);
	UpdateLoadingInteractions();
}

void AChongtongCannonActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveMountedOperator();
	Super::EndPlay(EndPlayReason);
}

bool AChongtongCannonActor::ReceiveCombatDamage_Implementation(const FCombatDamageSpec& DamageSpec)
{
	return HealthComponent && HealthComponent->ApplyDamage(DamageSpec);
}

void AChongtongCannonActor::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	// The actor origin sits on the battlement the cannon is mounted on, so a visibility trace from
	// there hits that mesh a few centimetres out and every target reads as blocked. The barrel is
	// also where shells actually leave from, so it is the honest origin for "can I hit this?".
	if (Muzzle)
	{
		OutLocation = Muzzle->GetComponentLocation();
		OutRotation = Muzzle->GetComponentRotation();
		return;
	}
	Super::GetActorEyesViewPoint(OutLocation, OutRotation);
}

void AChongtongCannonActor::SetGateTarget(AActor* NewGateTarget)
{
	GateTarget = NewGateTarget;
}

bool AChongtongCannonActor::TryFire()
{
	AActor* Target = SelectTarget();
	if (!IsValid(Target) || !ProjectileClass)
	{
		UE_LOG(LogOngseong, Verbose, TEXT("%s has no target to fire at."), *GetName());
		return false;
	}
	UE_LOG(LogOngseong, Verbose, TEXT("%s firing at %s (%.0f cm)."),
		*GetName(), *Target->GetName(), FVector::Dist(GetActorLocation(), Target->GetActorLocation()));

	FVector TargetLocation;
	FVector TargetExtent;
	Target->GetActorBounds(true, TargetLocation, TargetExtent);
	// Turn the authored assembly before solving the shot because rotating the carriage also moves
	// its attached muzzle. Only yaw changes; the barrel and hwacha keep their Blueprint offsets.
	AimAssemblyYawAtDirection(TargetLocation - HwachaBaseMesh->GetComponentLocation());

	FVector LaunchVelocity;
	const bool bSolved = SolveFiringArc(TargetLocation, LaunchVelocity);
	if (!bSolved)
	{
		UE_LOG(LogOngseong, Verbose, TEXT("%s cannot reach %s with an arc."), *GetName(), *Target->GetName());
		return false;
	}

	// The visual assembly turns horizontally only. The shell still uses the solved vertical
	// component so its existing ballistic arc remains intact.
	AGameplayProjectileActor* Projectile = SpawnProjectile(LaunchVelocity.GetSafeNormal(), LaunchVelocity.Size());
	if (!Projectile)
	{
		return false;
	}
	PlayFeedback(MuzzleEffect, FireSound, Muzzle->GetComponentLocation(), 1.0f, FVector(MuzzleEffectScale));
	OnFired.Broadcast(Target, Projectile);
	return true;
}

void AChongtongCannonActor::AimAssemblyYawAtDirection(const FVector& WorldDirection)
{
	if (!HwachaBaseMesh || !Muzzle)
	{
		return;
	}

	FVector DesiredHorizontal = WorldDirection;
	DesiredHorizontal.Z = 0.0f;
	FVector CurrentHorizontal = Muzzle->GetComponentTransform().GetUnitAxis(EAxis::Y);
	CurrentHorizontal.Z = 0.0f;
	if (!DesiredHorizontal.Normalize() || !CurrentHorizontal.Normalize())
	{
		return;
	}

	const float DeltaYaw = FMath::FindDeltaAngleDegrees(
		CurrentHorizontal.Rotation().Yaw,
		DesiredHorizontal.Rotation().Yaw - 90.0f);
	FRotator AssemblyRotation = HwachaBaseMesh->GetComponentRotation();
	AssemblyRotation.Yaw += DeltaYaw;
	HwachaBaseMesh->SetWorldRotation(AssemblyRotation);
}

bool AChongtongCannonActor::SolveFiringArc(const FVector& TargetLocation, FVector& OutLaunchVelocity) const
{
	if (!GetWorld() || !Muzzle)
	{
		return false;
	}
	float GravityScale = 1.0f;
	if (ProjectileClass)
	{
		if (const AGameplayProjectileActor* ProjectileDefaults = ProjectileClass->GetDefaultObject<AGameplayProjectileActor>())
		{
			GravityScale = ProjectileDefaults->GetProjectileGravityScale();
		}
	}
	// The solve has to use the gravity the shell will actually fall under, not world gravity.
	const float GravityZ = GetWorld()->GetGravityZ() * GravityScale;
	return UGameplayStatics::SuggestProjectileVelocity_CustomArc(
		this, OutLaunchVelocity, Muzzle->GetComponentLocation(), TargetLocation, GravityZ, FiringArc);
}

AGameplayProjectileActor* AChongtongCannonActor::SpawnProjectile(const FVector& Direction, const float Speed)
{
	if (!ProjectileClass || Direction.IsNearlyZero()) return nullptr;
	const FVector MuzzleLocation = Muzzle->GetComponentLocation() + Direction.GetSafeNormal() * ProjectileSpawnClearance;
	AGameplayProjectileActor* Projectile = nullptr;
	if (ProjectilePool)
	{
		AActor* AcquiredActor = ProjectilePool->AcquireActor(FTransform(Direction.Rotation(), MuzzleLocation));
		Projectile = Cast<AGameplayProjectileActor>(AcquiredActor);
		if (!Projectile && AcquiredActor) ProjectilePool->ReleaseActor(AcquiredActor);
	}
	else
	{
		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Projectile = GetWorld()->SpawnActor<AGameplayProjectileActor>(ProjectileClass, MuzzleLocation, Direction.Rotation(), Params);
	}
	if (!Projectile) return nullptr;
	FCombatDamageSpec Spec;
	Spec.Amount = ProjectileDamage;
	Spec.InstigatorActor = this;
	Spec.DamageCauser = Projectile;
	Projectile->LaunchProjectile(Direction, Speed, Spec);
	if (ProjectileFlightSound)
	{
		UGameplayStatics::SpawnSoundAttached(ProjectileFlightSound, Projectile->GetRootComponent(), NAME_None,
			FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset,
			true, 1.0f, 1.0f, 0.0f, CombatSoundAttenuation, CombatSoundConcurrency, true);
	}
	return Projectile;
}

bool AChongtongCannonActor::TryFirePlayer()
{
	return TryFirePlayerCharged(1.0f);
}

bool AChongtongCannonActor::TryFirePlayerCharged(const float ChargeAlpha)
{
	if (LoadingState != EChongtongLoadingState::ReadyToAim || !AimGrip->IsTwoHandAiming()) return false;
	const FVector Direction = FireDirection ? FireDirection->GetForwardVector().GetSafeNormal()
		: Muzzle->GetComponentTransform().GetUnitAxis(EAxis::Y).GetSafeNormal();
	const float Speed = FMath::Lerp(MinimumPlayerProjectileSpeed, ProjectileSpeed,
		FMath::Clamp(ChargeAlpha, 0.0f, 1.0f));
	AGameplayProjectileActor* Projectile = SpawnProjectile(Direction, Speed);
	if (!Projectile) return false;

	++CompletedShots;
	PlayFeedback(MuzzleEffect, FireSound, Muzzle->GetComponentLocation(), 1.0f, FVector(MuzzleEffectScale));
	OnFired.Broadcast(nullptr, Projectile);
	SetPlayerChargeVisible(false);
	SetPlayerChargePercent(0.0f);
	if (CompletedShots >= RequiredShotsToComplete)
	{
		SetLoadingState(EChongtongLoadingState::Completed);
		OnExperienceCompleted.Broadcast(CompletedShots);
	}
	else
	{
		CompletedRammerStrokes = 0;
		bRammerInserted = false;
		SetLoadingState(EChongtongLoadingState::NeedsPowder);
	}
	return true;
}

void AChongtongCannonActor::SetPlayerChargeVisible(const bool bVisible)
{
	if (ChargeDisplay)
	{
		ChargeDisplay->SetVisibility(bVisible);
		ChargeDisplay->SetHiddenInGame(!bVisible);
	}
}

void AChongtongCannonActor::SetPlayerChargePercent(const float ChargeAlpha)
{
	if (!ChargeDisplay)
	{
		return;
	}
	ChargeDisplay->InitWidget();
	if (UChongtongChargeWidget* Widget = Cast<UChongtongChargeWidget>(ChargeDisplay->GetUserWidgetObject()))
	{
		Widget->SetChargePercent(ChargeAlpha);
	}
}

void AChongtongCannonActor::BeginPlayerAim()
{
	// Keep the player in place; the earlier mounted interaction unexpectedly relocated them.
}

void AChongtongCannonActor::EndPlayerAim()
{
	SetPlayerChargeVisible(false);
	SetPlayerChargePercent(0.0f);
}

void AChongtongCannonActor::PrepareForImmediatePlayerFire()
{
	if (!IsPlayerOperable() || LoadingState == EChongtongLoadingState::ReadyToAim ||
		LoadingState == EChongtongLoadingState::Completed)
	{
		return;
	}

	CompletedRammerStrokes = 0;
	SetLoadingState(EChongtongLoadingState::ReadyToAim);
}

bool AChongtongCannonActor::TryLoadItem(const EChongtongLoadingItemType ItemType)
{
	if (ItemType == EChongtongLoadingItemType::Powder && LoadingState == EChongtongLoadingState::NeedsPowder)
	{
		SetLoadingState(EChongtongLoadingState::NeedsRamming);
		return true;
	}
	if (ItemType == EChongtongLoadingItemType::Cannonball && LoadingState == EChongtongLoadingState::NeedsCannonball)
	{
		SetLoadingState(EChongtongLoadingState::ReadyToAim);
		return true;
	}
	return false;
}

bool AChongtongCannonActor::RegisterRammerStroke()
{
	if (LoadingState != EChongtongLoadingState::NeedsRamming) return false;
	CompletedRammerStrokes = FMath::Min(CompletedRammerStrokes + 1, RequiredRammerStrokes);
	OnRammingProgress.Broadcast(CompletedRammerStrokes, RequiredRammerStrokes);
	if (CompletedRammerStrokes >= RequiredRammerStrokes) SetLoadingState(EChongtongLoadingState::NeedsCannonball);
	return true;
}

void AChongtongCannonActor::UpdateLoadingInteractions()
{
	UpdateInteractionPrompts();
	if (IsPlayerOperable()) return;
	if (!LoadingSocket || LoadingState == EChongtongLoadingState::ReadyToAim || LoadingState == EChongtongLoadingState::Completed) return;
	for (AChongtongLoadingItemActor* Item : LoadingItems)
	{
		if (!IsValid(Item) || Item->IsHidden() || !Item->IsHeldForInteraction()) continue;
		const USceneComponent* AcceptancePoint = LoadingAcceptancePoint ? LoadingAcceptancePoint : LoadingSocket;
		const float Distance = FVector::Distance(Item->GetInteractionLocation(), AcceptancePoint->GetComponentLocation());
		if (Item->GetItemType() == EChongtongLoadingItemType::Rammer)
		{
			if (LoadingState == EChongtongLoadingState::NeedsRamming && Distance <= LoadingAcceptanceRadius && !AnimatedRammer)
			{
				BeginAutomatedRamming(Item);
			}
		}
		else if (Distance <= LoadingAcceptanceRadius && TryLoadItem(Item->GetItemType()))
		{
			Item->ConsumeAndRespawn();
		}
	}
}

void AChongtongCannonActor::BeginAutomatedRamming(AChongtongLoadingItemActor* Rammer)
{
	if (!IsValid(Rammer) || AnimatedRammer || LoadingState != EChongtongLoadingState::NeedsRamming)
	{
		return;
	}
	AnimatedRammer = Rammer;
	RammerAnimationElapsed = 0.0f;
	AnimatedRammerCompletedStrokes = 0;
	Rammer->SetLoadingPromptActive(false);
	Rammer->BeginAutomatedUse();
	UpdateAutomatedRamming(0.0f);
}

void AChongtongCannonActor::UpdateAutomatedRamming(const float DeltaSeconds)
{
	if (!IsValid(AnimatedRammer) || !LoadingSocket)
	{
		AnimatedRammer = nullptr;
		return;
	}

	const float StrokeDuration = FMath::Max(0.1f, RammerStrokeDuration);
	RammerAnimationElapsed += FMath::Max(0.0f, DeltaSeconds);
	const int32 FinishedStrokes = FMath::Min(FMath::FloorToInt(RammerAnimationElapsed / StrokeDuration), RequiredRammerStrokes);
	while (AnimatedRammerCompletedStrokes < FinishedStrokes)
	{
		++AnimatedRammerCompletedStrokes;
		RegisterRammerStroke();
	}

	const FVector Forward = FireDirection ? FireDirection->GetForwardVector().GetSafeNormal()
		: Muzzle->GetComponentTransform().GetUnitAxis(EAxis::Y).GetSafeNormal();
	const FVector OuterLocation = LoadingSocket->GetComponentLocation() + Forward * RammerStrokeWithdrawDistance;
	const FVector InnerLocation = LoadingSocket->GetComponentLocation() - Forward * RammerStrokeInsertDepth;
	const float StrokePhase = FMath::Fmod(RammerAnimationElapsed, StrokeDuration) / StrokeDuration;
	const float LinearInsert = StrokePhase < 0.5f ? StrokePhase * 2.0f : (1.0f - StrokePhase) * 2.0f;
	const float SmoothInsert = FMath::SmoothStep(0.0f, 1.0f, LinearInsert);
	AnimatedRammer->SetActorLocationAndRotation(FMath::Lerp(OuterLocation, InnerLocation, SmoothInsert),
		(Forward.Rotation() + RammerRotationOffset), false, nullptr, ETeleportType::TeleportPhysics);

	if (FinishedStrokes >= RequiredRammerStrokes)
	{
		AChongtongLoadingItemActor* FinishedRammer = AnimatedRammer;
		AnimatedRammer = nullptr;
		FinishedRammer->ConsumeAndRespawn();
	}
}

bool AChongtongCannonActor::IsItemRequiredNow(const EChongtongLoadingItemType ItemType) const
{
	switch (LoadingState)
	{
	case EChongtongLoadingState::NeedsPowder: return ItemType == EChongtongLoadingItemType::Powder;
	case EChongtongLoadingState::NeedsRamming: return ItemType == EChongtongLoadingItemType::Rammer;
	case EChongtongLoadingState::NeedsCannonball: return ItemType == EChongtongLoadingItemType::Cannonball;
	default: return false;
	}
}

void AChongtongCannonActor::UpdateInteractionPrompts()
{
	// Exactly one thing is worth touching at a time, so exactly one thing glows.
	for (AChongtongLoadingItemActor* Item : LoadingItems)
	{
		if (IsValid(Item))
		{
			Item->SetLoadingPromptActive(IsItemRequiredNow(Item->GetItemType()));
		}
	}
	if (AimPrompt)
	{
		AimPrompt->SetHighlightActive(LoadingState == EChongtongLoadingState::ReadyToAim);
	}
}

void AChongtongCannonActor::SetLoadingState(const EChongtongLoadingState NewState)
{
	LoadingState = NewState;
	OnLoadingStateChanged.Broadcast(LoadingState, CompletedShots);
}

void AChongtongCannonActor::SpawnPlaceholderLoadingItems()
{
	const EChongtongLoadingItemType Types[] = {EChongtongLoadingItemType::Powder, EChongtongLoadingItemType::Rammer, EChongtongLoadingItemType::Cannonball};
	const TSubclassOf<AChongtongLoadingItemActor> Classes[] = {PowderItemClass, RammerItemClass, CannonballItemClass};
	USceneComponent* SpawnPoints[] = {PowderSpawnPoint, RammerSpawnPoint, CannonballSpawnPoint};
	for (int32 Index = 0; Index < 3; ++Index)
	{
		USceneComponent* SpawnPoint = SpawnPoints[Index];
		if (!SpawnPoint) continue;
		TSubclassOf<AChongtongLoadingItemActor> ItemClass = Classes[Index];
		if (!ItemClass) ItemClass = AChongtongLoadingItemActor::StaticClass();
		AChongtongLoadingItemActor* Item = GetWorld()->SpawnActor<AChongtongLoadingItemActor>(
			ItemClass, SpawnPoint->GetComponentLocation(), SpawnPoint->GetComponentRotation());
		if (Item)
		{
			Item->ConfigureItem(Types[Index]);
			LoadingItems.Add(Item);
		}
	}
}

void AChongtongCannonActor::PlayFeedback(UParticleSystem* Effect, USoundBase* Sound, const FVector& Location, const float Pitch, const FVector EffectScale)
{
	UCombatFXLibrary::SpawnPooledEmitterAtLocation(this, Effect, Location, FRotator::ZeroRotator, EffectScale);
	UCombatFXLibrary::PlayPooledSoundAtLocation(this, Sound, Location, 0.7f, Pitch, CombatSoundConcurrency, CombatSoundAttenuation);
}

AActor* AChongtongCannonActor::SelectTarget() const
{
	TArray<AActor*> Visible = TargetingComponent->FindVisibleHostileTargets(FireRange);
	if (bEngageEnemyInfantryOnly)
	{
		const int32 BeforeFilter = Visible.Num();
		Visible.RemoveAll([](const AActor* Candidate)
		{
			return !IsValid(Candidate) || !Candidate->FindComponentByClass<UCharacterMovementComponent>();
		});
		if (BeforeFilter != Visible.Num())
		{
			UE_LOG(LogOngseong, VeryVerbose, TEXT("%s ignored %d non-infantry target(s)."),
				*GetName(), BeforeFilter - Visible.Num());
		}
	}
	if (Visible.IsEmpty())
	{
		const TArray<AActor*> InRange = TargetingComponent->FindHostileTargets(FireRange);
		FString BlockerText;
		if (!InRange.IsEmpty() && GetWorld())
		{
			FVector ViewLocation;
			FRotator ViewRotation;
			GetActorEyesViewPoint(ViewLocation, ViewRotation);
			FVector TargetLocation;
			FVector TargetExtent;
			InRange[0]->GetActorBounds(true, TargetLocation, TargetExtent);
			FCollisionQueryParams Params(SCENE_QUERY_STAT(ChongtongVisibilityDebug), true, this);
			Params.AddIgnoredActor(this);
			FHitResult Hit;
			if (GetWorld()->LineTraceSingleByChannel(Hit, ViewLocation, TargetLocation, ECC_Visibility, Params))
			{
				BlockerText = FString::Printf(TEXT(" Sight to %s blocked by %s / %s at %s."),
					*InRange[0]->GetName(), *GetNameSafe(Hit.GetActor()),
					Hit.GetComponent() ? *Hit.GetComponent()->GetName() : TEXT("?"),
					*Hit.ImpactPoint.ToCompactString());
			}
		}
		UE_LOG(LogOngseong, Verbose, TEXT("%s: %d hostile(s) within %.0f cm, none visible.%s"),
			*GetName(), InRange.Num(), FireRange, *BlockerText);
	}
	return TargetingComponent->SelectRandom(Visible);
}

bool AChongtongCannonActor::SpawnMountedOperator()
{
	if (IsValid(MountedOperator))
	{
		return true;
	}

	if (!OperatorClass || !GetWorld())
	{
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	MountedOperator = GetWorld()->SpawnActor<AAllyCombatCharacter>(OperatorClass, OperatorSeat->GetComponentTransform(), SpawnParameters);
	if (!MountedOperator)
	{
		return false;
	}

	MountedOperator->AttachToComponent(OperatorSeat, FAttachmentTransformRules::KeepRelativeTransform);
	MountedOperator->SetActorRelativeTransform(OperatorRelativeTransform);
	MountedOperator->SetActorEnableCollision(false);
	if (UCharacterMovementComponent* MovementComponent = MountedOperator->GetCharacterMovement())
	{
		MovementComponent->DisableMovement();
	}
	return true;
}

void AChongtongCannonActor::RemoveMountedOperator()
{
	if (IsValid(MountedOperator))
	{
		MountedOperator->Destroy();
	}
	MountedOperator = nullptr;
}

void AChongtongCannonActor::HandleDeath(UHealthComponent* DeadHealthComponent, const FCombatDamageSpec& KillingDamage)
{
	AutomaticFire->StopAutomaticFire();
	RemoveMountedOperator();
	SetActorEnableCollision(false);
}
