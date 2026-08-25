#include "Main/Education/MainEducationScenarioManagerActor.h"

#include "Core/Experience/ExperienceDefinition.h"
#include "Core/Experience/ExperienceSubsystem.h"
#include "Core/Scenario/ScenarioExperienceBridgeComponent.h"
#include "Core/Scenario/ScenarioManagerComponent.h"
#include "GameFramework/Pawn.h"
#include "Gameplay/UI/VRHUDComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Main/Education/MainEducationScenarioDefinition.h"

AMainEducationScenarioManagerActor::AMainEducationScenarioManagerActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AMainEducationScenarioManagerActor::BeginPlay()
{
	// Bind before the base actor auto-starts the configured Scenario.
	if (UScenarioManagerComponent* Manager = GetScenarioManager())
	{
		Manager->OnInteractionRequested.AddDynamic(this, &ThisClass::HandleInteractionRequested);
	}
	Super::BeginPlay();

	if (UMainEducationScenarioDefinition* Definition = GetEducationDefinition())
	{
		FString Error;
		if (!Definition->ValidateEducationScenario(Error))
		{
			UE_LOG(LogTemp, Error, TEXT("Main education definition is invalid: %s"), *Error);
		}
	}
}

bool AMainEducationScenarioManagerActor::ContinuePresentation()
{
	UScenarioManagerComponent* Manager = GetScenarioManager();
	if (!Manager || CurrentContent.ContentID.IsNone())
	{
		return false;
	}

	const FScenarioInteraction Interaction = Manager->GetCurrentInteraction();
	if (Interaction.InteractionType == EScenarioInteractionType::Quiz ||
		Interaction.InteractionType == EScenarioInteractionType::Narration ||
		Interaction.TargetID != CurrentContent.ContentID)
	{
		return false;
	}

	CurrentContent = FMainEducationContent();
	if (UVRHUDComponent* HUD = ResolveVRHUD())
	{
		HUD->ClearPrompt();
	}
	return Manager->CompleteInteraction(Interaction.InteractionID);
}

bool AMainEducationScenarioManagerActor::SubmitQuizAnswer(const FString& Answer)
{
	UScenarioManagerComponent* Manager = GetScenarioManager();
	if (!Manager || CurrentContent.ContentType != EMainEducationContentType::Quiz)
	{
		return false;
	}

	const FScenarioInteraction Interaction = Manager->GetCurrentInteraction();
	if (Interaction.InteractionType != EScenarioInteractionType::Quiz ||
		Interaction.TargetID != CurrentContent.ContentID)
	{
		return false;
	}

	++QuizAttemptCount;
	UMainEducationScenarioDefinition* Definition = GetEducationDefinition();
	const bool bCorrect = Definition && Definition->IsAcceptedQuizAnswer(CurrentContent.ContentID, Answer);

	if (!bCorrect)
	{
		const FText Feedback = FText::FromString(TEXT("다시 생각해 보십시오."));
		OnQuizFeedback.Broadcast(false, Feedback);
		if (UVRHUDComponent* HUD = ResolveVRHUD())
		{
			HUD->ShowNotification(Feedback, EVRHUDNotificationType::Warning);
		}
		return false;
	}

	const FText CanonicalAnswer = CurrentContent.AcceptedAnswers.IsEmpty()
		? FText::GetEmpty() : CurrentContent.AcceptedAnswers[0];
	const FText Feedback = FText::Format(
		FText::FromString(TEXT("정답입니다: {0}")), CanonicalAnswer);
	OnQuizFeedback.Broadcast(true, Feedback);
	if (UVRHUDComponent* HUD = ResolveVRHUD())
	{
		HUD->ShowNotification(Feedback, EVRHUDNotificationType::Success);
		HUD->ClearPrompt();
	}
	CancelVoiceRecognition();
	CurrentContent = FMainEducationContent();
	return Manager->CompleteInteraction(Interaction.InteractionID);
}

bool AMainEducationScenarioManagerActor::SkipUnavailableExperience()
{
	UScenarioManagerComponent* Manager = GetScenarioManager();
	if (!Manager || UnavailableRouteID.IsNone())
	{
		return false;
	}

	const FScenarioInteraction Interaction = Manager->GetCurrentInteraction();
	if (Interaction.TargetID != UnavailableRouteID)
	{
		return false;
	}

	UnavailableRouteID = NAME_None;
	return Manager->CompleteInteraction(Interaction.InteractionID);
}

bool AMainEducationScenarioManagerActor::StartEducationAfterIntro()
{
	if (bEducationStartedByIntro)
	{
		return false;
	}

	const bool bStarted = StartConfiguredScenario();
	if (bStarted)
	{
		bEducationStartedByIntro = true;
		if (UScenarioExperienceBridgeComponent* Bridge = GetExperienceBridge())
		{
			Bridge->RestoreScenarioCheckpoint();
		}
	}
	return bStarted;
}

bool AMainEducationScenarioManagerActor::ShouldAutoStartScenario() const
{
	return !bWaitForIntroSequence;
}

void AMainEducationScenarioManagerActor::RequestVoiceRecognition_Implementation(FName QuizID)
{
	// Intentionally empty. The voice-recognition owner implements capture/STT and calls SubmitQuizAnswer.
}

void AMainEducationScenarioManagerActor::CancelVoiceRecognition_Implementation()
{
	// Intentionally empty. Kept as the matching teardown port for the future implementation.
}

void AMainEducationScenarioManagerActor::HandleInteractionRequested(FScenarioInteraction Interaction)
{
	UnavailableRouteID = NAME_None;
	QuizAttemptCount = 0;

	if (Interaction.TargetID.ToString().StartsWith(TEXT("Travel_")))
	{
		BeginExperienceTravel(Interaction);
		return;
	}

	UMainEducationScenarioDefinition* Definition = GetEducationDefinition();
	if (!Definition || !Definition->FindEducationContent(Interaction.TargetID, CurrentContent))
	{
		UE_LOG(LogTemp, Warning, TEXT("No Main education content for interaction %s (%s)."),
			*Interaction.InteractionID.ToString(), *Interaction.TargetID.ToString());
		return;
	}

	OnEducationContentRequested.Broadcast(CurrentContent);
	if (bMirrorTextToVRHUD)
	{
		if (UVRHUDComponent* HUD = ResolveVRHUD())
		{
			HUD->SetObjective(CurrentContent.Title, CurrentContent.Body);
			FText Prompt = CurrentContent.InteractionGuideText;
			if (Prompt.IsEmpty() && Interaction.InteractionType == EScenarioInteractionType::Narration)
			{
				Prompt = FText::FromString(TEXT("나레이션 재생 중입니다."));
			}
			else if (Prompt.IsEmpty() && CurrentContent.ContentType == EMainEducationContentType::Quiz)
			{
				Prompt = FText::Format(
					FText::FromString(TEXT("정답을 말하거나 입력하세요: {0}")),
					CurrentContent.InitialConsonants);
			}
			else if (Prompt.IsEmpty())
			{
				Prompt = FText::FromString(TEXT("계속하려면 확인을 누르세요."));
			}
			HUD->ShowPrompt(Prompt);
		}
	}

	if (Interaction.InteractionType == EScenarioInteractionType::Quiz)
	{
		RequestVoiceRecognition(CurrentContent.ContentID);
	}
}

bool AMainEducationScenarioManagerActor::BeginExperienceTravel(const FScenarioInteraction& Interaction)
{
	UMainEducationScenarioDefinition* Definition = GetEducationDefinition();
	FMainEducationExperienceRoute Route;
	UExperienceDefinition* Experience = nullptr;
	if (Definition && Definition->FindExperienceRoute(Interaction.TargetID, Route) && !Route.Experience.IsNull())
	{
		Experience = Route.Experience.LoadSynchronous();
	}

	if (!Experience)
	{
		UnavailableRouteID = Interaction.TargetID;
		const FText Reason = FText::FromString(TEXT("연결된 체험 Level/Experience가 아직 없습니다."));
		OnExperienceUnavailable.Broadcast(Interaction.TargetID, Reason);
		if (UVRHUDComponent* HUD = ResolveVRHUD())
		{
			HUD->SetObjective(FText::FromName(Interaction.TargetID), Reason);
			HUD->ShowPrompt(FText::FromString(TEXT("개발 중에는 Skip Unavailable Experience를 사용하세요.")));
		}
		return false;
	}

	UScenarioManagerComponent* Manager = GetScenarioManager();
	UExperienceSubsystem* ExperienceSubsystem = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UExperienceSubsystem>() : nullptr;
	const FScenarioDebugSnapshot Snapshot = Manager ? Manager->GetDebugSnapshot() : FScenarioDebugSnapshot();
	if (!Manager || !ExperienceSubsystem || Interaction.NextInteractionID.IsNone() ||
		!ExperienceSubsystem->SetScenarioResumeCheckpoint(
			Snapshot.ScenarioID, Snapshot.SceneID, Interaction.NextInteractionID))
	{
		UnavailableRouteID = Interaction.TargetID;
		const FText Reason = FText::FromString(TEXT("체험 복귀 체크포인트를 만들 수 없습니다."));
		OnExperienceUnavailable.Broadcast(Interaction.TargetID, Reason);
		return false;
	}

	if (!ExperienceSubsystem->StartExperience(Experience, false))
	{
		ExperienceSubsystem->ClearScenarioResumeCheckpoint(Snapshot.ScenarioID);
		UnavailableRouteID = Interaction.TargetID;
		const FText Reason = FText::FromString(TEXT("체험 이동 요청이 거부되었습니다."));
		OnExperienceUnavailable.Broadcast(Interaction.TargetID, Reason);
		return false;
	}
	return true;
}

UMainEducationScenarioDefinition* AMainEducationScenarioManagerActor::GetEducationDefinition() const
{
	return Cast<UMainEducationScenarioDefinition>(ScenarioDefinition);
}

UVRHUDComponent* AMainEducationScenarioManagerActor::ResolveVRHUD() const
{
	const APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	return Pawn ? Pawn->FindComponentByClass<UVRHUDComponent>() : nullptr;
}
