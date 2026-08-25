#pragma once

#include "ComponentVisualizer.h"

class FScopedTransaction;
class UScenarioInteractableComponent;

struct HScenarioGuideAnchorProxy final : public HComponentVisProxy
{
	DECLARE_HIT_PROXY();

	explicit HScenarioGuideAnchorProxy(const UActorComponent* Component)
		: HComponentVisProxy(Component, HPP_Foreground)
	{
	}
};

/** Clickable world-space guide anchor used to author Scenario guide placement. */
class FScenarioGuideComponentVisualizer final : public FComponentVisualizer
{
public:
	virtual void DrawVisualization(
		const UActorComponent* Component,
		const FSceneView* View,
		FPrimitiveDrawInterface* PDI) override;
	virtual bool VisProxyHandleClick(
		FEditorViewportClient* ViewportClient,
		HComponentVisProxy* VisProxy,
		const FViewportClick& Click) override;
	virtual void EndEditing() override;
	virtual bool GetWidgetLocation(
		const FEditorViewportClient* ViewportClient,
		FVector& OutLocation) const override;
	virtual bool HandleInputDelta(
		FEditorViewportClient* ViewportClient,
		FViewport* Viewport,
		FVector& DeltaTranslate,
		FRotator& DeltaRotate,
		FVector& DeltaScale) override;
	virtual void TrackingStarted(FEditorViewportClient* ViewportClient) override;
	virtual void TrackingStopped(FEditorViewportClient* ViewportClient, bool bDidMove) override;
	virtual UActorComponent* GetEditedComponent() const override;

private:
	UScenarioInteractableComponent* GetEditedScenarioComponent() const;

	FComponentPropertyPath EditedComponentPath;
	TUniquePtr<FScopedTransaction> ActiveTransaction;
};
