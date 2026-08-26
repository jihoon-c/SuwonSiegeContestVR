#include "Ongseong/OngseongBoltProjectileActor.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/VR/InteractionHighlightComponent.h"
#include "NiagaraSystem.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

AOngseongBoltProjectileActor::AOngseongBoltProjectileActor()
{
	CollisionComponent->InitSphereRadius(3.0f);
	ProjectileMovement->ProjectileGravityScale = 0.15f;
	ProjectileMovement->MaxSpeed = 15000.0f;
	LifeSpanSeconds = 8.0f;

	BoltMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoltMesh"));
	BoltMesh->SetupAttachment(CollisionComponent);
	BoltMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// BP_OngseongBolt never had a mesh assigned, so pooled arrows flew invisibly and the
	// archers looked like they were firing nothing. Default to the feature's own arrow art.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ArrowAsset(
		TEXT("/GF_OngseongCrossbow/Asset/Prop/Arrow/bow_and_arrow/StaticMeshes/Arrow.Arrow"));
	if (ArrowAsset.Succeeded())
	{
		BoltMesh->SetStaticMesh(ArrowAsset.Object);
	}

	VisibilityHighlight = CreateDefaultSubobject<UInteractionHighlightComponent>(TEXT("VisibilityHighlight"));
	VisibilityHighlight->SetupAttachment(CollisionComponent);
	VisibilityHighlight->ConfigureHighlight(true, VisibilityHighlightColor);
	// Overlay only: a looping Niagara marker dragged along a 6500 cm/s arrow is pure cost.
	VisibilityHighlight->SetHighlightEffectAsset(TSoftObjectPtr<UNiagaraSystem>());
}

void AOngseongBoltProjectileActor::BeginPlay()
{
	Super::BeginPlay();
	BoltMesh->SetRelativeScale3D(BoltMeshScale);
	BoltMesh->SetRelativeRotation(BoltMeshRotation);
	ApplyVisibilityHighlight(bShowVisibilityHighlight);
}

void AOngseongBoltProjectileActor::OnAcquiredFromPool_Implementation()
{
	Super::OnAcquiredFromPool_Implementation();
	ApplyVisibilityHighlight(bShowVisibilityHighlight);
}

void AOngseongBoltProjectileActor::OnReleasedToPool_Implementation()
{
	Super::OnReleasedToPool_Implementation();
	// A pulsing overlay on a parked pool actor keeps ticking for nothing.
	ApplyVisibilityHighlight(false);
}

void AOngseongBoltProjectileActor::ApplyVisibilityHighlight(const bool bActive)
{
	if (!VisibilityHighlight)
	{
		return;
	}
	VisibilityHighlight->SetHighlightColor(VisibilityHighlightColor);
	VisibilityHighlight->SetHighlightPulse(bPulseVisibilityHighlight, VisibilityHighlightPulsesPerSecond);
	VisibilityHighlight->SetHighlightActive(bActive);
}
