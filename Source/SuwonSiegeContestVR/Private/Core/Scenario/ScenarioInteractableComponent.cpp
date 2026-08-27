#include "Core/Scenario/ScenarioInteractableComponent.h"

#include "Core/Scenario/ScenarioManagerComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

namespace
{
	FVector GetScenarioGuideBoundsTop(const AActor* Owner)
	{
		if (!Owner)
		{
			return FVector::ZeroVector;
		}

		FBox Bounds(ForceInit);
		TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents(Owner);
		for (const UPrimitiveComponent* Primitive : PrimitiveComponents)
		{
		if (Primitive && Primitive->IsRegistered()
#if WITH_EDITORONLY_DATA
			&& !Primitive->IsVisualizationComponent()
#endif
		)
			{
				Bounds += Primitive->Bounds.GetBox();
			}
		}
		return Bounds.IsValid
			? FVector(Bounds.GetCenter().X, Bounds.GetCenter().Y, Bounds.Max.Z)
			: Owner->GetActorLocation();
	}
}

UScenarioInteractableComponent::UScenarioInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FVector UScenarioInteractableComponent::GetGuideAnchorWorldLocation() const
{
	if (!GetOwner())
	{
		return GuideAnchorOffset;
	}
	return GetScenarioGuideBoundsTop(GetOwner()) + GuideAnchorOffset;
}

void UScenarioInteractableComponent::SetGuideAnchorWorldLocation(const FVector& WorldLocation)
{
	if (GetOwner())
	{
		GuideAnchorOffset = WorldLocation - GetScenarioGuideBoundsTop(GetOwner());
	}
	else
	{
		GuideAnchorOffset = WorldLocation;
	}
	RefreshEditorGuideAnchor();
}

void UScenarioInteractableComponent::OnRegister()
{
	Super::OnRegister();
	RefreshEditorGuideAnchor();
}

void UScenarioInteractableComponent::OnUnregister()
{
#if WITH_EDITORONLY_DATA
	if (EditorGuideAnchor)
	{
		EditorGuideAnchor->DestroyComponent();
		EditorGuideAnchor = nullptr;
	}
#endif
	Super::OnUnregister();
}

#if WITH_EDITOR
void UScenarioInteractableComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	RefreshEditorGuideAnchor();
}
#endif

void UScenarioInteractableComponent::RefreshEditorGuideAnchor()
{
#if WITH_EDITORONLY_DATA
	AActor* Owner = GetOwner();
	if (!GIsEditor || !Owner || (GetWorld() && GetWorld()->IsGameWorld()))
	{
		return;
	}

	if (!EditorGuideAnchor)
	{
		EditorGuideAnchor = NewObject<UArrowComponent>(
			Owner, NAME_None, RF_Transient | RF_TextExportTransient);
		if (USceneComponent* Root = Owner->GetRootComponent())
		{
			EditorGuideAnchor->SetupAttachment(Root);
		}
		EditorGuideAnchor->ArrowColor = FColor(255, 210, 32);
		EditorGuideAnchor->ArrowLength = 32.0f;
		EditorGuideAnchor->ArrowSize = 1.25f;
		EditorGuideAnchor->bIsScreenSizeScaled = true;
		EditorGuideAnchor->SetIsVisualizationComponent(true);
		EditorGuideAnchor->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		EditorGuideAnchor->SetHiddenInGame(true);
		EditorGuideAnchor->RegisterComponent();
	}

	EditorGuideAnchor->SetVisibility(bShowGuideAnchorInEditor);
	EditorGuideAnchor->SetWorldLocation(GetGuideAnchorWorldLocation());
	EditorGuideAnchor->SetWorldRotation(FRotator(90.0f, 0.0f, 0.0f));
#endif
}

void UScenarioInteractableComponent::SetInteractionEnabled(const bool bEnabled)
{
	bInteractionEnabled = bEnabled;
}

bool UScenarioInteractableComponent::SupportsInteractionType(
	const EScenarioInteractionType InteractionType) const
{
	return SupportedInteractionTypes.Contains(InteractionType);
}

bool UScenarioInteractableComponent::CanReportInteraction(
	const EScenarioInteractionType InteractionType) const
{
	if (!bInteractionEnabled || !SupportsInteractionType(InteractionType) || TargetID.IsNone())
	{
		return false;
	}
	if (bAutoReportToScenarioManager)
	{
		if (const UScenarioManagerComponent* Manager = FindScenarioManager())
		{
			return Manager->CanReportInteractionResult(TargetID, InteractionType);
		}
	}
	return true;
}

bool UScenarioInteractableComponent::ReportInteractionStarted(
	const EScenarioInteractionType InteractionType)
{
	if (!bInteractionEnabled || !SupportsInteractionType(InteractionType) || TargetID.IsNone())
	{
		return false;
	}
	OnInteractionStarted.Broadcast(TargetID, InteractionType);
	return true;
}

bool UScenarioInteractableComponent::ReportInteractionProgress(
	const EScenarioInteractionType InteractionType,
	const float Progress)
{
	if (!bInteractionEnabled || !SupportsInteractionType(InteractionType) || TargetID.IsNone())
	{
		return false;
	}
	OnInteractionProgress.Broadcast(TargetID, InteractionType, FMath::Clamp(Progress, 0.0f, 1.0f));
	return true;
}

bool UScenarioInteractableComponent::ReportInteractionCompleted(
	const EScenarioInteractionType InteractionType)
{
	if (!CanReportInteraction(InteractionType))
	{
		return false;
	}
	if (bAutoReportToScenarioManager)
	{
		if (UScenarioManagerComponent* Manager = FindScenarioManager())
		{
			if (!Manager->ReportInteractionResult(TargetID, InteractionType, true))
			{
				return false;
			}
		}
	}
	OnInteractionCompleted.Broadcast(TargetID, InteractionType);
	return true;
}

bool UScenarioInteractableComponent::ReportInteractionFailed(
	const EScenarioInteractionType InteractionType)
{
	if (!CanReportInteraction(InteractionType))
	{
		return false;
	}
	if (bAutoReportToScenarioManager)
	{
		if (UScenarioManagerComponent* Manager = FindScenarioManager())
		{
			if (!Manager->ReportInteractionResult(TargetID, InteractionType, false))
			{
				return false;
			}
		}
	}
	OnInteractionFailed.Broadcast(TargetID, InteractionType);
	return true;
}

UScenarioManagerComponent* UScenarioInteractableComponent::FindScenarioManager() const
{
	if (!GetWorld())
	{
		return nullptr;
	}

	for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
	{
		if (UScenarioManagerComponent* Manager = ActorIterator->FindComponentByClass<UScenarioManagerComponent>())
		{
			return Manager;
		}
	}
	return nullptr;
}
