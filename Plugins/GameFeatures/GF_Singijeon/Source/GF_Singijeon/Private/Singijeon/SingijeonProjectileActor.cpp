#include "Singijeon/SingijeonProjectileActor.h"

#include "Components/StaticMeshComponent.h"
#include "Core/Scenario/ScenarioInteractableComponent.h"
#include "Core/Scenario/ScenarioTypes.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "UObject/StructOnScope.h"

ASingijeonProjectileActor::ASingijeonProjectileActor()
{
    PrimaryActorTick.bCanEverTick = false;

    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
    SetRootComponent(ProjectileMesh);
    ProjectileMesh->SetCollisionProfileName(TEXT("PhysicsActor"));

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = ProjectileMesh;
    ProjectileMovement->InitialSpeed = 0.0f;
    ProjectileMovement->MaxSpeed = 10000.0f;
    ProjectileMovement->ProjectileGravityScale = 1.0f;
    ProjectileMovement->bAutoActivate = false;
    ProjectileMovement->bRotationFollowsVelocity = true;

    GrabScenarioInteractor = CreateDefaultSubobject<UScenarioInteractableComponent>(TEXT("GrabScenarioInteractor"));
    GrabScenarioInteractor->TargetID = TEXT("Singijeon_Ammo");
    GrabScenarioInteractor->SupportedInteractionTypes = { EScenarioInteractionType::Grab };
}

bool ASingijeonProjectileActor::CanBeLoaded_Implementation() const
{
    return !bIsLoaded && !bWasLaunched;
}

bool ASingijeonProjectileActor::PrepareForLoading_Implementation()
{
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
    ProjectileMovement->StopMovementImmediately();
    ProjectileMovement->Deactivate();
    ProjectileMesh->SetSimulatePhysics(false);
    ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASingijeonProjectileActor::OnUnloaded_Implementation()
{
    bIsLoaded = false;
    ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void ASingijeonProjectileActor::OnLaunched_Implementation(const FVector Direction, const float Speed)
{
    bIsLoaded = false;
    bWasLaunched = true;
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    ProjectileMesh->SetSimulatePhysics(false);
    ProjectileMovement->Velocity = Direction.GetSafeNormal() * Speed;
    ProjectileMovement->Activate(true);
}
