#include "Ongseong/ChongtongLoadingItemActor.h"

#include "Components/StaticMeshComponent.h"
#include "MotionControllerComponent.h"
#include "Core/VR/InteractionHighlightComponent.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/StructOnScope.h"

AChongtongLoadingItemActor::AChongtongLoadingItemActor()
{
	PrimaryActorTick.bCanEverTick = false;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	// Match Singijeon's Blueprint GrabPoint path, while also making the whole visible prop a
	// reliable target. A SceneComponent grab point at the mesh pivot is too easy to miss when
	// the player reaches for the side of a powder bag or a cannonball.
	Mesh->ComponentTags.Add(TEXT("VRGrab"));
	LoadingPrompt = CreateDefaultSubobject<UInteractionHighlightComponent>(TEXT("LoadingPrompt"));
	LoadingPrompt->SetupAttachment(Mesh);
	// The placeholder meshes are scaled per item type (the rammer is 24x taller than it is wide),
	// so the prompt effect must not inherit that scale.
	LoadingPrompt->SetUsingAbsoluteScale(true);

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
	// Blueprint children own their visual component. Reapplying the native placeholder here
	// used to overwrite every mesh and scale edit made in the Blueprint viewport.
	if (GetClass() == StaticClass())
	{
		ApplyNativePlaceholderAppearance();
	}
}

void AChongtongLoadingItemActor::ConfigureItem(EChongtongLoadingItemType NewType)
{
	ItemType = NewType;
	if (GetClass() == StaticClass())
	{
		ApplyNativePlaceholderAppearance();
	}
	HomeTransform = GetActorTransform();
	bHomeSimulatingPhysics = Mesh && Mesh->IsSimulatingPhysics();
}

void AChongtongLoadingItemActor::BeginAutomatedUse()
{
	HoldingMotionController.Reset();
	TInlineComponentArray<USceneComponent*> SceneComponents(this);
	for (USceneComponent* Component : SceneComponents)
	{
		if (UFunction* ReleaseFunction = Component ? Component->FindFunction(TEXT("TryRelease")) : nullptr)
		{
			FStructOnScope Parameters(ReleaseFunction);
			Component->ProcessEvent(ReleaseFunction, Parameters.GetStructMemory());
		}
	}
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	if (Mesh)
	{
		Mesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		Mesh->SetSimulatePhysics(false);
	}
	SetActorEnableCollision(false);
}

bool AChongtongLoadingItemActor::HandleVRGrabbed(
	USceneComponent* GrabComponent, UMotionControllerComponent* MotionController)
{
	if (GrabComponent != Mesh)
	{
		// The normal path is the same BP_GrabComponent used by the Singijeon arrow and torch.
		HoldingMotionController = MotionController;
		return true;
	}

	TInlineComponentArray<USceneComponent*> SceneComponents(this);
	for (USceneComponent* Component : SceneComponents)
	{
		if (!Component || Component == Mesh || Component->GetFName() != TEXT("GrabPoint"))
		{
			continue;
		}
		if (UFunction* GrabFunction = Component->FindFunction(TEXT("TryGrab")))
		{
			FStructOnScope Parameters(GrabFunction);
			for (TFieldIterator<FProperty> PropertyIt(GrabFunction); PropertyIt; ++PropertyIt)
			{
				FProperty* Property = *PropertyIt;
				if (!Property->HasAnyPropertyFlags(CPF_Parm) ||
					Property->HasAnyPropertyFlags(CPF_OutParm | CPF_ReturnParm))
				{
					continue;
				}
				if (FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(Property))
				{
					if (MotionController && MotionController->IsA(ObjectProperty->PropertyClass))
					{
						ObjectProperty->SetObjectPropertyValue_InContainer(Parameters.GetStructMemory(), MotionController);
					}
				}
			}
			Component->ProcessEvent(GrabFunction, Parameters.GetStructMemory());
			for (TFieldIterator<FProperty> PropertyIt(GrabFunction); PropertyIt; ++PropertyIt)
			{
				if (PropertyIt->HasAnyPropertyFlags(CPF_ReturnParm))
				{
					if (const FBoolProperty* ReturnProperty = CastField<FBoolProperty>(*PropertyIt))
					{
						if (!ReturnProperty->GetPropertyValue_InContainer(Parameters.GetStructMemory()))
						{
							return false;
						}
					}
					break;
				}
			}
			ForwardedGrabComponent = Component;
			HoldingMotionController = MotionController;
			return true;
		}
	}

	return false;
}

void AChongtongLoadingItemActor::HandleVRReleased(
	USceneComponent* GrabComponent, UMotionControllerComponent* MotionController)
{
	HoldingMotionController.Reset();
	if (GrabComponent != Mesh || !ForwardedGrabComponent.IsValid())
	{
		return;
	}

	USceneComponent* ForwardedComponent = ForwardedGrabComponent.Get();
	if (UFunction* ReleaseFunction = ForwardedComponent->FindFunction(TEXT("TryRelease")))
	{
		FStructOnScope Parameters(ReleaseFunction);
		for (TFieldIterator<FProperty> PropertyIt(ReleaseFunction); PropertyIt; ++PropertyIt)
		{
			FProperty* Property = *PropertyIt;
			if (!Property->HasAnyPropertyFlags(CPF_Parm) ||
				Property->HasAnyPropertyFlags(CPF_OutParm | CPF_ReturnParm))
			{
				continue;
			}
			if (FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(Property))
			{
				if (MotionController && MotionController->IsA(ObjectProperty->PropertyClass))
				{
					ObjectProperty->SetObjectPropertyValue_InContainer(Parameters.GetStructMemory(), MotionController);
				}
			}
		}
		ForwardedComponent->ProcessEvent(ReleaseFunction, Parameters.GetStructMemory());
	}
	ForwardedGrabComponent.Reset();
}

void AChongtongLoadingItemActor::ApplyNativePlaceholderAppearance()
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

void AChongtongLoadingItemActor::SetLoadingPromptActive(const bool bActive)
{
	if (LoadingPrompt)
	{
		LoadingPrompt->SetHighlightActive(bActive && !IsHidden());
	}
}

bool AChongtongLoadingItemActor::IsLoadingPromptActive() const
{
	return LoadingPrompt && LoadingPrompt->IsHighlightActive();
}

void AChongtongLoadingItemActor::ConsumeAndRespawn(float DelaySeconds)
{
	HoldingMotionController.Reset();
	SetLoadingPromptActive(false);
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

FVector AChongtongLoadingItemActor::GetInteractionLocation() const
{
	return HoldingMotionController.IsValid()
		? HoldingMotionController->GetComponentLocation()
		: GetActorLocation();
}

bool AChongtongLoadingItemActor::IsHeldForInteraction() const
{
	return HoldingMotionController.IsValid();
}

void AChongtongLoadingItemActor::RespawnAtHome()
{
	SetActorTransform(HomeTransform, false, nullptr, ETeleportType::TeleportPhysics);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	if (Mesh)
	{
		Mesh->SetSimulatePhysics(bHomeSimulatingPhysics);
	}
}
