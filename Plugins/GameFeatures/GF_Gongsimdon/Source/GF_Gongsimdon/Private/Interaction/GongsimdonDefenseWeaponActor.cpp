#include "Interaction/GongsimdonDefenseWeaponActor.h"

#include "Components/StaticMeshComponent.h"
#include "Core/Scenario/ScenarioInteractableComponent.h"
#include "Core/Scenario/ScenarioTypes.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "Shared/Combat/LegacyHealthComponent.h"
#include "UObject/ConstructorHelpers.h"

AGongsimdonDefenseWeaponActor::AGongsimdonDefenseWeaponActor()
{
	PrimaryActorTick.bCanEverTick = false;
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	WeaponMesh->ComponentTags.Add(TEXT("VRGrab"));
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	WeaponMesh->SetRelativeScale3D(FVector(0.35f, 0.06f, 0.06f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> WeaponAsset(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (WeaponAsset.Succeeded())
	{
		WeaponMesh->SetStaticMesh(WeaponAsset.Object);
	}
	CombatGuideInteractor = CreateDefaultSubobject<UScenarioInteractableComponent>(
		TEXT("CombatGuideInteractor"));
	CombatGuideInteractor->TargetID = TEXT("COMBAT_RETREATING");
	CombatGuideInteractor->SupportedInteractionTypes = {EScenarioInteractionType::Combat};
}

bool AGongsimdonDefenseWeaponActor::HandleVRGrabbed(
	USceneComponent*, UMotionControllerComponent* MotionController)
{
	return MotionController && FireFromTransform(
		MotionController->GetComponentLocation(),
		MotionController->GetForwardVector(),
		MotionController->GetOwner());
}

bool AGongsimdonDefenseWeaponActor::FireFromTransform(
	const FVector Origin, const FVector Direction, AActor* InstigatorActor)
{
	if (!GetWorld() || Direction.IsNearlyZero())
	{
		return false;
	}

	const FVector ShotDirection = Direction.GetSafeNormal();
	const FVector End = Origin + ShotDirection * FMath::Max(100.0f, ShotRange);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(GongsimdonDefenseShot), false, this);
	QueryParams.AddIgnoredActor(this);
	if (InstigatorActor)
	{
		QueryParams.AddIgnoredActor(InstigatorActor);
	}

	TArray<FHitResult> Hits;
	GetWorld()->SweepMultiByChannel(
		Hits, Origin, End, FQuat::Identity, ECC_Visibility,
		FCollisionShape::MakeSphere(FMath::Max(0.0f, AimAssistRadius)), QueryParams);
	Hits.Sort([](const FHitResult& Left, const FHitResult& Right)
	{
		return Left.Distance < Right.Distance;
	});

	AActor* EnemyHit = nullptr;
	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (IsValid(HitActor) && HitActor->FindComponentByClass<ULegacyHealthComponent>())
		{
			EnemyHit = HitActor;
			break;
		}
	}
	if (!EnemyHit)
	{
		float BestForwardDistance = TNumericLimits<float>::Max();
		for (TActorIterator<AActor> It(GetWorld()); It; ++It)
		{
			AActor* Candidate = *It;
			if (!IsValid(Candidate) || Candidate == this || Candidate == InstigatorActor ||
				Candidate->IsHidden() || !Candidate->FindComponentByClass<ULegacyHealthComponent>())
			{
				continue;
			}
			FVector BoundsOrigin;
			FVector BoundsExtent;
			Candidate->GetActorBounds(true, BoundsOrigin, BoundsExtent);
			const FVector ToTarget = BoundsOrigin - Origin;
			const float ForwardDistance = FVector::DotProduct(ToTarget, ShotDirection);
			const float PerpendicularDistance =
				(ToTarget - ShotDirection * ForwardDistance).Size();
			const float EffectiveRadius = FMath::Max(0.0f, AimAssistRadius) + BoundsExtent.Size2D();
			if (ForwardDistance > 0.0f && ForwardDistance <= ShotRange &&
				PerpendicularDistance <= EffectiveRadius && ForwardDistance < BestForwardDistance)
			{
				BestForwardDistance = ForwardDistance;
				EnemyHit = Candidate;
			}
		}
	}

	if (EnemyHit)
	{
		ULegacyHealthComponent* Health = EnemyHit->FindComponentByClass<ULegacyHealthComponent>();
		const float HealthBeforeDamage = Health ? Health->GetCurrentHealth() : 0.0f;
		UGameplayStatics::ApplyDamage(
			EnemyHit, FMath::Max(0.1f, ShotDamage), nullptr, this, nullptr);
		// Standard damage delegates are not installed before BeginPlay in editor test worlds.
		// The component fallback also keeps authored preview worlds and runtime-spawn edge cases usable.
		if (Health && FMath::IsNearlyEqual(Health->GetCurrentHealth(), HealthBeforeDamage))
		{
			Health->ApplyHealthDamage(FMath::Max(0.1f, ShotDamage));
		}
	}
	OnDefenseShot.Broadcast(EnemyHit != nullptr, EnemyHit);
	return true;
}
