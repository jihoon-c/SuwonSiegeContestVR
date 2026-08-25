#include "ScenarioGuideComponentVisualizer.h"

#include "Core/Scenario/ScenarioInteractableComponent.h"
#include "GameFramework/Actor.h"
#include "Math/RotationMatrix.h"
#include "PrimitiveDrawingUtils.h"
#include "ScopedTransaction.h"
#include "UObject/UnrealType.h"

IMPLEMENT_HIT_PROXY(HScenarioGuideAnchorProxy, HComponentVisProxy);

#define LOCTEXT_NAMESPACE "ScenarioGuideComponentVisualizer"

void FScenarioGuideComponentVisualizer::DrawVisualization(
	const UActorComponent* Component,
	const FSceneView* View,
	FPrimitiveDrawInterface* PDI)
{
	const UScenarioInteractableComponent* ScenarioComponent =
		Cast<const UScenarioInteractableComponent>(Component);
	if (!ScenarioComponent || !ScenarioComponent->bShowGuideAnchorInEditor)
	{
		return;
	}

	const FVector Anchor = ScenarioComponent->GetGuideAnchorWorldLocation();
	const FVector BoundsTop = Anchor - ScenarioComponent->GuideAnchorOffset;
	const bool bSelected = GetEditedScenarioComponent() == ScenarioComponent;
	const FColor Color = bSelected ? FColor::White : FColor(255, 210, 32);

	PDI->SetHitProxy(new HScenarioGuideAnchorProxy(Component));
	PDI->DrawLine(BoundsTop, Anchor, Color, SDPG_Foreground, 1.5f);
	PDI->DrawPoint(Anchor, Color, 18.0f, SDPG_Foreground);
	const FMatrix ArrowTransform = FTransform(
		FRotationMatrix::MakeFromX(FVector::UpVector).ToQuat(), Anchor).ToMatrixNoScale();
	DrawDirectionalArrow(PDI, ArrowTransform, Color, 36.0f, 1.0f, SDPG_Foreground, 2.0f);
	PDI->SetHitProxy(nullptr);
}

bool FScenarioGuideComponentVisualizer::VisProxyHandleClick(
	FEditorViewportClient* ViewportClient,
	HComponentVisProxy* VisProxy,
	const FViewportClick& Click)
{
	if (!VisProxy || !VisProxy->IsA(HScenarioGuideAnchorProxy::StaticGetType()) ||
		!VisProxy->Component.IsValid())
	{
		return false;
	}

	const UScenarioInteractableComponent* Component =
		Cast<UScenarioInteractableComponent>(VisProxy->Component.Get());
	if (!Component)
	{
		return false;
	}
	EditedComponentPath = FComponentPropertyPath(Component);
	return EditedComponentPath.IsValid();
}

void FScenarioGuideComponentVisualizer::EndEditing()
{
	ActiveTransaction.Reset();
	EditedComponentPath.Reset();
}

bool FScenarioGuideComponentVisualizer::GetWidgetLocation(
	const FEditorViewportClient* ViewportClient,
	FVector& OutLocation) const
{
	if (const UScenarioInteractableComponent* Component = GetEditedScenarioComponent())
	{
		OutLocation = Component->GetGuideAnchorWorldLocation();
		return true;
	}
	return false;
}

bool FScenarioGuideComponentVisualizer::HandleInputDelta(
	FEditorViewportClient* ViewportClient,
	FViewport* Viewport,
	FVector& DeltaTranslate,
	FRotator& DeltaRotate,
	FVector& DeltaScale)
{
	UScenarioInteractableComponent* Component = GetEditedScenarioComponent();
	if (!Component || DeltaTranslate.IsNearlyZero())
	{
		return false;
	}

	Component->Modify();
	Component->SetGuideAnchorWorldLocation(
		Component->GetGuideAnchorWorldLocation() + DeltaTranslate);
	if (FProperty* Property = FindFProperty<FProperty>(
		UScenarioInteractableComponent::StaticClass(),
		GET_MEMBER_NAME_CHECKED(UScenarioInteractableComponent, GuideAnchorOffset)))
	{
		NotifyPropertyModified(Component, Property, EPropertyChangeType::Interactive);
	}
	return true;
}

void FScenarioGuideComponentVisualizer::TrackingStarted(FEditorViewportClient* ViewportClient)
{
	if (GetEditedScenarioComponent() && !ActiveTransaction)
	{
		ActiveTransaction = MakeUnique<FScopedTransaction>(
			LOCTEXT("MoveScenarioGuideAnchor", "Move Scenario Guide Anchor"));
	}
}

void FScenarioGuideComponentVisualizer::TrackingStopped(
	FEditorViewportClient* ViewportClient,
	const bool bDidMove)
{
	if (UScenarioInteractableComponent* Component = GetEditedScenarioComponent())
	{
		if (bDidMove)
		{
			if (FProperty* Property = FindFProperty<FProperty>(
				UScenarioInteractableComponent::StaticClass(),
				GET_MEMBER_NAME_CHECKED(UScenarioInteractableComponent, GuideAnchorOffset)))
			{
				NotifyPropertyModified(Component, Property, EPropertyChangeType::ValueSet);
			}
		}
	}
	ActiveTransaction.Reset();
}

UActorComponent* FScenarioGuideComponentVisualizer::GetEditedComponent() const
{
	return EditedComponentPath.GetComponent();
}

UScenarioInteractableComponent*
FScenarioGuideComponentVisualizer::GetEditedScenarioComponent() const
{
	return Cast<UScenarioInteractableComponent>(EditedComponentPath.GetComponent());
}

#undef LOCTEXT_NAMESPACE
