#include "Singijeon/SingijeonProjectileActor.h"

#include "Components/StaticMeshComponent.h"
#include "Core/Scenario/ScenarioInteractableComponent.h"
#include "Core/Scenario/ScenarioTypes.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Shared/Combat/LegacyHealthComponent.h"
#include "UObject/StructOnScope.h"
#include "UObject/ConstructorHelpers.h"

ASingijeonProjectileActor::ASingijeonProjectileActor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
    PrimaryActorTick.TickGroup = TG_PostPhysics;

    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
    SetRootComponent(ProjectileMesh);
    ProjectileMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
    ProjectileMesh->SetNotifyRigidBodyCollision(true);
    ProjectileMesh->OnComponentHit.AddDynamic(this, &ThisClass::HandleProjectileHit);
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultArrowMaterial(
        TEXT("/GF_Singijeon/Asset/Arrow/arrowb/Materials/M_SingijeonArrow_Runtime.M_SingijeonArrow_Runtime"));
    if (DefaultArrowMaterial.Succeeded())
    {
        ProjectileMaterialOverride = DefaultArrowMaterial.Object;
        ProjectileMesh->SetMaterial(0, ProjectileMaterialOverride);
    }

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = ProjectileMesh;
    ProjectileMovement->InitialSpeed = 0.0f;
    ProjectileMovement->MaxSpeed = 10000.0f;
    ProjectileMovement->ProjectileGravityScale = 1.0f;
    ProjectileMovement->bAutoActivate = false;
    // ProjectileMovement assumes +X is the visual front. The imported arrowhead
    // points along -X, so the authored tip axis is aligned explicitly in Tick.
    ProjectileMovement->bRotationFollowsVelocity = false;

    FlightTrailEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FlightTrailEffect"));
    FlightTrailEffect->SetupAttachment(ProjectileMesh);
    FlightTrailEffect->SetAutoActivate(false);
    FlightTrailEffect->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    GrabScenarioInteractor = CreateDefaultSubobject<UScenarioInteractableComponent>(TEXT("GrabScenarioInteractor"));
    GrabScenarioInteractor->TargetID = TEXT("Singijeon_Ammo");
    GrabScenarioInteractor->SupportedInteractionTypes = { EScenarioInteractionType::Grab };
}

void ASingijeonProjectileActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    ApplyFlightTrailSettings();
}

void ASingijeonProjectileActor::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bWasLaunched || !ProjectileMovement)
    {
        return;
    }

    const FVector VelocityDirection = ProjectileMovement->Velocity.GetSafeNormal();
    const FVector CurrentTipDirection = GetArrowTipDirection();
    if (!VelocityDirection.IsNearlyZero() && !CurrentTipDirection.IsNearlyZero())
    {
        const FQuat TipAlignment = FQuat::FindBetweenNormals(CurrentTipDirection, VelocityDirection);
        SetActorRotation(TipAlignment * GetActorQuat(), ETeleportType::TeleportPhysics);
    }
}

FVector ASingijeonProjectileActor::GetVelocity() const
{
    return ProjectileMovement ? ProjectileMovement->Velocity : Super::GetVelocity();
}

bool ASingijeonProjectileActor::CanBeLoaded_Implementation() const
{
    return !bIsLoaded && !bWasLaunched;
}

bool ASingijeonProjectileActor::PrepareForLoading_Implementation()
{
	StopFlightTrail();
	TInlineComponentArray<UActorComponent*> Components(this);
	for (UActorComponent* Component : Components)
	{
		if (!IsValid(Component))
		{
			continue;
		}
		if (UFunction* ReleaseFunction = Component->FindFunction(TEXT("TryRelease")))
		{
			// Blueprint functions can have a return value even when they have no input pins.
			// ProcessEvent must receive a correctly sized parameter frame in that case.
			FStructOnScope ParameterScope(ReleaseFunction);
			Component->ProcessEvent(ReleaseFunction, ParameterScope.GetStructMemory());
			break;
		}
	}

	// Template grab release restores physics. Disable it again before the slot attempts
	// attachment; attaching a simulated root can fail and leave the ammunition unsnapped.
	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->Deactivate();
	ProjectileMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
	ProjectileMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	ProjectileMesh->SetSimulatePhysics(false);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    return true;
}

void ASingijeonProjectileActor::OnLoaded_Implementation(USceneComponent* Slot)
{
    bIsLoaded = true;
    SetActorTickEnabled(false);
    ProjectileMovement->StopMovementImmediately();
    ProjectileMovement->Deactivate();
    ProjectileMesh->SetSimulatePhysics(false);
    ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    StopFlightTrail();
    ApplyProjectileMaterial();
}

void ASingijeonProjectileActor::OnUnloaded_Implementation()
{
    bIsLoaded = false;
    SetActorTickEnabled(false);
    ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    StopFlightTrail();
    ApplyProjectileMaterial();
}

void ASingijeonProjectileActor::OnLaunched_Implementation(const FVector Direction, const float Speed)
{
    bIsLoaded = false;
    bWasLaunched = true;
    bDamageApplied = false;
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    ProjectileMesh->SetSimulatePhysics(false);
    if (AActor* LaunchOwner = GetOwner())
    {
        ProjectileMesh->IgnoreActorWhenMoving(LaunchOwner, true);
    }
    ApplyProjectileMaterial();
    ApplyFlightTrailSettings();
    const FVector TipDirection = GetArrowTipDirection();
    const FVector RequestedDirection = Direction.GetSafeNormal();
    const FVector LaunchDirection = RequestedDirection.IsNearlyZero() ? TipDirection : RequestedDirection;
    if (!TipDirection.IsNearlyZero() && !LaunchDirection.IsNearlyZero())
    {
        const FQuat TipAlignment = FQuat::FindBetweenNormals(TipDirection, LaunchDirection);
        SetActorRotation(TipAlignment * GetActorQuat(), ETeleportType::TeleportPhysics);
    }
    ProjectileMovement->bRotationFollowsVelocity = false;
    ProjectileMovement->Velocity = LaunchDirection * Speed;
    ProjectileMovement->Activate(true);
    if (FlightTrailEffect && FlightTrailSystem)
    {
        FlightTrailEffect->Activate(true);
    }
    SetActorTickEnabled(true);
    if (LaunchedLifeSpan > 0.0f)
    {
        SetLifeSpan(LaunchedLifeSpan);
    }
}

void ASingijeonProjectileActor::RefreshLoadedVisual_Implementation()
{
    if (bIsLoaded)
    {
        ApplyProjectileMaterial();
    }
}

void ASingijeonProjectileActor::HandleProjectileHit(
    UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, FVector,
    const FHitResult& Hit)
{
    if (!bWasLaunched || !ApplyImpactDamage(OtherActor))
    {
        return;
    }
    if (ProjectileMovement)
    {
        ProjectileMovement->StopMovementImmediately();
        ProjectileMovement->Deactivate();
    }
    StopFlightTrail();
    SetActorTickEnabled(false);
    SetLifeSpan(2.0f);
}

FVector ASingijeonProjectileActor::GetArrowTipDirection() const
{
    if (!ProjectileMesh)
    {
        return -GetActorForwardVector();
    }

    const FVector LocalTipAxis = ArrowTipLocalAxis.GetSafeNormal();
    return ProjectileMesh->GetComponentTransform().TransformVectorNoScale(
        LocalTipAxis.IsNearlyZero() ? FVector(-1.0f, 0.0f, 0.0f) : LocalTipAxis).GetSafeNormal();
}

bool ASingijeonProjectileActor::ApplyImpactDamage(AActor* OtherActor)
{
    if (!bWasLaunched || bDamageApplied || !IsValid(OtherActor) || OtherActor == this)
    {
        return false;
    }
    ULegacyHealthComponent* Health = OtherActor->FindComponentByClass<ULegacyHealthComponent>();
    if (!Health)
    {
        return false;
    }

    bDamageApplied = true;
    const float HealthBeforeDamage = Health->GetCurrentHealth();
    UGameplayStatics::ApplyDamage(
        OtherActor, FMath::Max(0.0f, ImpactDamage), GetInstigatorController(), this, nullptr);
    if (FMath::IsNearlyEqual(Health->GetCurrentHealth(), HealthBeforeDamage))
    {
        Health->ApplyHealthDamage(FMath::Max(0.0f, ImpactDamage));
    }
    OnProjectileImpact.Broadcast(OtherActor);
    return Health->GetCurrentHealth() < HealthBeforeDamage;
}

void ASingijeonProjectileActor::ApplyProjectileMaterial()
{
    if (!ProjectileMesh)
    {
        return;
    }

    if (!ProjectileMaterialOverride)
    {
        ProjectileMaterialOverride = LoadObject<UMaterialInterface>(nullptr,
            TEXT("/GF_Singijeon/Asset/Arrow/arrowb/Materials/"
                 "M_SingijeonArrow_Runtime.M_SingijeonArrow_Runtime"));
    }
    if (!ProjectileMaterialOverride)
    {
        UE_LOG(LogTemp, Error, TEXT("%s could not resolve its runtime arrow material."), *GetName());
        return;
    }

    const int32 MaterialSlotCount = FMath::Max(1, ProjectileMesh->GetNumMaterials());
    for (int32 MaterialIndex = 0; MaterialIndex < MaterialSlotCount; ++MaterialIndex)
    {
        ProjectileMesh->SetMaterial(MaterialIndex, ProjectileMaterialOverride);
    }
}

void ASingijeonProjectileActor::ApplyFlightTrailSettings()
{
    if (!FlightTrailEffect)
    {
        return;
    }

    FlightTrailEffect->SetAsset(FlightTrailSystem);
    FlightTrailEffect->SetRelativeLocation(FlightTrailRelativeLocation);
    FlightTrailEffect->SetRelativeRotation(FlightTrailRelativeRotation);
    FlightTrailEffect->SetRelativeScale3D(FlightTrailRelativeScale);
}

void ASingijeonProjectileActor::StopFlightTrail()
{
    if (FlightTrailEffect)
    {
        FlightTrailEffect->Deactivate();
    }
}
