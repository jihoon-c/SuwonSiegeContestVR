#include "Ongseong/OngseongBoltProjectileActor.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/VR/InteractionHighlightComponent.h"
#include "NiagaraSystem.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
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

	// Query-only sensor: no physics/movement response, no Tick, just a Pawn-overlap event so a
	// near miss can still play a whoosh. Kept separate from CollisionComponent's small hit sphere.
	FlybySensor = CreateDefaultSubobject<USphereComponent>(TEXT("FlybySensor"));
	FlybySensor->SetupAttachment(CollisionComponent);
	FlybySensor->InitSphereRadius(FlybySensorRadius);
	FlybySensor->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FlybySensor->SetCollisionObjectType(ECC_WorldDynamic);
	FlybySensor->SetCollisionResponseToAllChannels(ECR_Ignore);
	FlybySensor->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	FlybySensor->SetGenerateOverlapEvents(true);
}

void AOngseongBoltProjectileActor::BeginPlay()
{
	Super::BeginPlay();
	BoltMesh->SetRelativeScale3D(BoltMeshScale);
	BoltMesh->SetRelativeRotation(BoltMeshRotation);
	ApplyVisibilityHighlight(bShowVisibilityHighlight);
	FlybySensor->OnComponentBeginOverlap.AddDynamic(this, &AOngseongBoltProjectileActor::HandleFlybyOverlap);
}

void AOngseongBoltProjectileActor::OnAcquiredFromPool_Implementation()
{
	Super::OnAcquiredFromPool_Implementation();
	ApplyVisibilityHighlight(bShowVisibilityHighlight);
	// Reset so a pooled arrow can whoosh again on its next flight.
	bFlybyTriggered = false;
}

void AOngseongBoltProjectileActor::OnReleasedToPool_Implementation()
{
	Super::OnReleasedToPool_Implementation();
	// A pulsing overlay on a parked pool actor keeps ticking for nothing.
	ApplyVisibilityHighlight(false);
}

void AOngseongBoltProjectileActor::HandleFlybyOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Event-driven, one-shot per flight -- no per-frame distance checks. Only the player pawn
	// triggers the cue; the sensor already filters to Pawn-only overlaps at the collision level.
	if (bFlybyTriggered || !FlybyWhooshSound || !IsValid(OtherActor))
	{
		return;
	}
	if (OtherActor != UGameplayStatics::GetPlayerPawn(this, 0))
	{
		return;
	}
	bFlybyTriggered = true;
	UGameplayStatics::PlaySoundAtLocation(this, FlybyWhooshSound, GetActorLocation(), FlybyWhooshVolume);
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
