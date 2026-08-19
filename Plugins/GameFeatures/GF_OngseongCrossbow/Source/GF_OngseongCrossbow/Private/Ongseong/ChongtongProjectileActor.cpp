#include "Ongseong/ChongtongProjectileActor.h"

#include "GameFramework/ProjectileMovementComponent.h"

AChongtongProjectileActor::AChongtongProjectileActor()
{
	ProjectileMovement->ProjectileGravityScale = 0.15f;
	ProjectileMovement->MaxSpeed = 8000.0f;
}
