#include "Core/Scenario/ScenarioInteractionGuideComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "Core/Scenario/ScenarioInteractableComponent.h"
#include "Core/Scenario/ScenarioInteractionGuideWidget.h"
#include "Core/Scenario/ScenarioManagerComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UScenarioInteractionGuideComponent::UScenarioInteractionGuideComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickInterval = 1.0f / 30.0f;
	GuideWidgetClass = UScenarioInteractionGuideWidget::StaticClass();
}

void UScenarioInteractionGuideComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeGuide();
}

bool UScenarioInteractionGuideComponent::InitializeGuide()
{
	Unbind();
	ScenarioManager = GetOwner() ? GetOwner()->FindComponentByClass<UScenarioManagerComponent>() : nullptr;
	if (!ScenarioManager)
	{
		return false;
	}
	ScenarioManager->OnInteractionRequested.AddUniqueDynamic(this, &ThisClass::HandleInteractionRequested);
	ScenarioManager->OnInteractionStateChanged.AddUniqueDynamic(this, &ThisClass::HandleInteractionStateChanged);
	ScenarioManager->OnScenarioStateChanged.AddUniqueDynamic(this, &ThisClass::HandleScenarioStateChanged);

	// Components added or reinitialized after Scenario start must still present the active guide.
	const FScenarioDebugSnapshot Snapshot = ScenarioManager->GetDebugSnapshot();
	if (Snapshot.InteractionState == EScenarioInteractionState::Running)
	{
		HandleInteractionRequested(ScenarioManager->GetCurrentInteraction());
	}
	return true;
}

void UScenarioInteractionGuideComponent::HandleInteractionRequested(FScenarioInteraction Interaction)
{
	HideGuide();
	const EScenarioGuideAction Action = ResolveGuideAction(Interaction);
	if (!bEnableInteractionGuides || Action == EScenarioGuideAction::Hidden || Interaction.TargetID.IsNone())
	{
		return;
	}

	ActiveInteraction = Interaction;
	ActiveInteractionID = Interaction.InteractionID;
	bHasActiveInteraction = true;
	SetComponentTickInterval(FMath::Max(0.0f, ActiveGuideUpdateInterval));
	SetComponentTickEnabled(true);
	TryShowActiveGuide();
}

bool UScenarioInteractionGuideComponent::TryShowActiveGuide()
{
	if (!bHasActiveInteraction)
	{
		return false;
	}

	UScenarioInteractableComponent* ResolvedInteractor = nullptr;
	TargetActor = ResolveTargetActor(ActiveInteraction, ResolvedInteractor);
	TargetInteractor = ResolvedInteractor;
	if (!TargetActor)
	{
		return false;
	}

	EnsureWidgetComponent();
	if (!GuideWidgetComponent)
	{
		return false;
	}
	GuideWidgetComponent->SetWidgetClass(GuideWidgetClass);
	GuideWidgetComponent->InitWidget();
	if (UScenarioInteractionGuideWidget* Widget =
		Cast<UScenarioInteractionGuideWidget>(GuideWidgetComponent->GetUserWidgetObject()))
	{
		const EScenarioGuideAction Action = ResolveGuideAction(ActiveInteraction);
		const FText Instruction = ActiveInteraction.GuideText.IsEmpty()
			? GetDefaultInstruction(Action) : ActiveInteraction.GuideText;
		Widget->SetGuide(GetDefaultActionLabel(Action), Instruction,
			Action == EScenarioGuideAction::Trigger || Action == EScenarioGuideAction::Combat
				? FLinearColor(1.0f, 0.38f, 0.12f, 1.0f)
				: FLinearColor(0.15f, 0.78f, 1.0f, 1.0f));
	}
	GuideWidgetComponent->SetHiddenInGame(false, true);
	GuideWidgetComponent->SetVisibility(true, true);
	UpdateGuideTransform();
	return true;
}

void UScenarioInteractionGuideComponent::HandleInteractionStateChanged(
	FScenarioInteraction Interaction, const EScenarioInteractionState State)
{
	if (Interaction.InteractionID == ActiveInteractionID &&
		(State == EScenarioInteractionState::Completed ||
		 State == EScenarioInteractionState::Failed ||
		 State == EScenarioInteractionState::Skipped))
	{
		HideGuide();
	}
}

void UScenarioInteractionGuideComponent::HandleScenarioStateChanged(
	EScenarioState, const EScenarioState NewState)
{
	if (NewState != EScenarioState::Running)
	{
		HideGuide();
	}
}

AActor* UScenarioInteractionGuideComponent::ResolveTargetActor(
	const FScenarioInteraction& Interaction,
	UScenarioInteractableComponent*& OutInteractor) const
{
	OutInteractor = nullptr;
	if (!GetWorld())
	{
		return nullptr;
	}
	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	const FVector PlayerLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;
	AActor* BestMatch = nullptr;
	UScenarioInteractableComponent* BestInteractor = nullptr;
	float BestDistanceSquared = TNumericLimits<float>::Max();
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		TInlineComponentArray<UScenarioInteractableComponent*> Interactors;
		It->GetComponents(Interactors);
		UScenarioInteractableComponent* MatchingInteractor = nullptr;
		for (const UScenarioInteractableComponent* Interactor : Interactors)
		{
			if (IsValid(Interactor) && Interactor->TargetID == Interaction.TargetID &&
				Interactor->SupportsInteractionType(Interaction.InteractionType))
			{
				MatchingInteractor = const_cast<UScenarioInteractableComponent*>(Interactor);
				break;
			}
		}
		if (!MatchingInteractor)
		{
			continue;
		}
		const float DistanceSquared = FVector::DistSquared(It->GetActorLocation(), PlayerLocation);
		if (!BestMatch || DistanceSquared < BestDistanceSquared)
		{
			BestMatch = *It;
			BestInteractor = MatchingInteractor;
			BestDistanceSquared = DistanceSquared;
		}
	}
	OutInteractor = BestInteractor;
	return BestMatch;
}

void UScenarioInteractionGuideComponent::EnsureWidgetComponent()
{
	if (GuideWidgetComponent || !GetOwner())
	{
		return;
	}
	GuideWidgetComponent = NewObject<UWidgetComponent>(GetOwner(), TEXT("RuntimeScenarioInteractionGuide"));
	GuideWidgetComponent->RegisterComponent();
	GuideWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	GuideWidgetComponent->SetDrawSize(DrawSize);
	GuideWidgetComponent->SetPivot(FVector2D(0.5f, 1.0f));
	GuideWidgetComponent->SetRelativeScale3D(FVector(WorldScale));
	GuideWidgetComponent->SetBlendMode(EWidgetBlendMode::Transparent);
	GuideWidgetComponent->SetTwoSided(true);
	GuideWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GuideWidgetComponent->SetTranslucentSortPriority(10000);
	GuideWidgetComponent->SetHiddenInGame(true, true);
	GuideWidgetComponent->SetVisibility(false);
}

void UScenarioInteractionGuideComponent::UpdateGuideTransform()
{
	if (!GuideWidgetComponent || !IsValid(TargetActor))
	{
		TargetActor = nullptr;
		TargetInteractor = nullptr;
		if (GuideWidgetComponent)
		{
			GuideWidgetComponent->SetHiddenInGame(true, true);
			GuideWidgetComponent->SetVisibility(false, true);
		}
		return;
	}
	FVector GuideLocation;
	if (IsValid(TargetInteractor))
	{
		GuideLocation = TargetInteractor->GetGuideAnchorWorldLocation();
	}
	else
	{
		FVector Origin;
		FVector Extent;
		TargetActor->GetActorBounds(false, Origin, Extent, true);
		GuideLocation = Origin + FVector(0.0f, 0.0f, Extent.Z + HeightOffset);
	}
	GuideWidgetComponent->SetWorldLocation(GuideLocation);

	FVector ViewLocation;
	if (ResolveViewLocation(ViewLocation))
	{
		const float Distance = FVector::Distance(ViewLocation, GuideLocation);
		const float AdjustedScale = bScaleWithViewDistance
			? CalculateDistanceAdjustedScale(
				WorldScale, Distance, ScaleReferenceDistance, MaximumWorldScale)
			: WorldScale;
		GuideWidgetComponent->SetWorldScale3D(FVector(AdjustedScale));
		GuideWidgetComponent->SetWorldRotation(
			CalculateGuideFacingRotation(GuideLocation, ViewLocation));
	}
}

bool UScenarioInteractionGuideComponent::ResolveViewLocation(FVector& OutViewLocation) const
{
	if (const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		TInlineComponentArray<UCameraComponent*> Cameras(PlayerPawn);
		for (const UCameraComponent* Camera : Cameras)
		{
			if (IsValid(Camera) && Camera->IsActive())
			{
				OutViewLocation = Camera->GetComponentLocation();
				return true;
			}
		}
		if (!Cameras.IsEmpty() && IsValid(Cameras[0]))
		{
			OutViewLocation = Cameras[0]->GetComponentLocation();
			return true;
		}
	}

	if (const APlayerController* Controller = UGameplayStatics::GetPlayerController(this, 0))
	{
		FRotator ViewRotation;
		Controller->GetPlayerViewPoint(OutViewLocation, ViewRotation);
		return true;
	}
	return false;
}

FRotator UScenarioInteractionGuideComponent::CalculateGuideFacingRotation(
	const FVector& GuideLocation, const FVector& ViewLocation)
{
	FRotator FacingRotation = (ViewLocation - GuideLocation).Rotation();
	// WidgetComponent faces along local +X. Preserve Pitch so close, low targets
	// such as the Hwacha fuse remain perpendicular to the HMD view instead of
	// appearing skewed below the player. Roll stays locked to prevent text tilt.
	FacingRotation.Roll = 0.0f;
	return FacingRotation;
}

float UScenarioInteractionGuideComponent::CalculateDistanceAdjustedScale(
	const float BaseScale,
	const float Distance,
	const float ReferenceDistance,
	const float MaximumScale)
{
	const float SafeBaseScale = FMath::Max(BaseScale, 0.001f);
	const float SafeReferenceDistance = FMath::Max(ReferenceDistance, 1.0f);
	const float SafeMaximumScale = FMath::Max(MaximumScale, SafeBaseScale);
	return FMath::Clamp(
		SafeBaseScale * FMath::Max(Distance, SafeReferenceDistance) / SafeReferenceDistance,
		SafeBaseScale,
		SafeMaximumScale);
}

void UScenarioInteractionGuideComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	RefreshGuide();
}

void UScenarioInteractionGuideComponent::RefreshGuide()
{
	if (bHasActiveInteraction && (!IsValid(TargetActor) || !IsGuideVisible()))
	{
		TryShowActiveGuide();
	}
	UpdateGuideTransform();
}

void UScenarioInteractionGuideComponent::HideGuide()
{
	if (GuideWidgetComponent)
	{
		GuideWidgetComponent->SetHiddenInGame(true, true);
		GuideWidgetComponent->SetVisibility(false, true);
	}
	TargetActor = nullptr;
	TargetInteractor = nullptr;
	bHasActiveInteraction = false;
	ActiveInteraction = FScenarioInteraction();
	ActiveInteractionID = NAME_None;
	SetComponentTickEnabled(false);
}

bool UScenarioInteractionGuideComponent::IsGuideVisible() const
{
	return GuideWidgetComponent && GuideWidgetComponent->IsVisible();
}

EScenarioGuideAction UScenarioInteractionGuideComponent::ResolveGuideAction(
	const FScenarioInteraction& Interaction)
{
	if (Interaction.GuideAction != EScenarioGuideAction::Auto)
	{
		return Interaction.GuideAction;
	}
	switch (Interaction.InteractionType)
	{
	case EScenarioInteractionType::Grab: return EScenarioGuideAction::Grab;
	case EScenarioInteractionType::Press: return EScenarioGuideAction::Press;
	case EScenarioInteractionType::Observe: return EScenarioGuideAction::Observe;
	case EScenarioInteractionType::Move: return EScenarioGuideAction::Drag;
	case EScenarioInteractionType::Trigger: return EScenarioGuideAction::Trigger;
	case EScenarioInteractionType::Quiz: return EScenarioGuideAction::Select;
	case EScenarioInteractionType::VoiceCommand: return EScenarioGuideAction::Speak;
	case EScenarioInteractionType::Combat: return EScenarioGuideAction::Combat;
	case EScenarioInteractionType::Custom: return EScenarioGuideAction::Interact;
	default: return EScenarioGuideAction::Hidden;
	}
}

FText UScenarioInteractionGuideComponent::GetDefaultActionLabel(const EScenarioGuideAction Action)
{
	switch (Action)
	{
	case EScenarioGuideAction::Grab: return FText::FromString(TEXT("TRIGGER"));
	case EScenarioGuideAction::Drag: return FText::FromString(TEXT("HOLD + MOVE"));
	case EScenarioGuideAction::Trigger: return FText::FromString(TEXT("TRIGGER"));
	case EScenarioGuideAction::Press: return FText::FromString(TEXT("PRESS"));
	case EScenarioGuideAction::Observe: return FText::FromString(TEXT("LOOK"));
	case EScenarioGuideAction::Select: return FText::FromString(TEXT("SELECT"));
	case EScenarioGuideAction::Speak: return FText::FromString(TEXT("SPEAK"));
	case EScenarioGuideAction::Combat: return FText::FromString(TEXT("AIM + TRIGGER"));
	case EScenarioGuideAction::Interact: return FText::FromString(TEXT("INTERACT"));
	default: return FText::GetEmpty();
	}
}

FText UScenarioInteractionGuideComponent::GetDefaultInstruction(const EScenarioGuideAction Action)
{
	switch (Action)
	{
	case EScenarioGuideAction::Grab: return FText::FromString(TEXT("트리거를 눌러 잡기"));
	case EScenarioGuideAction::Drag: return FText::FromString(TEXT("트리거를 누른 채 이동"));
	case EScenarioGuideAction::Trigger: return FText::FromString(TEXT("트리거로 작동"));
	case EScenarioGuideAction::Press: return FText::FromString(TEXT("버튼 누르기"));
	case EScenarioGuideAction::Observe: return FText::FromString(TEXT("대상을 바라보기"));
	case EScenarioGuideAction::Select: return FText::FromString(TEXT("항목 선택"));
	case EScenarioGuideAction::Speak: return FText::FromString(TEXT("내용 말하기"));
	case EScenarioGuideAction::Combat: return FText::FromString(TEXT("조준하고 트리거로 발사"));
	case EScenarioGuideAction::Interact: return FText::FromString(TEXT("대상과 상호작용"));
	default: return FText::GetEmpty();
	}
}

void UScenarioInteractionGuideComponent::Unbind()
{
	if (ScenarioManager)
	{
		ScenarioManager->OnInteractionRequested.RemoveDynamic(this, &ThisClass::HandleInteractionRequested);
		ScenarioManager->OnInteractionStateChanged.RemoveDynamic(this, &ThisClass::HandleInteractionStateChanged);
		ScenarioManager->OnScenarioStateChanged.RemoveDynamic(this, &ThisClass::HandleScenarioStateChanged);
	}
	ScenarioManager = nullptr;
}

void UScenarioInteractionGuideComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	HideGuide();
	Unbind();
	Super::EndPlay(EndPlayReason);
}
