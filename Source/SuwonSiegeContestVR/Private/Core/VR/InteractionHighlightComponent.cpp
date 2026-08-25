#include "Core/VR/InteractionHighlightComponent.h"

#include "Components/MeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

namespace
{
	const TCHAR* DefaultHighlightMaterialPath = TEXT("/Game/Core/VR/Interaction/MI_InteractionHighlight_Blue.MI_InteractionHighlight_Blue");
	const TCHAR* DefaultHighlightEffectPath = TEXT("/Game/NiagaraExamples/FX_PickUp/NS_Pickup_Idle.NS_Pickup_Idle");
	const FName HighlightColorParameter(TEXT("HighlightColor"));
	const FName HighlightIntensityParameter(TEXT("Intensity"));
}

UInteractionHighlightComponent::UInteractionHighlightComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	HighlightMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(DefaultHighlightMaterialPath));
	HighlightEffect = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(DefaultHighlightEffectPath));
}

void UInteractionHighlightComponent::BeginPlay()
{
	Super::BeginPlay();
	CollectOwnerMeshes();
}

void UInteractionHighlightComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// A pooled Actor keeps its components, so a highlight left on would follow it back into play.
	if (bHighlightActive)
	{
		SetHighlightActive(false);
	}
	Super::EndPlay(EndPlayReason);
}

void UInteractionHighlightComponent::CollectOwnerMeshes()
{
	if (!bHighlightOwnerMeshes)
	{
		return;
	}

	const AActor* OwningActor = GetOwner();
	if (!OwningActor)
	{
		return;
	}

	TInlineComponentArray<UMeshComponent*> MeshComponents(OwningActor);
	for (UMeshComponent* MeshComponent : MeshComponents)
	{
		AddHighlightMesh(MeshComponent);
	}
}

void UInteractionHighlightComponent::AddHighlightMesh(UMeshComponent* MeshComponent)
{
	if (IsValid(MeshComponent) && !HighlightMeshes.Contains(MeshComponent))
	{
		HighlightMeshes.Add(MeshComponent);
		if (bHighlightActive && HighlightMaterialInstance)
		{
			MeshComponent->SetOverlayMaterial(HighlightMaterialInstance);
		}
	}
}

void UInteractionHighlightComponent::ConfigureHighlight(const bool bInHighlightOwnerMeshes, const FLinearColor InHighlightColor)
{
	bHighlightOwnerMeshes = bInHighlightOwnerMeshes;
	SetHighlightColor(InHighlightColor);
}

void UInteractionHighlightComponent::SetHighlightActive(const bool bActive)
{
	if (bHighlightActive == bActive)
	{
		return;
	}

	bHighlightActive = bActive;
	ApplyOverlayMaterial(bActive);
	ApplyHighlightEffect(bActive);
}

void UInteractionHighlightComponent::SetHighlightColor(const FLinearColor NewColor)
{
	HighlightColor = NewColor;
	if (HighlightMaterialInstance)
	{
		HighlightMaterialInstance->SetVectorParameterValue(HighlightColorParameter, HighlightColor);
	}
}

void UInteractionHighlightComponent::ApplyOverlayMaterial(const bool bActive)
{
	if (bActive && !HighlightMaterialInstance)
	{
		if (UMaterialInterface* LoadedMaterial = HighlightMaterial.LoadSynchronous())
		{
			HighlightMaterialInstance = UMaterialInstanceDynamic::Create(LoadedMaterial, this);
			if (HighlightMaterialInstance)
			{
				HighlightMaterialInstance->SetVectorParameterValue(HighlightColorParameter, HighlightColor);
				HighlightMaterialInstance->SetScalarParameterValue(HighlightIntensityParameter, HighlightIntensity);
			}
		}
	}

	for (int32 Index = HighlightMeshes.Num() - 1; Index >= 0; --Index)
	{
		UMeshComponent* MeshComponent = HighlightMeshes[Index].Get();
		if (!IsValid(MeshComponent))
		{
			HighlightMeshes.RemoveAtSwap(Index);
			continue;
		}
		MeshComponent->SetOverlayMaterial(bActive ? HighlightMaterialInstance : nullptr);
	}
}

void UInteractionHighlightComponent::ApplyHighlightEffect(const bool bActive)
{
	if (!bActive)
	{
		if (HighlightEffectComponent)
		{
			HighlightEffectComponent->Deactivate();
			HighlightEffectComponent->SetVisibility(false);
		}
		return;
	}

	if (!HighlightEffectComponent)
	{
		UNiagaraSystem* LoadedEffect = HighlightEffect.LoadSynchronous();
		if (!LoadedEffect)
		{
			return;
		}
		// Looping systems must not use the component pool: they never report completion.
		HighlightEffectComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			LoadedEffect,
			this,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			/*bAutoDestroy=*/false,
			/*bAutoActivate=*/false,
			ENCPoolMethod::None,
			/*bPreCullCheck=*/true);
		if (HighlightEffectComponent)
		{
			HighlightEffectComponent->SetRelativeScale3D(HighlightEffectScale);
		}
	}

	if (HighlightEffectComponent)
	{
		HighlightEffectComponent->SetVisibility(true);
		HighlightEffectComponent->Activate(true);
	}
}
