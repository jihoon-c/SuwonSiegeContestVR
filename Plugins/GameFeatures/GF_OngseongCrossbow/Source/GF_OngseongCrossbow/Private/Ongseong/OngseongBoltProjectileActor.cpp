#include "Ongseong/OngseongBoltProjectileActor.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AOngseongBoltProjectileActor::AOngseongBoltProjectileActor()
{
	CollisionComponent->InitSphereRadius(3.0f);
	ProjectileMovement->ProjectileGravityScale = 0.15f;
	ProjectileMovement->MaxSpeed = 15000.0f;
	LifeSpanSeconds = 8.0f;

	BoltMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoltMesh"));
	BoltMesh->SetupAttachment(CollisionComponent);
	BoltMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
