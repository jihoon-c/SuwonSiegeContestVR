#include "Ongseong/ChongtongCannonActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/PointLightComponent.h"
#include "Core/VR/InteractionHighlightComponent.h"
#include "Core/VR/VRPlayerPawn.h"
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
#include "Ongseong/OngseongNarrationComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GF_OngseongCrossbow.h"
#include "Gameplay/Combat/CombatFXLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
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
	LoadingSocket = CreateDefaultSubobject<USceneComponent>(TEXT("LoadingSocket"));
	LoadingSocket->SetupAttachment(Muzzle);
	PlayerCameraAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("PlayerCameraAnchor"));
	PlayerCameraAnchor->SetupAttachment(HwachaBaseMesh);
	PlayerCameraAnchor->SetRelativeLocation(FVector(-115.0f, -45.0f, 165.0f));
	PlayerCameraAnchor->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	AimGrip = CreateDefaultSubobject<UChongtongAimGripComponent>(TEXT("AimGrip"));
	AimGrip->SetupAttachment(HwachaBaseMesh);
	AimGrip->SetRelativeLocation(FVector(-40.0f, 0.0f, 120.0f));
	AimGrip->SetAimTarget(BarrelPivot);
	AimPrompt = CreateDefaultSubobject<UInteractionHighlightComponent>(TEXT("AimPrompt"));
	AimPrompt->SetupAttachment(AimGrip);
	// Amber, and only on the grip marker: glowing the whole carriage would hide the barrel.
	AimPrompt->ConfigureHighlight(false, FLinearColor(1.0f, 0.55f, 0.05f));
	AutomaticFire = CreateDefaultSubobject<UChongtongAutomaticFireComponent>(TEXT("AutomaticFire"));
	Narration = CreateDefaultSubobject<UOngseongNarrationComponent>(TEXT("OngseongNarration"));
	StatusText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StatusText"));
	StatusText->SetupAttachment(HwachaBaseMesh);
	StatusText->SetRelativeLocation(FVector(-25.0f, -10.0f, 205.0f));
	StatusText->SetWorldSize(18.0f);
	StatusText->SetHorizontalAlignment(EHTA_Center);
	StatusLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("StatusLight"));
	StatusLight->SetupAttachment(StatusText);
	StatusLight->SetIntensity(1500.0f);
	StatusLight->SetAttenuationRadius(120.0f);

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
	ProjectileClass = AChongtongProjectileActor::StaticClass();
	bSpawnOperatorOnBeginPlay = false;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> LoadFX(TEXT("/Game/NiagaraExamples/FX_PickUp/NS_Pickup_Success.NS_Pickup_Success"));
	// A muzzle flash, not a generic explosion: the shell is what explodes, at the far end.
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> FireFX(TEXT("/Game/NiagaraExamples/FX_Weapons/MuzzleFlashes/NS_MuzzleFlash.NS_MuzzleFlash"));
	static ConstructorHelpers::FObjectFinder<USoundBase> TempFireSound(TEXT("/Game/XRFramework/Audio/Fire_Cue.Fire_Cue"));
	LoadSuccessEffect = LoadFX.Object;
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
	if (bSpawnPlaceholderProps)
	{
		SpawnPlaceholderLoadingItems();
	}
	UpdateStatusSignal();
}

void AChongtongCannonActor::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
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
	PlayFeedback(MuzzleEffect, FireSound, Muzzle->GetComponentLocation());
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
		DesiredHorizontal.Rotation().Yaw);
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
	return Projectile;
}

bool AChongtongCannonActor::TryFirePlayer()
{
	if (LoadingState != EChongtongLoadingState::ReadyToAim || !AimGrip->IsTwoHandAiming()) return false;
	const FVector Direction = Muzzle->GetComponentTransform().GetUnitAxis(EAxis::Y).GetSafeNormal();
	AGameplayProjectileActor* Projectile = SpawnProjectile(Direction, ProjectileSpeed);
	if (!Projectile) return false;

	++CompletedShots;
	PlayFeedback(MuzzleEffect, FireSound, Muzzle->GetComponentLocation());
	OnFired.Broadcast(nullptr, Projectile);
	ExitReadyStation();
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

void AChongtongCannonActor::BeginPlayerAim()
{
	if (LoadingState == EChongtongLoadingState::ReadyToAim) EnterReadyStation();
}

void AChongtongCannonActor::EndPlayerAim()
{
}

bool AChongtongCannonActor::TryLoadItem(const EChongtongLoadingItemType ItemType)
{
	if (ItemType == EChongtongLoadingItemType::Powder && LoadingState == EChongtongLoadingState::NeedsPowder)
	{
		SetLoadingState(EChongtongLoadingState::NeedsRamming);
		PlayFeedback(LoadSuccessEffect, InteractionSound, LoadingSocket->GetComponentLocation(), 1.15f);
		return true;
	}
	if (ItemType == EChongtongLoadingItemType::Cannonball && LoadingState == EChongtongLoadingState::NeedsCannonball)
	{
		SetLoadingState(EChongtongLoadingState::ReadyToAim);
		PlayFeedback(LoadSuccessEffect, InteractionSound, LoadingSocket->GetComponentLocation(), 1.35f);
		EnterReadyStation();
		return true;
	}
	return false;
}

bool AChongtongCannonActor::RegisterRammerStroke()
{
	if (LoadingState != EChongtongLoadingState::NeedsRamming) return false;
	CompletedRammerStrokes = FMath::Min(CompletedRammerStrokes + 1, RequiredRammerStrokes);
	OnRammingProgress.Broadcast(CompletedRammerStrokes, RequiredRammerStrokes);
	PlayFeedback(LoadSuccessEffect, InteractionSound, LoadingSocket->GetComponentLocation(), 0.8f + CompletedRammerStrokes * 0.1f);
	if (CompletedRammerStrokes >= RequiredRammerStrokes) SetLoadingState(EChongtongLoadingState::NeedsCannonball);
	else UpdateStatusSignal();
	return true;
}

void AChongtongCannonActor::UpdateLoadingInteractions()
{
	UpdateInteractionPrompts();
	if (!LoadingSocket || LoadingState == EChongtongLoadingState::ReadyToAim || LoadingState == EChongtongLoadingState::Completed) return;
	for (AChongtongLoadingItemActor* Item : LoadingItems)
	{
		if (!IsValid(Item) || Item->IsHidden()) continue;
		const float Distance = Item->GetDistanceToPoint(LoadingSocket->GetComponentLocation());
		if (Item->GetItemType() == EChongtongLoadingItemType::Rammer)
		{
			if (LoadingState == EChongtongLoadingState::NeedsRamming && Distance <= LoadingAcceptanceRadius && !bRammerInserted)
			{
				bRammerInserted = RegisterRammerStroke();
			}
			else if (Distance >= RammerWithdrawRadius) bRammerInserted = false;
		}
		else if (Distance <= LoadingAcceptanceRadius && TryLoadItem(Item->GetItemType()))
		{
			Item->ConsumeAndRespawn();
		}
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
	UpdateStatusSignal();
	OnLoadingStateChanged.Broadcast(LoadingState, CompletedShots);
}

void AChongtongCannonActor::UpdateStatusSignal()
{
	if (!StatusText || !StatusLight) return;
	FText Text;
	FLinearColor Color = FLinearColor::Yellow;
	switch (LoadingState)
	{
	case EChongtongLoadingState::NeedsPowder: Text = FText::FromString(TEXT("1. LOAD POWDER")); Color = FLinearColor::Red; break;
	case EChongtongLoadingState::NeedsRamming: Text = FText::FromString(FString::Printf(TEXT("2. RAM %d / %d"), CompletedRammerStrokes, RequiredRammerStrokes)); Color = FLinearColor::Yellow; break;
	case EChongtongLoadingState::NeedsCannonball: Text = FText::FromString(TEXT("3. LOAD CANNONBALL")); Color = FLinearColor(1.0f, 0.45f, 0.0f); break;
	case EChongtongLoadingState::ReadyToAim: Text = FText::FromString(TEXT("READY - GRIP BOTH HANDS")); Color = FLinearColor::Green; break;
	default: Text = FText::FromString(TEXT("ALL ENEMIES DEFEATED")); Color = FLinearColor::Blue; break;
	}
	StatusText->SetText(Text);
	StatusText->SetTextRenderColor(Color.ToFColor(true));
	StatusLight->SetLightColor(Color);
}

void AChongtongCannonActor::EnterReadyStation()
{
	for (TActorIterator<AVRPlayerPawn> It(GetWorld()); It; ++It)
	{
		It->EnterMountedInteraction(PlayerCameraAnchor);
		break;
	}
}

void AChongtongCannonActor::ExitReadyStation()
{
	for (TActorIterator<AVRPlayerPawn> It(GetWorld()); It; ++It) It->ExitMountedInteraction(PlayerCameraAnchor);
}

void AChongtongCannonActor::SpawnPlaceholderLoadingItems()
{
	const FVector Base = PlayerCameraAnchor->GetComponentLocation();
	const FVector Right = PlayerCameraAnchor->GetRightVector();
	const FVector Forward = PlayerCameraAnchor->GetForwardVector();
	const EChongtongLoadingItemType Types[] = {EChongtongLoadingItemType::Powder, EChongtongLoadingItemType::Rammer, EChongtongLoadingItemType::Cannonball};
	const TSubclassOf<AChongtongLoadingItemActor> Classes[] = {PowderItemClass, RammerItemClass, CannonballItemClass};
	for (int32 Index = 0; Index < 3; ++Index)
	{
		const FVector Location = Base + Forward * 65.0f + Right * ((Index - 1) * 38.0f) - FVector::UpVector * 65.0f;
		TSubclassOf<AChongtongLoadingItemActor> ItemClass = Classes[Index];
		if (!ItemClass) ItemClass = AChongtongLoadingItemActor::StaticClass();
		AChongtongLoadingItemActor* Item = GetWorld()->SpawnActor<AChongtongLoadingItemActor>(ItemClass, Location, GetActorRotation());
		if (Item)
		{
			Item->ConfigureItem(Types[Index]);
			LoadingItems.Add(Item);
		}
	}
}

void AChongtongCannonActor::PlayFeedback(UNiagaraSystem* Effect, USoundBase* Sound, const FVector& Location, const float Pitch)
{
	UCombatFXLibrary::SpawnPooledSystemAtLocation(this, Effect, Location);
	UCombatFXLibrary::PlayPooledSoundAtLocation(this, Sound, Location, 0.7f, Pitch, CombatSoundConcurrency);
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
