#include "Nokro/NokroCraneActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "Core/Scenario/ScenarioInteractableComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Materials/MaterialInterface.h"
#include "Nokro/NokroControlGripComponent.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	UStaticMeshComponent* AddCube(AActor* Owner, USceneComponent* Parent, const TCHAR* Name,
		const FVector& Location, const FVector& Scale, UStaticMesh* Mesh, UMaterialInterface* Material = nullptr)
	{
		UStaticMeshComponent* Component = Owner->CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Component->SetupAttachment(Parent);
		Component->SetRelativeLocation(Location);
		Component->SetRelativeScale3D(Scale);
		Component->SetStaticMesh(Mesh);
		Component->SetCollisionProfileName(TEXT("BlockAll"));
		if (Material) Component->SetMaterial(0, Material);
		return Component;
	}
}

ANokroCraneActor::ANokroCraneActor()
{
	PrimaryActorTick.bCanEverTick = true;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	BoomPivot = CreateDefaultSubobject<USceneComponent>(TEXT("BoomPivot"));
	BoomPivot->SetupAttachment(Root);
	HandlePivot = CreateDefaultSubobject<USceneComponent>(TEXT("HandlePivot"));
	HandlePivot->SetupAttachment(Root);
	HandlePivot->SetRelativeLocation(FVector(0.0f, -85.0f, 125.0f));

	ControlGrip = CreateDefaultSubobject<UNokroControlGripComponent>(TEXT("ControlGrip"));
	ControlGrip->SetupAttachment(HandlePivot);
	ControlGrip->SetRelativeLocation(FVector(55.0f, 0.0f, 0.0f));

	StoneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("StoneRoot"));
	StoneRoot->SetupAttachment(BoomPivot);
	CarriedStone = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CarriedStone"));
	CarriedStone->SetupAttachment(StoneRoot);
	CarriedStone->SetRelativeScale3D(FVector(1.0f, 0.5f, 0.5f));
	CarriedStone->SetCollisionProfileName(TEXT("BlockAll"));

	ScenarioInteraction = CreateDefaultSubobject<UScenarioInteractableComponent>(TEXT("ScenarioInteraction"));
	ScenarioInteraction->TargetID = TEXT("Nokro_Crane");
	ScenarioInteraction->SupportedInteractionTypes = {
		EScenarioInteractionType::Grab, EScenarioInteractionType::Move,
		EScenarioInteractionType::Trigger };
	ScenarioInteraction->bAutoReportToScenarioManager = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> WoodFinder(TEXT("/GF_Geojunggi/Materials/M_NokroWood.M_NokroWood"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> StoneFinder(TEXT("/GF_Geojunggi/Materials/M_NokroStone.M_NokroStone"));
	UStaticMesh* Cube = CubeFinder.Object;
	UMaterialInterface* Wood = WoodFinder.Object;

	AddCube(this, Root, TEXT("BaseBeamA"), FVector(0, 90, 20), FVector(2.2f, 0.18f, 0.18f), Cube, Wood);
	AddCube(this, Root, TEXT("BaseBeamB"), FVector(0, -90, 20), FVector(2.2f, 0.18f, 0.18f), Cube, Wood);
	AddCube(this, Root, TEXT("CrossBaseA"), FVector(75, 0, 20), FVector(0.18f, 2.0f, 0.18f), Cube, Wood);
	AddCube(this, Root, TEXT("CrossBaseB"), FVector(-75, 0, 20), FVector(0.18f, 2.0f, 0.18f), Cube, Wood);
	AddCube(this, Root, TEXT("MastLeft"), FVector(0, 70, 245), FVector(0.22f, 0.22f, 2.25f), Cube, Wood);
	AddCube(this, Root, TEXT("MastRight"), FVector(0, -70, 245), FVector(0.22f, 0.22f, 2.25f), Cube, Wood);
	AddCube(this, BoomPivot, TEXT("Boom"), FVector(BoomLength * 0.5f, 0, 470), FVector(BoomLength / 200.0f, 0.18f, 0.18f), Cube, Wood);
	AddCube(this, BoomPivot, TEXT("BoomCounterweight"), FVector(-85, 0, 445), FVector(0.8f, 0.5f, 0.5f), Cube, StoneFinder.Object);
	AddCube(this, HandlePivot, TEXT("HandleBar"), FVector::ZeroVector, FVector(0.65f, 0.08f, 0.08f), Cube, Wood);
	AddCube(this, HandlePivot, TEXT("HandleGripVisual"), FVector(55, 0, 0), FVector(0.12f, 0.12f, 0.35f), Cube, Wood);

	RopeVisual = AddCube(this, BoomPivot, TEXT("RopeVisual"), FVector(BoomLength, 0, 300), FVector(0.025f, 0.025f, 1.7f), Cube);
	RopeVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (Cube) CarriedStone->SetStaticMesh(Cube);
	if (StoneFinder.Succeeded()) CarriedStone->SetMaterial(0, StoneFinder.Object);

	CurrentHeight = InitialHeight;
}

void ANokroCraneActor::BeginPlay()
{
	Super::BeginPlay();
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		EnableInput(PC);
		if (InputComponent)
		{
			InputComponent->Priority = 20;
			const FKey Keys[] = {
				FKey(TEXT("OculusTouch_Right_Thumbstick_X")), FKey(TEXT("ValveIndex_Right_Thumbstick_X")),
				FKey(TEXT("MixedReality_Right_Thumbstick_X")), EKeys::Gamepad_RightX };
			for (const FKey& Key : Keys)
			{
				FInputAxisKeyBinding& Binding = InputComponent->BindAxisKey(Key);
				Binding.bConsumeInput = false;
			}
		}
	}
	UpdateStoneVisual();
}

void ANokroCraneActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr) DisableInput(PC);
	Super::EndPlay(EndPlayReason);
}

void ANokroCraneActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!ControlGrip || !ControlGrip->IsHeld()) return;
	const float Axis = ReadRotationJoystick();
	if (FMath::Abs(Axis) <= JoystickDeadZone) return;
	SetBoomYaw(GetCurrentBoomYaw() + Axis * RotationSpeedDegrees * DeltaSeconds);
	if (!bReportedDirectionAdjustment)
	{
		bReportedDirectionAdjustment = true;
		OnDirectionAdjusted.Broadcast();
		ScenarioInteraction->ReportInteractionCompleted(EScenarioInteractionType::Move);
	}
}

void ANokroCraneActor::ApplyHandleRotation(const float DeltaDegrees)
{
	const float PreviousHeight = CurrentHeight;
	CurrentHeight = FMath::Clamp(CurrentHeight + DeltaDegrees * HeightPerHandleDegree, MinimumHeight, MaximumHeight);
	UpdateStoneVisual();
	if (!FMath::IsNearlyEqual(PreviousHeight, CurrentHeight) && !bReportedHeightAdjustment)
	{
		bReportedHeightAdjustment = true;
		OnHeightAdjusted.Broadcast();
		ScenarioInteraction->ReportInteractionProgress(EScenarioInteractionType::Move,
			(CurrentHeight - MinimumHeight) / FMath::Max(1.0f, MaximumHeight - MinimumHeight));
	}
}

void ANokroCraneActor::SetBoomYaw(const float NewYaw)
{
	BoomPivot->SetRelativeRotation(FRotator(0.0f, FRotator::NormalizeAxis(NewYaw), 0.0f));
}

void ANokroCraneActor::RequestStonePlacement()
{
	if (!bPlacementEnabled || !CarriedStone->IsVisible()) return;
	ScenarioInteraction->ReportInteractionStarted(EScenarioInteractionType::Trigger);
	OnPlacementRequested.Broadcast(GetCarriedStoneTransform());
}

void ANokroCraneActor::ResetCarriedStone(const bool bShowStone)
{
	CurrentHeight = FMath::Clamp(InitialHeight, MinimumHeight, MaximumHeight);
	SetBoomYaw(0.0f);
	CarriedStone->SetVisibility(bShowStone);
	bReportedHeightAdjustment = false;
	bReportedDirectionAdjustment = false;
	UpdateStoneVisual();
}

void ANokroCraneActor::NotifyHandleGrabbed()
{
	OnHandleGrabbed.Broadcast();
}

void ANokroCraneActor::SetJoystickInputCapture(const bool bCapture)
{
	if (!InputComponent) return;
	for (FInputAxisKeyBinding& Binding : InputComponent->AxisKeyBindings) Binding.bConsumeInput = bCapture;
}

FTransform ANokroCraneActor::GetCarriedStoneTransform() const
{
	return CarriedStone->GetComponentTransform();
}

float ANokroCraneActor::GetCurrentBoomYaw() const
{
	return BoomPivot ? FRotator::NormalizeAxis(BoomPivot->GetRelativeRotation().Yaw) : 0.0f;
}

float ANokroCraneActor::ReadRotationJoystick() const
{
	const APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC) return 0.0f;
	const FKey Keys[] = {
		FKey(TEXT("OculusTouch_Right_Thumbstick_X")), FKey(TEXT("ValveIndex_Right_Thumbstick_X")),
		FKey(TEXT("MixedReality_Right_Thumbstick_X")), EKeys::Gamepad_RightX };
	float Result = 0.0f;
	for (const FKey& Key : Keys)
	{
		const float Value = PC->GetInputAnalogKeyState(Key);
		if (FMath::Abs(Value) > FMath::Abs(Result)) Result = Value;
	}
	return Result;
}

void ANokroCraneActor::UpdateStoneVisual()
{
	if (!StoneRoot || !RopeVisual) return;
	StoneRoot->SetRelativeLocation(FVector(BoomLength, 0.0f, CurrentHeight));
	const float RopeTop = 455.0f;
	const float RopeLength = FMath::Max(5.0f, RopeTop - CurrentHeight);
	RopeVisual->SetRelativeLocation(FVector(BoomLength, 0.0f, CurrentHeight + RopeLength * 0.5f));
	RopeVisual->SetRelativeScale3D(FVector(0.025f, 0.025f, RopeLength / 100.0f));
}
