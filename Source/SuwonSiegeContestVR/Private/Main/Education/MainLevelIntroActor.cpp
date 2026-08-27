#include "Main/Education/MainLevelIntroActor.h"

#include "Components/ArrowComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/WidgetComponent.h"
#include "Core/Experience/ExperienceSubsystem.h"
#include "Core/Scenario/ScenarioDefinition.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Main/Education/MainEducationScenarioManagerActor.h"
#include "Main/Education/MainLevelTitleWidget.h"
#if WITH_EDITOR
#include "UObject/UnrealType.h"
#endif

AMainLevelIntroActor::AMainLevelIntroActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	OverviewAnchor = CreateDefaultSubobject<UArrowComponent>(TEXT("OverviewAnchor"));
	OverviewAnchor->SetupAttachment(SceneRoot);
	OverviewAnchor->ArrowColor = FColor(70, 160, 255);
	OverviewAnchor->SetRelativeLocation(FVector(-5000.0f, -8000.0f, 2500.0f));

	TitleAnchor = CreateDefaultSubobject<UArrowComponent>(TEXT("TitleAnchor"));
	TitleAnchor->SetupAttachment(SceneRoot);
	TitleAnchor->ArrowColor = FColor(255, 215, 70);

	PlayerAnchor = CreateDefaultSubobject<UArrowComponent>(TEXT("PlayerAnchor"));
	PlayerAnchor->SetupAttachment(SceneRoot);
	PlayerAnchor->ArrowColor = FColor(80, 255, 120);

	TitleTextRender = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TitleTextRender"));
	TitleTextRender->SetupAttachment(SceneRoot);
	TitleTextRender->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	TitleTextRender->SetTextRenderColor(FColor::White);
	TitleTextRender->SetHiddenInGame(true);
	TitleTextRender->SetVisibility(false);

	SubtitleTextRender = CreateDefaultSubobject<UTextRenderComponent>(TEXT("SubtitleTextRender"));
	SubtitleTextRender->SetupAttachment(SceneRoot);
	SubtitleTextRender->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	SubtitleTextRender->SetTextRenderColor(FColor::White);
	SubtitleTextRender->SetHiddenInGame(true);
	SubtitleTextRender->SetVisibility(false);

	TitleWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("TitleWidgetComponent"));
	TitleWidgetComponent->SetupAttachment(SceneRoot);
	TitleWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	TitleWidgetComponent->SetDrawSize(FVector2D(1600.0f, 180.0f));
	TitleWidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));
	TitleWidgetComponent->SetBlendMode(EWidgetBlendMode::Transparent);
	TitleWidgetComponent->SetTwoSided(true);
	TitleWidgetComponent->SetTranslucentSortPriority(10000);
	TitleWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TitleWidgetComponent->SetWidgetClass(UMainLevelTitleWidget::StaticClass());
	TitleWidgetComponent->SetHiddenInGame(true);

	SubtitleWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("SubtitleWidgetComponent"));
	SubtitleWidgetComponent->SetupAttachment(SceneRoot);
	SubtitleWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	SubtitleWidgetComponent->SetDrawSize(FVector2D(1600.0f, 320.0f));
	SubtitleWidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));
	SubtitleWidgetComponent->SetBlendMode(EWidgetBlendMode::Transparent);
	SubtitleWidgetComponent->SetTwoSided(true);
	SubtitleWidgetComponent->SetTranslucentSortPriority(10001);
	SubtitleWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SubtitleWidgetComponent->SetWidgetClass(UMainLevelTitleWidget::StaticClass());
	SubtitleWidgetComponent->SetHiddenInGame(true);
}

void AMainLevelIntroActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RefreshEditorTitlePreview();
}

void AMainLevelIntroActor::RefreshTitlePreviewInEditor()
{
	RefreshEditorTitlePreview();
}

#if WITH_EDITOR
void AMainLevelIntroActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	RefreshEditorTitlePreview();
}
#endif

void AMainLevelIntroActor::BeginPlay()
{
	Super::BeginPlay();
	HideTitle();
	if (bPlayOnBeginPlay)
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &ThisClass::BeginIntroWhenReady);
	}
}

void AMainLevelIntroActor::BeginIntroWhenReady()
{
	if (bSkipIntroOnExperienceReturn && HasPendingExperienceReturn())
	{
		// Returning from an experience: hand straight to the education flow so it resumes at its
		// checkpoint instead of flying the camera through the castle intro a second time.
		UE_LOG(LogTemp, Display, TEXT("MainLevelIntroActor %s skipped the intro for an experience return."),
			*GetName());
		FinishIntro();
		return;
	}
	PlayIntro();
}

bool AMainLevelIntroActor::HasPendingExperienceReturn() const
{
	const AMainEducationScenarioManagerActor* Manager = ResolveEducationManager();
	const UScenarioDefinition* Definition = Manager ? Manager->ScenarioDefinition : nullptr;
	if (!Definition || Definition->ScenarioID.IsNone())
	{
		return false;
	}

	const UGameInstance* GameInstance = GetGameInstance();
	const UExperienceSubsystem* ExperienceSubsystem = GameInstance
		? GameInstance->GetSubsystem<UExperienceSubsystem>() : nullptr;
	FScenarioResumeCheckpoint Checkpoint;
	return ExperienceSubsystem &&
		ExperienceSubsystem->GetScenarioResumeCheckpoint(Definition->ScenarioID, Checkpoint);
}

void AMainLevelIntroActor::PlayIntro()
{
	IntroPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	IntroPlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!IntroPawn.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("MainLevelIntroActor %s could not find player pawn; skipping intro."), *GetName());
		FinishIntro();
		return;
	}

	HideTitle();
	SetInputLocked(bLockPlayerInputDuringIntro);
	IntroPhase = EMainLevelIntroPhase::MoveToTitle;
	PhaseElapsed = 0.0f;
	MovementStartTransform = OverviewAnchor->GetComponentTransform();
	MovePawnToAnchor(OverviewAnchor, 1.0f);
	SetActorTickEnabled(true);
}

void AMainLevelIntroActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!IntroPawn.IsValid())
	{
		FinishIntro();
		return;
	}

	PhaseElapsed += DeltaSeconds;
	switch (IntroPhase)
	{
	case EMainLevelIntroPhase::MoveToTitle:
	{
		const float Alpha = OverviewToTitleDuration <= 0.0f ? 1.0f : FMath::Clamp(PhaseElapsed / OverviewToTitleDuration, 0.0f, 1.0f);
		MovePawnToAnchor(TitleAnchor, Alpha);
		if (Alpha >= 1.0f)
		{
			IntroPhase = EMainLevelIntroPhase::ShowTitle;
			PhaseElapsed = 0.0f;
			ShowTitle(1.0f);
		}
		break;
	}
	case EMainLevelIntroPhase::ShowTitle:
		if (PhaseElapsed >= TitleDisplayDuration)
		{
			IntroPhase = EMainLevelIntroPhase::FadeTitle;
			PhaseElapsed = 0.0f;
		}
		break;
	case EMainLevelIntroPhase::FadeTitle:
	{
		const float Opacity = TitleFadeDuration <= 0.0f ? 0.0f : 1.0f - FMath::Clamp(PhaseElapsed / TitleFadeDuration, 0.0f, 1.0f);
		ShowTitle(Opacity);
		if (Opacity <= 0.0f)
		{
			HideTitle();
			IntroPhase = EMainLevelIntroPhase::MoveToPlayer;
			PhaseElapsed = 0.0f;
			MovementStartTransform = IntroPawn->GetActorTransform();
		}
		break;
	}
	case EMainLevelIntroPhase::MoveToPlayer:
	{
		const float Alpha = TitleToPlayerDuration <= 0.0f ? 1.0f : FMath::Clamp(PhaseElapsed / TitleToPlayerDuration, 0.0f, 1.0f);
		MovePawnToAnchor(PlayerAnchor, Alpha);
		if (Alpha >= 1.0f)
		{
			FinishIntro();
		}
		break;
	}
	default:
		break;
	}
}

void AMainLevelIntroActor::SkipIntro()
{
	if (IntroPawn.IsValid())
	{
		MovementStartTransform = IntroPawn->GetActorTransform();
		MovePawnToAnchor(PlayerAnchor, 1.0f);
	}
	HideTitle();
	FinishIntro();
}

void AMainLevelIntroActor::MovePawnToAnchor(const UArrowComponent* TargetAnchor, float Alpha)
{
	if (!IntroPawn.IsValid() || !TargetAnchor)
	{
		return;
	}

	const FTransform TargetTransform = TargetAnchor->GetComponentTransform();
	const FVector Location = FMath::Lerp(MovementStartTransform.GetLocation(), TargetTransform.GetLocation(), Alpha);
	const FQuat Rotation = FQuat::Slerp(MovementStartTransform.GetRotation(), TargetTransform.GetRotation(), Alpha);
	IntroPawn->SetActorLocationAndRotation(Location, Rotation, false, nullptr, ETeleportType::TeleportPhysics);
}

void AMainLevelIntroActor::ShowTitle(float Opacity)
{
	if (!IntroPawn.IsValid())
	{
		return;
	}

	UCameraComponent* Camera = IntroPawn->FindComponentByClass<UCameraComponent>();
	if (!Camera)
	{
		return;
	}

	const FLinearColor Color(TitleColor.R, TitleColor.G, TitleColor.B, TitleColor.A);
	const FVector WidgetScale(TitleWidgetWorldScale);

	TitleWidgetComponent->AttachToComponent(Camera, FAttachmentTransformRules::KeepRelativeTransform);
	TitleWidgetComponent->SetRelativeLocation(TitleOffset);
	TitleWidgetComponent->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	TitleWidgetComponent->SetRelativeScale3D(WidgetScale);
	TitleWidgetComponent->InitWidget();
	if (UMainLevelTitleWidget* Widget = Cast<UMainLevelTitleWidget>(TitleWidgetComponent->GetUserWidgetObject()))
	{
		Widget->Configure(TitleText, TitleFont, TitleFontSize, Color);
		Widget->SetRenderOpacity(Opacity);
	}
	TitleWidgetComponent->SetVisibility(true);
	TitleWidgetComponent->SetHiddenInGame(false);

	SubtitleWidgetComponent->AttachToComponent(Camera, FAttachmentTransformRules::KeepRelativeTransform);
	SubtitleWidgetComponent->SetRelativeLocation(SubtitleOffset);
	SubtitleWidgetComponent->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	SubtitleWidgetComponent->SetRelativeScale3D(WidgetScale);
	SubtitleWidgetComponent->InitWidget();
	if (UMainLevelTitleWidget* Widget = Cast<UMainLevelTitleWidget>(SubtitleWidgetComponent->GetUserWidgetObject()))
	{
		Widget->Configure(SubtitleText, SubtitleFont ? SubtitleFont : TitleFont, SubtitleFontSize, Color);
		Widget->SetRenderOpacity(Opacity);
	}
	SubtitleWidgetComponent->SetVisibility(!SubtitleText.IsEmpty());
	SubtitleWidgetComponent->SetHiddenInGame(SubtitleText.IsEmpty());
}

void AMainLevelIntroActor::HideTitle()
{
	TitleTextRender->SetHiddenInGame(true);
	TitleTextRender->SetVisibility(false);
	SubtitleTextRender->SetHiddenInGame(true);
	SubtitleTextRender->SetVisibility(false);
	TitleWidgetComponent->SetHiddenInGame(true);
	TitleWidgetComponent->SetVisibility(false);
	SubtitleWidgetComponent->SetHiddenInGame(true);
	SubtitleWidgetComponent->SetVisibility(false);
}

void AMainLevelIntroActor::RefreshEditorTitlePreview()
{
#if WITH_EDITOR
	if (!GetWorld() || GetWorld()->IsGameWorld())
	{
		return;
	}

	const FTransform AnchorTransform = TitleAnchor->GetComponentTransform();
	const FQuat WidgetRotation = AnchorTransform.GetRotation() * FQuat(FRotator(0.0f, 180.0f, 0.0f));
	const FVector WidgetScale(TitleWidgetWorldScale);

	// Retain the old components for serialized level compatibility, but never render them.
	TitleTextRender->SetVisibility(false);
	TitleTextRender->SetHiddenInGame(true);
	SubtitleTextRender->SetVisibility(false);
	SubtitleTextRender->SetHiddenInGame(true);

	TitleWidgetComponent->SetWorldLocation(AnchorTransform.TransformPosition(TitleOffset));
	TitleWidgetComponent->SetWorldRotation(WidgetRotation);
	TitleWidgetComponent->SetWorldScale3D(WidgetScale);
	TitleWidgetComponent->InitWidget();
	if (UMainLevelTitleWidget* Widget = Cast<UMainLevelTitleWidget>(TitleWidgetComponent->GetUserWidgetObject()))
	{
		Widget->Configure(TitleText, TitleFont, TitleFontSize, TitleColor);
		Widget->SetRenderOpacity(1.0f);
	}
	TitleWidgetComponent->SetVisibility(bShowEditorTitlePreview);
	TitleWidgetComponent->SetHiddenInGame(true);

	SubtitleWidgetComponent->SetWorldLocation(AnchorTransform.TransformPosition(SubtitleOffset));
	SubtitleWidgetComponent->SetWorldRotation(WidgetRotation);
	SubtitleWidgetComponent->SetWorldScale3D(WidgetScale);
	SubtitleWidgetComponent->InitWidget();
	if (UMainLevelTitleWidget* Widget = Cast<UMainLevelTitleWidget>(SubtitleWidgetComponent->GetUserWidgetObject()))
	{
		Widget->Configure(SubtitleText, SubtitleFont ? SubtitleFont : TitleFont, SubtitleFontSize, TitleColor);
		Widget->SetRenderOpacity(1.0f);
	}
	SubtitleWidgetComponent->SetVisibility(bShowEditorTitlePreview && !SubtitleText.IsEmpty());
	SubtitleWidgetComponent->SetHiddenInGame(true);
#endif
}

void AMainLevelIntroActor::FinishIntro()
{
	HideTitle();
	SetInputLocked(false);
	SetActorTickEnabled(false);
	IntroPhase = EMainLevelIntroPhase::Complete;
	if (AMainEducationScenarioManagerActor* Manager = ResolveEducationManager())
	{
		Manager->StartEducationAfterIntro();
	}
}

void AMainLevelIntroActor::SetInputLocked(bool bLocked)
{
	if (!bLockPlayerInputDuringIntro || bInputLocked == bLocked || !IntroPlayerController.IsValid())
	{
		return;
	}

	IntroPlayerController->SetIgnoreMoveInput(bLocked);
	IntroPlayerController->SetIgnoreLookInput(bLocked);
	bInputLocked = bLocked;
}

AMainEducationScenarioManagerActor* AMainLevelIntroActor::ResolveEducationManager() const
{
	if (TargetEducationManager)
	{
		return TargetEducationManager;
	}

	for (TActorIterator<AMainEducationScenarioManagerActor> It(GetWorld()); It; ++It)
	{
		return *It;
	}
	return nullptr;
}
