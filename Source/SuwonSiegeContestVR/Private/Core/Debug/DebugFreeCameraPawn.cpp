#include "Core/Debug/DebugFreeCameraPawn.h"

#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PlayerController.h"

ADebugFreeCameraPawn::ADebugFreeCameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	bAddDefaultMovementBindings = false;
	bCollideWhenPlacing = false;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AutoPossessPlayer = EAutoReceiveInput::Disabled;
	if (UPawnMovementComponent* Movement = GetMovementComponent())
	{
		if (UFloatingPawnMovement* FloatingMovement = Cast<UFloatingPawnMovement>(Movement))
		{
			FloatingMovement->MaxSpeed = BaseFlySpeed * FastSpeedMultiplier;
			FloatingMovement->Acceleration = 16000.0f;
			FloatingMovement->Deceleration = 16000.0f;
		}
	}
	if (UPrimitiveComponent* Collision = GetCollisionComponent())
	{
		Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ADebugFreeCameraPawn::BeginPlay()
{
	Super::BeginPlay();
	SetActorEnableCollision(false);
	CurrentFlySpeed = BaseFlySpeed;
}

void ADebugFreeCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Intentionally does not call Super: the base class binds legacy axis mappings that would
	// double-apply on top of the polled input below.
}

void ADebugFreeCameraPawn::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return;
	}

	TickLook(PlayerController, DeltaSeconds);
	TickMovement(PlayerController, DeltaSeconds);
}

void ADebugFreeCameraPawn::TickLook(APlayerController* PlayerController, const float DeltaSeconds)
{
	if (bRequireRightMouseButtonToLook && !PlayerController->IsInputKeyDown(EKeys::RightMouseButton))
	{
		return;
	}

	float MouseDeltaX = 0.0f;
	float MouseDeltaY = 0.0f;
	PlayerController->GetInputMouseDelta(MouseDeltaX, MouseDeltaY);
	if (FMath::IsNearlyZero(MouseDeltaX) && FMath::IsNearlyZero(MouseDeltaY))
	{
		return;
	}

	FRotator ControlRotation = PlayerController->GetControlRotation();
	ControlRotation.Yaw += MouseDeltaX * MouseSensitivity;
	ControlRotation.Pitch = FMath::ClampAngle(ControlRotation.Pitch + MouseDeltaY * MouseSensitivity, -MaxPitchDegrees, MaxPitchDegrees);
	ControlRotation.Roll = 0.0f;
	PlayerController->SetControlRotation(ControlRotation);
}

void ADebugFreeCameraPawn::TickMovement(APlayerController* PlayerController, const float DeltaSeconds)
{
	FVector MovementInput = FVector::ZeroVector;
	if (PlayerController->IsInputKeyDown(EKeys::W) || PlayerController->IsInputKeyDown(EKeys::Up)) MovementInput.X += 1.0f;
	if (PlayerController->IsInputKeyDown(EKeys::S) || PlayerController->IsInputKeyDown(EKeys::Down)) MovementInput.X -= 1.0f;
	if (PlayerController->IsInputKeyDown(EKeys::D) || PlayerController->IsInputKeyDown(EKeys::Right)) MovementInput.Y += 1.0f;
	if (PlayerController->IsInputKeyDown(EKeys::A) || PlayerController->IsInputKeyDown(EKeys::Left)) MovementInput.Y -= 1.0f;
	if (PlayerController->IsInputKeyDown(EKeys::E) || PlayerController->IsInputKeyDown(EKeys::SpaceBar)) MovementInput.Z += 1.0f;
	if (PlayerController->IsInputKeyDown(EKeys::Q)) MovementInput.Z -= 1.0f;

	CurrentFlySpeed = BaseFlySpeed;
	if (PlayerController->IsInputKeyDown(EKeys::LeftShift) || PlayerController->IsInputKeyDown(EKeys::RightShift))
	{
		CurrentFlySpeed *= FastSpeedMultiplier;
	}
	else if (PlayerController->IsInputKeyDown(EKeys::LeftControl) || PlayerController->IsInputKeyDown(EKeys::RightControl))
	{
		CurrentFlySpeed *= SlowSpeedMultiplier;
	}

	if (MovementInput.IsNearlyZero())
	{
		return;
	}

	const FRotator ControlRotation = PlayerController->GetControlRotation();
	const FVector WorldDirection =
		FRotationMatrix(ControlRotation).GetUnitAxis(EAxis::X) * MovementInput.X +
		FRotationMatrix(ControlRotation).GetUnitAxis(EAxis::Y) * MovementInput.Y +
		FVector::UpVector * MovementInput.Z;

	AddActorWorldOffset(WorldDirection.GetSafeNormal() * CurrentFlySpeed * DeltaSeconds, false);
}
