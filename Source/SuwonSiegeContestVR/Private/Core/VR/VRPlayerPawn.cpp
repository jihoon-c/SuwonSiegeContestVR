#include "Core/VR/VRPlayerPawn.h"

#include "Blueprint/UserWidget.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedActionKeyMapping.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Core/Narration/NarrationSequenceComponent.h"
#include "Core/Scenario/ScenarioInteractableComponent.h"
#include "Core/Scenario/ScenarioTypes.h"
#include "Core/Narration/SubtitleWidget.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "NavigationSystem.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/Field.h"
#include "UObject/StructOnScope.h"

AVRPlayerPawn::AVRPlayerPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	VROrigin->SetupAttachment(Root);

	VRCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	VRCamera->SetupAttachment(VROrigin);
	VRCamera->bLockToHmd = true;

	MotionControllerLeftGrip = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionControllerLeftGrip"));
	MotionControllerLeftGrip->SetupAttachment(VROrigin);
	MotionControllerLeftGrip->SetTrackingMotionSource(TEXT("LeftGrip"));

	MotionControllerLeftAim = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionControllerLeftAim"));
	MotionControllerLeftAim->SetupAttachment(VROrigin);
	MotionControllerLeftAim->SetTrackingMotionSource(TEXT("LeftAim"));

	MotionControllerRightGrip = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionControllerRightGrip"));
	MotionControllerRightGrip->SetupAttachment(VROrigin);
	MotionControllerRightGrip->SetTrackingMotionSource(TEXT("RightGrip"));

	MotionControllerRightAim = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionControllerRightAim"));
	MotionControllerRightAim->SetupAttachment(VROrigin);
	MotionControllerRightAim->SetTrackingMotionSource(TEXT("RightAim"));

	LeftHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LeftHandMesh"));
	LeftHandMesh->SetupAttachment(MotionControllerLeftGrip);
	LeftHandMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftHandMesh->SetGenerateOverlapEvents(false);
	LeftHandMesh->SetCastShadow(false);
	LeftHandMesh->SetRelativeLocation(FVector(-2.981260f, -3.5f, 4.561753f));
	LeftHandMesh->SetRelativeRotation(FRotator(-25.0f, -180.0f, 90.0f));

	RightHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RightHandMesh"));
	RightHandMesh->SetupAttachment(MotionControllerRightGrip);
	RightHandMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightHandMesh->SetGenerateOverlapEvents(false);
	RightHandMesh->SetCastShadow(false);
	RightHandMesh->SetRelativeLocation(FVector(-2.981260f, 3.5f, 4.561753f));
	RightHandMesh->SetRelativeRotation(FRotator(25.0f, 0.0f, 90.0f));

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> LeftHandMeshAsset(
		TEXT("/Game/XRMannequins/Meshes/SKM_MannyXR_left.SKM_MannyXR_left"));
	if (LeftHandMeshAsset.Succeeded())
	{
		LeftHandMesh->SetSkeletalMesh(LeftHandMeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> RightHandMeshAsset(
		TEXT("/Game/XRMannequins/Meshes/SKM_MannyXR_right.SKM_MannyXR_right"));
	if (RightHandMeshAsset.Succeeded())
	{
		RightHandMesh->SetSkeletalMesh(RightHandMeshAsset.Object);
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> HandAnimBlueprint(
		TEXT("/Game/XRMannequins/Meshes/ABP_MannequinsXR"));
	if (HandAnimBlueprint.Succeeded())
	{
		LeftHandMesh->SetAnimInstanceClass(HandAnimBlueprint.Class);
		RightHandMesh->SetAnimInstanceClass(HandAnimBlueprint.Class);
	}

	WidgetInteractionLeft = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteractionLeft"));
	WidgetInteractionLeft->SetupAttachment(MotionControllerLeftAim);
	WidgetInteractionLeft->InteractionSource = EWidgetInteractionSource::World;
	WidgetInteractionLeft->PointerIndex = 0;

	WidgetInteractionRight = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteractionRight"));
	WidgetInteractionRight->SetupAttachment(MotionControllerRightAim);
	WidgetInteractionRight->InteractionSource = EWidgetInteractionSource::World;
	WidgetInteractionRight->PointerIndex = 1;

	SubtitleHUD = CreateDefaultSubobject<UWidgetComponent>(TEXT("SubtitleHUD"));
	SubtitleHUD->SetupAttachment(VRCamera);
	SubtitleHUD->SetRelativeLocation(SubtitleHUDOffset);
	SubtitleHUD->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	SubtitleHUD->SetWidgetSpace(EWidgetSpace::World);
	SubtitleHUD->SetDrawSize(FVector2D(900.0f, 180.0f));
	SubtitleHUD->SetRelativeScale3D(FVector(0.07f));
	SubtitleHUD->SetPivot(FVector2D(0.5f, 0.5f));
	SubtitleHUD->SetBlendMode(EWidgetBlendMode::Transparent);
	SubtitleHUD->SetTwoSided(true);
	SubtitleHUD->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SubtitleHUD->SetWidgetClass(USubtitleWidget::StaticClass());
	SubtitleHUD->SetVisibility(false);

	NarrationEventHUD = CreateDefaultSubobject<UWidgetComponent>(TEXT("NarrationEventHUD"));
	NarrationEventHUD->SetupAttachment(VRCamera);
	NarrationEventHUD->SetRelativeLocation(EventHUDOffset);
	NarrationEventHUD->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	NarrationEventHUD->SetWidgetSpace(EWidgetSpace::World);
	NarrationEventHUD->SetDrawSize(FVector2D(900.0f, 600.0f));
	NarrationEventHUD->SetPivot(FVector2D(0.5f, 0.5f));
	NarrationEventHUD->SetTwoSided(true);
	NarrationEventHUD->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NarrationEventHUD->SetVisibility(false);

	NarrationAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("NarrationAudio"));
	NarrationAudio->SetupAttachment(VRCamera);
	NarrationAudio->bAutoActivate = false;
	NarrationAudio->bIsUISound = true;
	NarrationAudio->bAllowSpatialization = false;

	NarrationSequence = CreateDefaultSubobject<UNarrationSequenceComponent>(TEXT("NarrationSequence"));

	static ConstructorHelpers::FObjectFinder<UInputAction> TeleportActionFinder(TEXT("/Game/XRFramework/Input/Actions/IA_Move.IA_Move"));
	static ConstructorHelpers::FObjectFinder<UInputAction> TurnActionFinder(TEXT("/Game/XRFramework/Input/Actions/IA_Turn.IA_Turn"));
	static ConstructorHelpers::FObjectFinder<UInputAction> SmoothMoveActionFinder(TEXT("/Game/XRFramework/Input/Actions/IA_Menu_Cursor_Right.IA_Menu_Cursor_Right"));
	static ConstructorHelpers::FObjectFinder<UInputAction> TriggerGrabLeftActionFinder(TEXT("/Game/XRFramework/Input/Actions/Hands/IA_Hand_IndexCurl_Left.IA_Hand_IndexCurl_Left"));
	static ConstructorHelpers::FObjectFinder<UInputAction> TriggerGrabRightActionFinder(TEXT("/Game/XRFramework/Input/Actions/Hands/IA_Hand_IndexCurl_Right.IA_Hand_IndexCurl_Right"));
	static ConstructorHelpers::FObjectFinder<UInputAction> GrabLeftFinder(TEXT("/Game/XRFramework/Input/Actions/IA_Grab_Left_Pressed.IA_Grab_Left_Pressed"));
	static ConstructorHelpers::FObjectFinder<UInputAction> GrabRightFinder(TEXT("/Game/XRFramework/Input/Actions/IA_Grab_Right_Pressed.IA_Grab_Right_Pressed"));
	static ConstructorHelpers::FObjectFinder<UInputAction> ReleaseLeftFinder(TEXT("/Game/XRFramework/Input/Actions/IA_Grab_Left_Released.IA_Grab_Left_Released"));
	static ConstructorHelpers::FObjectFinder<UInputAction> ReleaseRightFinder(TEXT("/Game/XRFramework/Input/Actions/IA_Grab_Right_Released.IA_Grab_Right_Released"));
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultMappingContextFinder(TEXT("/Game/XRFramework/Input/IMC_Default.IMC_Default"));
	static ConstructorHelpers::FClassFinder<USceneComponent> GrabComponentFinder(TEXT("/Game/XRFramework/Blueprints/BP_GrabComponent"));
	static ConstructorHelpers::FClassFinder<AActor> TeleportVisualizerFinder(TEXT("/Game/XRFramework/Blueprints/BP_TeleportVisualizer"));

	TeleportAction = TeleportActionFinder.Object;
	TurnAction = TurnActionFinder.Object;
	// OpenXR builds its controller action set from startup-registered assets. Reuse the
	// template actions already present in default mapping contexts instead of transient actions.
	SmoothMoveAction = SmoothMoveActionFinder.Object;
	ViewTurnAction = TurnActionFinder.Object;
	TriggerGrabLeftAction = TriggerGrabLeftActionFinder.Object;
	TriggerGrabRightAction = TriggerGrabRightActionFinder.Object;
	GrabLeftAction = GrabLeftFinder.Object;
	GrabRightAction = GrabRightFinder.Object;
	ReleaseLeftAction = ReleaseLeftFinder.Object;
	ReleaseRightAction = ReleaseRightFinder.Object;
	DefaultMappingContext = DefaultMappingContextFinder.Object;
	GrabComponentClass = GrabComponentFinder.Class;
	TeleportVisualizerClass = TeleportVisualizerFinder.Class;
	AutoPossessPlayer = EAutoReceiveInput::Disabled;
}

void AVRPlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!ensureMsgf(EnhancedInput, TEXT("VRPlayerPawn requires an EnhancedInputComponent.")))
	{
		return;
	}

	if (SmoothMoveAction)
	{
		EnhancedInput->BindAction(SmoothMoveAction, ETriggerEvent::Triggered, this, &AVRPlayerPawn::HandleSmoothMoveTriggered);
	}
	if (TeleportAction && TeleportAction != SmoothMoveAction)
	{
		// IMC_Default already registers IA_Move with OpenXR at startup. Keep it as a
		// locomotion fallback when IMC_Menu is filtered by the current input mode.
		EnhancedInput->BindAction(TeleportAction, ETriggerEvent::Triggered, this, &AVRPlayerPawn::HandleSmoothMoveTriggered);
	}

	if (ViewTurnAction)
	{
		EnhancedInput->BindAction(ViewTurnAction, ETriggerEvent::Triggered, this, &AVRPlayerPawn::HandleTurnTriggered);
		EnhancedInput->BindAction(ViewTurnAction, ETriggerEvent::Completed, this, &AVRPlayerPawn::HandleTurnCompleted);
		EnhancedInput->BindAction(ViewTurnAction, ETriggerEvent::Canceled, this, &AVRPlayerPawn::HandleTurnCompleted);
	}

	if (GrabLeftAction)
	{
		EnhancedInput->BindAction(GrabLeftAction, ETriggerEvent::Started, this, &AVRPlayerPawn::HandleGrabLeft);
		EnhancedInput->BindAction(GrabLeftAction, ETriggerEvent::Completed, this, &AVRPlayerPawn::HandleReleaseLeft);
		EnhancedInput->BindAction(GrabLeftAction, ETriggerEvent::Canceled, this, &AVRPlayerPawn::HandleReleaseLeft);
	}
	if (GrabRightAction)
	{
		EnhancedInput->BindAction(GrabRightAction, ETriggerEvent::Started, this, &AVRPlayerPawn::HandleGrabRight);
		EnhancedInput->BindAction(GrabRightAction, ETriggerEvent::Completed, this, &AVRPlayerPawn::HandleReleaseRight);
		EnhancedInput->BindAction(GrabRightAction, ETriggerEvent::Canceled, this, &AVRPlayerPawn::HandleReleaseRight);
	}
	if (TriggerGrabLeftAction)
	{
		EnhancedInput->BindAction(TriggerGrabLeftAction, ETriggerEvent::Started, this, &AVRPlayerPawn::HandleTriggerLeftPressed);
		EnhancedInput->BindAction(TriggerGrabLeftAction, ETriggerEvent::Completed, this, &AVRPlayerPawn::HandleTriggerLeftReleased);
		EnhancedInput->BindAction(TriggerGrabLeftAction, ETriggerEvent::Canceled, this, &AVRPlayerPawn::HandleTriggerLeftReleased);
	}
	if (TriggerGrabRightAction)
	{
		EnhancedInput->BindAction(TriggerGrabRightAction, ETriggerEvent::Started, this, &AVRPlayerPawn::HandleTriggerRightPressed);
		EnhancedInput->BindAction(TriggerGrabRightAction, ETriggerEvent::Completed, this, &AVRPlayerPawn::HandleTriggerRightReleased);
		EnhancedInput->BindAction(TriggerGrabRightAction, ETriggerEvent::Canceled, this, &AVRPlayerPawn::HandleTriggerRightReleased);
	}

	ConfigureLocomotionInput();
}

void AVRPlayerPawn::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!MountedCameraAnchor.IsValid() || !VRCamera)
	{
		return;
	}

	const FTransform AnchorTransform = MountedCameraAnchor->GetComponentTransform();
	const FVector CameraOffset = VRCamera->GetComponentLocation() - GetActorLocation();
	SetActorLocationAndRotation(
		AnchorTransform.GetLocation() - FVector(CameraOffset.X, CameraOffset.Y, CameraOffset.Z),
		FRotator(0.0f, AnchorTransform.Rotator().Yaw, 0.0f), false, nullptr, ETeleportType::TeleportPhysics);
}

void AVRPlayerPawn::EnterMountedInteraction(USceneComponent* CameraAnchor)
{
	if (!IsValid(CameraAnchor)) return;
	MountedCameraAnchor = CameraAnchor;
	bEnableMove = false;
	Tick(0.0f);
}

void AVRPlayerPawn::ExitMountedInteraction(USceneComponent* CameraAnchor)
{
	if (CameraAnchor && MountedCameraAnchor.Get() != CameraAnchor) return;
	MountedCameraAnchor.Reset();
	bEnableMove = true;
}

void AVRPlayerPawn::BeginPlay()
{
	Super::BeginPlay();

	if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (DefaultMappingContext && !InputSubsystem->HasMappingContext(DefaultMappingContext))
				{
					InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
				}
			}
		}
	}

	SubtitleHUD->SetRelativeLocation(SubtitleHUDOffset);
	NarrationEventHUD->SetRelativeLocation(EventHUDOffset);
	SubtitleHUD->InitWidget();
	NarrationSequence->SetAudioComponent(NarrationAudio);
	NarrationSequence->OnSubtitleChanged.AddUniqueDynamic(this, &AVRPlayerPawn::HandleSubtitleChanged);
	NarrationSequence->OnNarrationStarted.AddUniqueDynamic(this, &AVRPlayerPawn::HandleNarrationStarted);
	NarrationSequence->OnWidgetRequested.AddUniqueDynamic(this, &AVRPlayerPawn::HandleNarrationWidgetRequested);
}

void AVRPlayerPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveLocomotionInput();
	TryRelease(MotionControllerLeftGrip, HeldComponentLeft);
	TryRelease(MotionControllerRightGrip, HeldComponentRight);

	if (TeleportVisualizer)
	{
		TeleportVisualizer->Destroy();
		TeleportVisualizer = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AVRPlayerPawn::HandleTeleportStarted(const FInputActionValue& Value)
{
	StartTeleportTrace();
	HandleTeleportTriggered(Value);
}

void AVRPlayerPawn::HandleTeleportTriggered(const FInputActionValue& Value)
{
	if (!bTeleportTraceActive)
	{
		StartTeleportTrace();
	}

	FVector2D InputAxis = FVector2D::ZeroVector;
	if (Value.GetValueType() == EInputActionValueType::Axis2D)
	{
		InputAxis = Value.Get<FVector2D>();
	}
	else
	{
		InputAxis.Y = GetAxisX(Value);
	}
	UpdateTeleportTrace(InputAxis);
}

void AVRPlayerPawn::HandleTeleportCompleted(const FInputActionValue& Value)
{
	EndTeleportTrace(true);
}

void AVRPlayerPawn::HandleTeleportCanceled(const FInputActionValue& Value)
{
	EndTeleportTrace(false);
}

void AVRPlayerPawn::HandleTurnTriggered(const FInputActionValue& Value)
{
	const float AxisValue = GetAxisX(Value);
	if (FMath::Abs(AxisValue) < TurnDeadZone)
	{
		bTurnLatched = false;
		return;
	}

	if (!bTurnLatched)
	{
		SnapTurn(AxisValue);
		bTurnLatched = true;
	}
}

void AVRPlayerPawn::HandleTurnCompleted(const FInputActionValue& Value)
{
	bTurnLatched = false;
}

void AVRPlayerPawn::HandleSmoothMoveTriggered(const FInputActionValue& Value)
{
	if (!bEnableMove || Value.GetValueType() != EInputActionValueType::Axis2D)
	{
		return;
	}

	ApplySmoothMove(Value.Get<FVector2D>());
}

void AVRPlayerPawn::HandleGrabLeft(const FInputActionValue& Value)
{
	SetHandGraspAlpha(LeftHandMesh, 1.0f);
	TryGrab(MotionControllerLeftGrip, HeldComponentLeft);
}

void AVRPlayerPawn::HandleGrabRight(const FInputActionValue& Value)
{
	SetHandGraspAlpha(RightHandMesh, 1.0f);
	TryGrab(MotionControllerRightGrip, HeldComponentRight);
}

void AVRPlayerPawn::HandleReleaseLeft(const FInputActionValue& Value)
{
	SetHandGraspAlpha(LeftHandMesh, 0.0f);
	TryRelease(MotionControllerLeftGrip, HeldComponentLeft);
}

void AVRPlayerPawn::HandleReleaseRight(const FInputActionValue& Value)
{
	SetHandGraspAlpha(RightHandMesh, 0.0f);
	TryRelease(MotionControllerRightGrip, HeldComponentRight);
}

void AVRPlayerPawn::HandleTriggerLeftPressed(const FInputActionValue& Value)
{
	NotifyHeldTrigger(HeldComponentLeft, TEXT("TriggerPressed"), MotionControllerLeftGrip);
}

void AVRPlayerPawn::HandleTriggerRightPressed(const FInputActionValue& Value)
{
	NotifyHeldTrigger(HeldComponentRight, TEXT("TriggerPressed"), MotionControllerRightGrip);
}

void AVRPlayerPawn::HandleTriggerLeftReleased(const FInputActionValue& Value)
{
	NotifyHeldTrigger(HeldComponentLeft, TEXT("TriggerReleased"), MotionControllerLeftGrip);
}

void AVRPlayerPawn::HandleTriggerRightReleased(const FInputActionValue& Value)
{
	NotifyHeldTrigger(HeldComponentRight, TEXT("TriggerReleased"), MotionControllerRightGrip);
}

void AVRPlayerPawn::ConfigureLocomotionInput()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr;
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer
		? LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()
		: nullptr;
	if (!InputSubsystem)
	{
		return;
	}

	if (DefaultMappingContext && !InputSubsystem->HasMappingContext(DefaultMappingContext))
	{
		InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	if (!RuntimeLocomotionMappingContext)
	{
		RuntimeLocomotionMappingContext = NewObject<UInputMappingContext>(this, TEXT("RuntimeLocomotionMappingContext"));
		// OpenXR controller mappings live in the startup-loaded IMC_Default/IMC_Hands/IMC_Menu
		// assets. This runtime context is intentionally limited to keyboard PIE fallbacks.
		RuntimeLocomotionMappingContext->MapKey(SmoothMoveAction, EKeys::D);
		FEnhancedActionKeyMapping& MoveLeftMapping = RuntimeLocomotionMappingContext->MapKey(SmoothMoveAction, EKeys::A);
		MoveLeftMapping.Modifiers.Add(NewObject<UInputModifierNegate>(RuntimeLocomotionMappingContext));

		FEnhancedActionKeyMapping& MoveForwardMapping = RuntimeLocomotionMappingContext->MapKey(SmoothMoveAction, EKeys::W);
		UInputModifierSwizzleAxis* KeyboardForwardSwizzle = NewObject<UInputModifierSwizzleAxis>(RuntimeLocomotionMappingContext);
		KeyboardForwardSwizzle->Order = EInputAxisSwizzle::YXZ;
		MoveForwardMapping.Modifiers.Add(KeyboardForwardSwizzle);

		FEnhancedActionKeyMapping& MoveBackwardMapping = RuntimeLocomotionMappingContext->MapKey(SmoothMoveAction, EKeys::S);
		UInputModifierSwizzleAxis* KeyboardBackwardSwizzle = NewObject<UInputModifierSwizzleAxis>(RuntimeLocomotionMappingContext);
		KeyboardBackwardSwizzle->Order = EInputAxisSwizzle::YXZ;
		MoveBackwardMapping.Modifiers.Add(KeyboardBackwardSwizzle);
		MoveBackwardMapping.Modifiers.Add(NewObject<UInputModifierNegate>(RuntimeLocomotionMappingContext));

		RuntimeLocomotionMappingContext->MapKey(ViewTurnAction, EKeys::E);
		FEnhancedActionKeyMapping& TurnLeftMapping = RuntimeLocomotionMappingContext->MapKey(ViewTurnAction, EKeys::Q);
		TurnLeftMapping.Modifiers.Add(NewObject<UInputModifierNegate>(RuntimeLocomotionMappingContext));

		RuntimeLocomotionMappingContext->MapKey(GrabLeftAction, EKeys::F);
		RuntimeLocomotionMappingContext->MapKey(GrabRightAction, EKeys::G);
		RuntimeLocomotionMappingContext->MapKey(TriggerGrabLeftAction, EKeys::T);
		RuntimeLocomotionMappingContext->MapKey(TriggerGrabRightAction, EKeys::Y);
	}

	if (!InputSubsystem->HasMappingContext(RuntimeLocomotionMappingContext))
	{
		InputSubsystem->AddMappingContext(RuntimeLocomotionMappingContext, 100);
	}
}

void AVRPlayerPawn::RemoveLocomotionInput()
{
	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr;
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer
		? LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()
		: nullptr;
	if (InputSubsystem && RuntimeLocomotionMappingContext)
	{
		InputSubsystem->RemoveMappingContext(RuntimeLocomotionMappingContext);
	}
}

void AVRPlayerPawn::ApplySmoothMove(const FVector2D& InputAxis)
{
	if (!GetWorld())
	{
		return;
	}

	FVector2D ClampedAxis = InputAxis.GetClampedToMaxSize(1.0f);
	if (ClampedAxis.SizeSquared() < FMath::Square(SmoothMoveDeadZone))
	{
		return;
	}

	const FRotator CameraYaw(0.0f, VRCamera->GetComponentRotation().Yaw, 0.0f);
	const FVector MoveDirection =
		(CameraYaw.Vector() * ClampedAxis.Y) +
		(FRotationMatrix(CameraYaw).GetUnitAxis(EAxis::Y) * ClampedAxis.X);
	AddActorWorldOffset(MoveDirection * SmoothMoveSpeed * GetWorld()->GetDeltaSeconds(), true);
}

void AVRPlayerPawn::SetHandGraspAlpha(USkeletalMeshComponent* HandMesh, const float Alpha) const
{
	UAnimInstance* AnimInstance = HandMesh ? HandMesh->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		return;
	}

	if (FNumericProperty* GraspProperty = FindFProperty<FNumericProperty>(AnimInstance->GetClass(), TEXT("PoseAlphaGrasp"));
		GraspProperty && GraspProperty->IsFloatingPoint())
	{
		void* PropertyAddress = GraspProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		GraspProperty->SetFloatingPointPropertyValue(PropertyAddress, FMath::Clamp(static_cast<double>(Alpha), 0.0, 1.0));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s is missing the PoseAlphaGrasp animation property."), *GetNameSafe(AnimInstance));
	}
}

void AVRPlayerPawn::StartTeleportTrace()
{
	bTeleportTraceActive = true;
	bValidTeleportLocation = false;

	if (!TeleportVisualizer && TeleportVisualizerClass && GetWorld())
	{
		TeleportVisualizer = GetWorld()->SpawnActor<AActor>(TeleportVisualizerClass, GetActorTransform());
	}
	if (TeleportVisualizer)
	{
		TeleportVisualizer->SetActorHiddenInGame(true);
	}
}

void AVRPlayerPawn::UpdateTeleportTrace(const FVector2D& InputAxis)
{
	if (!bTeleportTraceActive || InputAxis.IsNearlyZero())
	{
		bValidTeleportLocation = false;
		if (TeleportVisualizer)
		{
			TeleportVisualizer->SetActorHiddenInGame(true);
		}
		return;
	}

	const FRotator CameraYaw(0.0f, VRCamera->GetComponentRotation().Yaw, 0.0f);
	const FVector HorizontalDirection =
		(CameraYaw.Vector() * InputAxis.Y) +
		(FRotationMatrix(CameraYaw).GetUnitAxis(EAxis::Y) * InputAxis.X);
	const FVector LaunchVelocity = HorizontalDirection.GetSafeNormal() * TeleportHorizontalSpeed + FVector::UpVector * TeleportVerticalSpeed;

	FPredictProjectilePathParams PathParams;
	PathParams.StartLocation = MotionControllerRightAim->GetComponentLocation();
	PathParams.LaunchVelocity = LaunchVelocity;
	PathParams.bTraceWithCollision = true;
	PathParams.ProjectileRadius = 3.0f;
	PathParams.MaxSimTime = TeleportMaxSimulationTime;
	PathParams.SimFrequency = 20.0f;
	PathParams.TraceChannel = ECC_Visibility;
	PathParams.ActorsToIgnore.Add(this);

	FPredictProjectilePathResult PathResult;
	bValidTeleportLocation = false;
	if (UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult))
	{
		if (UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetCurrent(GetWorld()))
		{
			FNavLocation ProjectedLocation;
			if (NavigationSystem->ProjectPointToNavigation(PathResult.HitResult.Location, ProjectedLocation, TeleportNavigationExtent))
			{
				ProjectedTeleportLocation = ProjectedLocation.Location;
				bValidTeleportLocation = true;
			}
		}
	}

	if (TeleportVisualizer)
	{
		TeleportVisualizer->SetActorHiddenInGame(!bValidTeleportLocation);
		if (bValidTeleportLocation)
		{
			TeleportVisualizer->SetActorLocation(ProjectedTeleportLocation);
		}
	}
}

void AVRPlayerPawn::EndTeleportTrace(const bool bCommitTeleport)
{
	if (bTeleportTraceActive && bCommitTeleport && bValidTeleportLocation)
	{
		const FVector CameraOffset = VRCamera->GetComponentLocation() - GetActorLocation();
		const FVector PawnDestination = ProjectedTeleportLocation - FVector(CameraOffset.X, CameraOffset.Y, 0.0f);
		SetActorLocation(PawnDestination, false, nullptr, ETeleportType::TeleportPhysics);
	}

	bTeleportTraceActive = false;
	bValidTeleportLocation = false;
	if (TeleportVisualizer)
	{
		TeleportVisualizer->SetActorHiddenInGame(true);
	}
}

void AVRPlayerPawn::SnapTurn(const float AxisValue)
{
	const FVector CameraLocationBeforeTurn = VRCamera->GetComponentLocation();
	AddActorWorldRotation(FRotator(0.0f, FMath::Sign(AxisValue) * SnapTurnDegrees, 0.0f));
	const FVector CameraLocationAfterTurn = VRCamera->GetComponentLocation();
	const FVector Correction = CameraLocationBeforeTurn - CameraLocationAfterTurn;
	AddActorWorldOffset(FVector(Correction.X, Correction.Y, 0.0f));
}

void AVRPlayerPawn::TryGrab(UMotionControllerComponent* MotionController, TObjectPtr<USceneComponent>& HeldComponent)
{
	if (!MotionController || HeldComponent)
	{
		return;
	}

	if (USceneComponent* Candidate = FindNearestGrabComponent(MotionController))
	{
		if (InvokeGrabFunction(Candidate, TEXT("TryGrab"), MotionController))
		{
			HeldComponent = Candidate;
			if (AActor* GrabbedActor = Candidate->GetOwner())
			{
				TInlineComponentArray<UScenarioInteractableComponent*> ScenarioInteractors(GrabbedActor);
				for (UScenarioInteractableComponent* Interactor : ScenarioInteractors)
				{
					if (IsValid(Interactor) && Interactor->SupportsInteractionType(EScenarioInteractionType::Grab))
					{
						Interactor->ReportInteractionCompleted(EScenarioInteractionType::Grab);
					}
				}
			}
		}
	}
}

void AVRPlayerPawn::TryRelease(UMotionControllerComponent* MotionController, TObjectPtr<USceneComponent>& HeldComponent)
{
	if (!HeldComponent)
	{
		return;
	}

	InvokeGrabFunction(HeldComponent, TEXT("TryRelease"), MotionController);
	HeldComponent = nullptr;
}

USceneComponent* AVRPlayerPawn::FindNearestGrabComponent(const UMotionControllerComponent* MotionController) const
{
	if (!MotionController || !GetWorld())
	{
		return nullptr;
	}

	USceneComponent* NearestComponent = nullptr;
	float NearestDistanceSquared = FMath::Square(GrabRadiusFromGripPosition);
	const FVector GripLocation = MotionController->GetComponentLocation();

	for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
	{
		TInlineComponentArray<USceneComponent*> SceneComponents(*ActorIterator);
		for (USceneComponent* SceneComponent : SceneComponents)
		{
			if (!SceneComponent || !SceneComponent->FindFunction(TEXT("TryGrab")))
			{
				continue;
			}
			if (SceneComponent == HeldComponentLeft || SceneComponent == HeldComponentRight)
			{
				const FBoolProperty* TwoHandProperty = FindFProperty<FBoolProperty>(SceneComponent->GetClass(), TEXT("bAllowTwoHandedGrab"));
				if (!TwoHandProperty || !TwoHandProperty->GetPropertyValue_InContainer(SceneComponent)) continue;
			}

			const float DistanceSquared = FVector::DistSquared(GripLocation, SceneComponent->GetComponentLocation());
			if (DistanceSquared <= NearestDistanceSquared)
			{
				NearestDistanceSquared = DistanceSquared;
				NearestComponent = SceneComponent;
			}
		}
	}

	return NearestComponent;
}

void AVRPlayerPawn::NotifyHeldTrigger(USceneComponent* HeldComponent, const FName FunctionName, UMotionControllerComponent* MotionController) const
{
	if (HeldComponent && HeldComponent->FindFunction(FunctionName))
	{
		InvokeGrabFunction(HeldComponent, FunctionName, MotionController);
	}
}

bool AVRPlayerPawn::InvokeGrabFunction(
	USceneComponent* GrabComponent,
	const FName FunctionName,
	UMotionControllerComponent* MotionController) const
{
	if (!GrabComponent)
	{
		return false;
	}

	UFunction* Function = GrabComponent->FindFunction(FunctionName);
	if (!Function)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s does not implement %s."), *GetNameSafe(GrabComponent), *FunctionName.ToString());
		return false;
	}

	FStructOnScope ParameterScope(Function);
	uint8* Parameters = ParameterScope.GetStructMemory();
	for (TFieldIterator<FProperty> PropertyIterator(Function); PropertyIterator; ++PropertyIterator)
	{
		FProperty* Property = *PropertyIterator;
		if (!Property->HasAnyPropertyFlags(CPF_Parm) || Property->HasAnyPropertyFlags(CPF_OutParm | CPF_ReturnParm))
		{
			continue;
		}

		if (FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(Property))
		{
			if (MotionController && MotionController->IsA(ObjectProperty->PropertyClass))
			{
				ObjectProperty->SetObjectPropertyValue_InContainer(Parameters, MotionController);
			}
		}
	}

	GrabComponent->ProcessEvent(Function, Parameters);

	for (TFieldIterator<FProperty> PropertyIterator(Function); PropertyIterator; ++PropertyIterator)
	{
		FProperty* Property = *PropertyIterator;
		if (Property->HasAnyPropertyFlags(CPF_ReturnParm))
		{
			if (const FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property))
			{
				return BoolProperty->GetPropertyValue_InContainer(Parameters);
			}
		}
	}

	if (const FBoolProperty* HeldProperty = FindFProperty<FBoolProperty>(GrabComponent->GetClass(), TEXT("bIsHeld")))
	{
		return HeldProperty->GetPropertyValue_InContainer(GrabComponent) || FunctionName == TEXT("TryRelease");
	}

	return true;
}

float AVRPlayerPawn::GetAxisX(const FInputActionValue& Value) const
{
	switch (Value.GetValueType())
	{
	case EInputActionValueType::Axis1D:
		return Value.Get<float>();
	case EInputActionValueType::Axis2D:
		return Value.Get<FVector2D>().X;
	case EInputActionValueType::Axis3D:
		return Value.Get<FVector>().X;
	default:
		return 0.0f;
	}
}

void AVRPlayerPawn::HandleSubtitleChanged(const FText SpeakerName, const FText Subtitle, const bool bVisible)
{
	SubtitleHUD->SetVisibility(bVisible);
	if (!bVisible)
	{
		return;
	}

	if (USubtitleWidget* Widget = Cast<USubtitleWidget>(SubtitleHUD->GetUserWidgetObject()))
	{
		Widget->SetSubtitle(SpeakerName, Subtitle);
	}
}

void AVRPlayerPawn::HandleNarrationWidgetRequested(const TSubclassOf<UUserWidget> WidgetClass, const FName SourceRow)
{
	if (!WidgetClass)
	{
		return;
	}

	NarrationEventHUD->SetWidgetClass(WidgetClass);
	NarrationEventHUD->SetVisibility(true);
}

void AVRPlayerPawn::HandleNarrationStarted(const FName RowName)
{
	NarrationEventHUD->SetVisibility(false);
	NarrationEventHUD->SetWidgetClass(nullptr);
}

void AVRPlayerPawn::DismissNarrationWidget(const bool bContinueSequence)
{
	NarrationEventHUD->SetVisibility(false);
	NarrationEventHUD->SetWidgetClass(nullptr);
	if (bContinueSequence && NarrationSequence->IsWaitingForContinue())
	{
		NarrationSequence->ContinueSequence();
	}
}
