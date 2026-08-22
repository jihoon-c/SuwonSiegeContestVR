#include "Nokro/NokroRepairTargetActor.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ANokroRepairTargetActor::ANokroRepairTargetActor()
{
	PrimaryActorTick.bCanEverTick = false;
	PlacementVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("PlacementVolume"));
	SetRootComponent(PlacementVolume);
	PlacementVolume->SetBoxExtent(FVector(50.0f, 25.0f, 25.0f));
	PlacementVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PlacementVolume->SetCollisionResponseToAllChannels(ECR_Ignore);

	YellowMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("YellowMarker"));
	YellowMarker->SetupAttachment(PlacementVolume);
	YellowMarker->SetRelativeScale3D(FVector(1.0f, 0.5f, 0.5f));
	YellowMarker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	YellowMarker->SetCastShadow(false);

	PlacedStone = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlacedStone"));
	PlacedStone->SetupAttachment(PlacementVolume);
	PlacedStone->SetRelativeScale3D(FVector(1.0f, 0.5f, 0.5f));
	PlacedStone->SetCollisionProfileName(TEXT("BlockAll"));
	PlacedStone->SetVisibility(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (Cube.Succeeded())
	{
		YellowMarker->SetStaticMesh(Cube.Object);
		PlacedStone->SetStaticMesh(Cube.Object);
	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> YellowMaterial(TEXT("/GF_Geojunggi/Materials/M_NokroTarget.M_NokroTarget"));
	if (YellowMaterial.Succeeded()) YellowMarker->SetMaterial(0, YellowMaterial.Object);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> StoneMaterial(TEXT("/GF_Geojunggi/Materials/M_NokroStone.M_NokroStone"));
	if (StoneMaterial.Succeeded()) PlacedStone->SetMaterial(0, StoneMaterial.Object);
}

bool ANokroRepairTargetActor::IsStoneWithinTolerance(const FTransform& StoneTransform) const
{
	if (bRepaired) return false;
	const FTransform Target = GetPlacementTransform();
	const float Distance = FVector::Distance(StoneTransform.GetLocation(), Target.GetLocation());
	const float YawDelta = FMath::Abs(FMath::FindDeltaAngleDegrees(StoneTransform.Rotator().Yaw, Target.Rotator().Yaw));
	return Distance <= PositionTolerance && YawDelta <= YawTolerance;
}

void ANokroRepairTargetActor::CompleteRepair()
{
	bRepaired = true;
	YellowMarker->SetVisibility(false);
	PlacedStone->SetVisibility(true);
	PlacementVolume->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void ANokroRepairTargetActor::ResetRepair()
{
	bRepaired = false;
	YellowMarker->SetVisibility(true);
	PlacedStone->SetVisibility(false);
	PlacementVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

FTransform ANokroRepairTargetActor::GetPlacementTransform() const
{
	return PlacementVolume->GetComponentTransform();
}
