#include "Ongseong/ChongtongProjectileActor.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/OverlapResult.h"
#include "Gameplay/Combat/CombatDamageLibrary.h"
#include "Gameplay/Combat/HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GF_OngseongCrossbow.h"
#include "Gameplay/Combat/CombatFXLibrary.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundAttenuation.h"
#include "UObject/ConstructorHelpers.h"

AChongtongProjectileActor::AChongtongProjectileActor()
{
	// A cannonball, not a bullet: heavy enough to arc visibly over the corridor.
	ProjectileMovement->ProjectileGravityScale = 0.7f;
	ProjectileMovement->MaxSpeed = 5000.0f;
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(CollisionComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMesh->SetRelativeScale3D(FVector(0.12f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	// Cascade instead of Niagara: the sample Niagara impact systems carry post-process and light
	// renderers that tint the whole scene, and P_Explosion is a self-contained local burst.
	static ConstructorHelpers::FObjectFinder<UParticleSystem> TempExplosion(TEXT("/Game/StarterContent/Particles/P_Explosion.P_Explosion"));
	static ConstructorHelpers::FObjectFinder<USoundBase> TempSound(TEXT("/Game/XRFramework/Audio/Fire_Cue.Fire_Cue"));
	ProjectileMesh->SetStaticMesh(Sphere.Object);
	ExplosionEffect = TempExplosion.Object;
	ExplosionSound = TempSound.Object;
	ExplosionSoundAttenuation = CreateDefaultSubobject<USoundAttenuation>(TEXT("ExplosionSoundAttenuation"));
	ExplosionSoundAttenuation->Attenuation.bAttenuate = true;
	ExplosionSoundAttenuation->Attenuation.AttenuationShape = EAttenuationShape::Sphere;
	ExplosionSoundAttenuation->Attenuation.AttenuationShapeExtents = FVector(500.0f);
	ExplosionSoundAttenuation->Attenuation.FalloffDistance = 16000.0f;
}

void AChongtongProjectileActor::BeginPlay()
{
	Super::BeginPlay();
	OnProjectileImpact.AddUniqueDynamic(this, &AChongtongProjectileActor::HandleExplosion);
}

void AChongtongProjectileActor::HandleExplosion(AGameplayProjectileActor* Projectile, AActor* HitActor, const FHitResult Hit)
{
	// Damage is resolved here, at the point the shell actually touched.
	const FVector Location = Hit.ImpactPoint.IsNearlyZero() ? GetActorLocation() : FVector(Hit.ImpactPoint);
	// The burst is played here, on the floor below, so a shell that clips a soldier does not
	// look like the soldier detonating.
	const FVector EffectLocation = bGroundExplosionEffect ? FindGroundedEffectLocation(Location, HitActor) : Location;
	const UHealthComponent* HitHealth = IsValid(HitActor) ? HitActor->FindComponentByClass<UHealthComponent>() : nullptr;
	UE_LOG(LogOngseong, Verbose, TEXT("Chongtong shell exploded on %s at %s.%s"),
		*GetNameSafe(HitActor), *Location.ToCompactString(),
		HitHealth ? *FString::Printf(TEXT(" Health now %.0f/%.0f (dead=%d)."),
			HitHealth->GetCurrentHealth(), HitHealth->GetMaxHealth(), HitHealth->IsDead() ? 1 : 0) : TEXT(""));
	// The shell's impact is the player's only confirmation that the shot landed.
	const UParticleSystemComponent* SpawnedExplosion = UCombatFXLibrary::SpawnPooledEmitterAtLocation(
		this, ExplosionEffect, EffectLocation, FRotator::ZeroRotator, ExplosionEffectScale);
	if (!SpawnedExplosion)
	{
		UE_LOG(LogOngseong, Warning,
			TEXT("%s landed at %s without an impact effect (ExplosionEffect=%s). Assign one in BP_ChongtongProjectile Class Defaults."),
			*GetName(), *EffectLocation.ToCompactString(), *GetNameSafe(ExplosionEffect));
	}
	// A concurrency asset may steal an existing explosion before its cue tail finishes.
	UCombatFXLibrary::PlayPooledSoundAtLocation(this, ExplosionSound, EffectLocation, 1.0f, 0.7f, nullptr, ExplosionSoundAttenuation);

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams Objects;
	Objects.AddObjectTypesToQuery(ECC_Pawn);
	Objects.AddObjectTypesToQuery(ECC_WorldDynamic);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(ChongtongExplosion), false, this);
	GetWorld()->OverlapMultiByObjectType(Overlaps, Location, FQuat::Identity, Objects, FCollisionShape::MakeSphere(ExplosionRadius), Params);
	TSet<AActor*> DamagedActors;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Target = Overlap.GetActor();
		if (!IsValid(Target) || Target == HitActor || Target == GetOwner() || DamagedActors.Contains(Target)) continue;
		DamagedActors.Add(Target);
		FCombatDamageSpec ExplosionDamage = DamageSpec;
		ExplosionDamage.Amount = AreaDamage;
		ExplosionDamage.DamageCauser = this;
		UCombatDamageLibrary::ApplyCombatDamage(Target, ExplosionDamage);
	}
}

FVector AChongtongProjectileActor::FindGroundedEffectLocation(const FVector& ImpactLocation, const AActor* HitActor) const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return ImpactLocation;
	}

	// Static geometry only: tracing against pawns would land the burst on the next soldier's head,
	// which is the problem this is here to solve.
	FCollisionObjectQueryParams Objects;
	Objects.AddObjectTypesToQuery(ECC_WorldStatic);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(ChongtongGroundProbe), /*bTraceComplex=*/true, this);
	if (IsValid(HitActor))
	{
		Params.AddIgnoredActor(HitActor);
	}
	if (IsValid(GetOwner()))
	{
		Params.AddIgnoredActor(GetOwner());
	}

	// Start slightly above the impact so a shell that grazed a soldier standing on the floor
	// still finds the floor rather than starting inside it.
	const FVector TraceStart = ImpactLocation + FVector::UpVector * 50.0f;
	const FVector TraceEnd = ImpactLocation - FVector::UpVector * FMath::Max(0.0f, GroundTraceDistance);
	FHitResult GroundHit;
	if (World->LineTraceSingleByObjectType(GroundHit, TraceStart, TraceEnd, Objects, Params))
	{
		return GroundHit.ImpactPoint + FVector::UpVector * GroundEffectHeightOffset;
	}
	// No floor within reach (a wall hit, or a shell caught over open air): keep the real impact.
	return ImpactLocation;
}
