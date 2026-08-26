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
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
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
	// Do not use NS_Dirt_Explosion_Medium here: its sample post-process emitter can tint the whole
	// scene lime green, including terrain and skeletal enemies. This impact system is local-only.
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> TempExplosion(TEXT("/Game/NiagaraExamples/FX_Weapons/Impacts/NS_Impact_Concrete.NS_Impact_Concrete"));
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
	const FVector Location = Hit.ImpactPoint.IsNearlyZero() ? GetActorLocation() : FVector(Hit.ImpactPoint);
	const UHealthComponent* HitHealth = IsValid(HitActor) ? HitActor->FindComponentByClass<UHealthComponent>() : nullptr;
	UE_LOG(LogOngseong, Verbose, TEXT("Chongtong shell exploded on %s at %s.%s"),
		*GetNameSafe(HitActor), *Location.ToCompactString(),
		HitHealth ? *FString::Printf(TEXT(" Health now %.0f/%.0f (dead=%d)."),
			HitHealth->GetCurrentHealth(), HitHealth->GetMaxHealth(), HitHealth->IsDead() ? 1 : 0) : TEXT(""));
	UCombatFXLibrary::SpawnPooledSystemAtLocation(this, ExplosionEffect, Location, FRotator::ZeroRotator, ExplosionEffectScale);
	// A concurrency asset may steal an existing explosion before its cue tail finishes.
	UCombatFXLibrary::PlayPooledSoundAtLocation(this, ExplosionSound, Location, 1.0f, 0.7f, nullptr, ExplosionSoundAttenuation);

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
