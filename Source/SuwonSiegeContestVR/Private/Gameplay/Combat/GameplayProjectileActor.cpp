#include "Gameplay/Combat/GameplayProjectileActor.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Gameplay/Combat/CombatDamageLibrary.h"
#include "Gameplay/Pooling/ActorPool.h"

AGameplayProjectileActor::AGameplayProjectileActor()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(5.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionComponent->SetNotifyRigidBodyCollision(true);
	SetRootComponent(CollisionComponent);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 0.0f;
	ProjectileMovement->MaxSpeed = 10000.0f;
	ProjectileMovement->bAutoActivate = false;
	ProjectileMovement->bRotationFollowsVelocity = true;
}

void AGameplayProjectileActor::BeginPlay()
{
	Super::BeginPlay();
	CollisionComponent->OnComponentHit.AddDynamic(this, &AGameplayProjectileActor::HandleProjectileHit);
	SetLifeSpan(LifeSpanSeconds);
}

void AGameplayProjectileActor::LifeSpanExpired()
{
	if (AActorPool* OwningPool = Cast<AActorPool>(GetOwner()))
	{
		if (OwningPool->ReleaseActor(this))
		{
			return;
		}
	}

	Super::LifeSpanExpired();
}

void AGameplayProjectileActor::OnAcquiredFromPool_Implementation()
{
	CollisionComponent->ClearMoveIgnoreActors();
	DamageSpec = FCombatDamageSpec();
	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->Deactivate();
	SetLifeSpan(LifeSpanSeconds);
}

void AGameplayProjectileActor::OnReleasedToPool_Implementation()
{
	CollisionComponent->ClearMoveIgnoreActors();
	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->Deactivate();
	DamageSpec = FCombatDamageSpec();
	SetLifeSpan(0.0f);
}

void AGameplayProjectileActor::LaunchProjectile(const FVector Direction, const float Speed, const FCombatDamageSpec& InDamageSpec)
{
	SetDamageSpec(InDamageSpec);
	ProjectileMovement->Velocity = Direction.GetSafeNormal() * FMath::Max(0.0f, Speed);
	ProjectileMovement->Activate(true);
}

void AGameplayProjectileActor::SetDamageSpec(const FCombatDamageSpec& InDamageSpec)
{
	DamageSpec = InDamageSpec;
	if (!DamageSpec.DamageCauser)
	{
		DamageSpec.DamageCauser = this;
	}
	if (IsValid(DamageSpec.InstigatorActor)) CollisionComponent->IgnoreActorWhenMoving(DamageSpec.InstigatorActor, true);
	if (IsValid(GetOwner())) CollisionComponent->IgnoreActorWhenMoving(GetOwner(), true);
}

void AGameplayProjectileActor::HandleProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!IsValid(OtherActor) || OtherActor == this || OtherActor == GetInstigator())
	{
		return;
	}

	UCombatDamageLibrary::ApplyCombatDamage(OtherActor, DamageSpec);
	OnProjectileImpact.Broadcast(this, OtherActor, Hit);
	if (bDestroyOnImpact)
	{
		if (AActorPool* OwningPool = Cast<AActorPool>(GetOwner()))
		{
			if (OwningPool->ReleaseActor(this))
			{
				return;
			}
		}
		Destroy();
	}
}
