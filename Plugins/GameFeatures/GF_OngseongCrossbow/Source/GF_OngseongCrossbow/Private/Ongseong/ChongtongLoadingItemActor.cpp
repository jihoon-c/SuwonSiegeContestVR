#include "Ongseong/ChongtongLoadingItemActor.h"

#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/StructOnScope.h"

AChongtongLoadingItemActor::AChongtongLoadingItemActor()
{
	PrimaryActorTick.bCanEverTick = false;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	PowderMesh = Cylinder.Object;
	RammerMesh = Cylinder.Object;
	CannonballMesh = Sphere.Object;
}

void AChongtongLoadingItemActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	HomeTransform = Transform;
	ApplyPlaceholderAppearance();
}

void AChongtongLoadingItemActor::ConfigureItem(EChongtongLoadingItemType NewType)
{
	ItemType = NewType;
	ApplyPlaceholderAppearance();
	HomeTransform = GetActorTransform();
}

void AChongtongLoadingItemActor::ApplyPlaceholderAppearance()
{
	if (ItemType == EChongtongLoadingItemType::Cannonball)
	{
		Mesh->SetStaticMesh(CannonballMesh);
		Mesh->SetRelativeScale3D(FVector(0.18f));
	}
	else
	{
		Mesh->SetStaticMesh(ItemType == EChongtongLoadingItemType::Rammer ? RammerMesh : PowderMesh);
		Mesh->SetRelativeScale3D(ItemType == EChongtongLoadingItemType::Rammer ? FVector(0.05f, 0.05f, 1.2f) : FVector(0.16f, 0.16f, 0.25f));
	}
}

void AChongtongLoadingItemActor::ConsumeAndRespawn(float DelaySeconds)
{
	TInlineComponentArray<USceneComponent*> SceneComponents(this);
	for (USceneComponent* Component : SceneComponents)
	{
		if (UFunction* ReleaseFunction = Component ? Component->FindFunction(TEXT("TryRelease")) : nullptr)
		{
			FStructOnScope Parameters(ReleaseFunction);
			Component->ProcessEvent(ReleaseFunction, Parameters.GetStructMemory());
		}
	}
	Mesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	GetWorldTimerManager().SetTimerForNextTick([this, DelaySeconds]()
	{
		FTimerHandle Handle;
		GetWorldTimerManager().SetTimer(Handle, this, &AChongtongLoadingItemActor::RespawnAtHome, FMath::Max(0.01f, DelaySeconds), false);
	});
}

float AChongtongLoadingItemActor::GetDistanceToPoint(const FVector WorldPoint) const
{
	FVector ClosestPoint = GetActorLocation();
	const float CollisionDistance = Mesh ? Mesh->GetClosestPointOnCollision(WorldPoint, ClosestPoint) : -1.0f;
	return CollisionDistance >= 0.0f ? CollisionDistance : FVector::Distance(WorldPoint, GetActorLocation());
}

void AChongtongLoadingItemActor::RespawnAtHome()
{
	SetActorTransform(HomeTransform, false, nullptr, ETeleportType::TeleportPhysics);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
}
