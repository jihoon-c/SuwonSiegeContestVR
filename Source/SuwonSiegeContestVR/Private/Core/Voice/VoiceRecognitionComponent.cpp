#include "Core/Voice/VoiceRecognitionComponent.h"

#include "Core/Text/HangulTextLibrary.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UVoiceRecognitionComponent::UVoiceRecognitionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UVoiceRecognitionComponent::StartListening(const FVoiceRecognitionRequest& Request)
{
	// A second request replaces the first rather than stacking microphone sessions.
	if (State == EVoiceRecognitionState::Listening)
	{
		StopListening();
	}

	ActiveRequest = Request;

	if (!BeginBackendListening(ActiveRequest))
	{
		SetVoiceState(EVoiceRecognitionState::Unavailable);
		FVoiceRecognitionResult Result;
		Result.RequestID = ActiveRequest.RequestID;
		Result.Outcome = EVoiceRecognitionOutcome::Failed;
		OnRecognitionResult.Broadcast(Result);
		return false;
	}

	SetVoiceState(EVoiceRecognitionState::Listening);

	if (ActiveRequest.ListenDuration > 0.0f)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(ListenTimeoutHandle, this,
				&UVoiceRecognitionComponent::HandleListenTimeout, ActiveRequest.ListenDuration, false);
		}
	}

	return true;
}

void UVoiceRecognitionComponent::StopListening()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ListenTimeoutHandle);
	}

	if (State == EVoiceRecognitionState::Idle)
	{
		return;
	}

	EndBackendListening();
	ActiveRequest = FVoiceRecognitionRequest();
	SetVoiceState(EVoiceRecognitionState::Idle);
}

bool UVoiceRecognitionComponent::ReportRecognizedText(const FString& RecognizedText, const float Confidence)
{
	if (State != EVoiceRecognitionState::Listening && State != EVoiceRecognitionState::Processing)
	{
		return false;
	}

	FVoiceRecognitionResult Result;
	Result.RequestID = ActiveRequest.RequestID;
	Result.RecognizedText = RecognizedText;
	Result.Confidence = Confidence;

	FString MatchedKeyword;
	const bool bConfident = Confidence >= ActiveRequest.ConfidenceThreshold;
	const bool bMatched = bConfident && MatchKeyword(RecognizedText, ActiveRequest.Keywords, MatchedKeyword);

	Result.MatchedKeyword = bMatched ? MatchedKeyword : FString();
	Result.Outcome = bMatched ? EVoiceRecognitionOutcome::KeywordMatched : EVoiceRecognitionOutcome::NoMatch;

	FinishRequest(Result);
	return bMatched;
}

void UVoiceRecognitionComponent::ReportRecognitionFailed(const EVoiceRecognitionOutcome Outcome)
{
	if (State == EVoiceRecognitionState::Idle)
	{
		return;
	}

	FVoiceRecognitionResult Result;
	Result.RequestID = ActiveRequest.RequestID;
	Result.Outcome = Outcome;
	FinishRequest(Result);
}

void UVoiceRecognitionComponent::HandleListenTimeout()
{
	ReportRecognitionFailed(EVoiceRecognitionOutcome::TimedOut);
}

void UVoiceRecognitionComponent::FinishRequest(const FVoiceRecognitionResult& Result)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ListenTimeoutHandle);
	}

	EndBackendListening();
	ActiveRequest = FVoiceRecognitionRequest();
	SetVoiceState(EVoiceRecognitionState::Idle);

	// Broadcast last: a listener may start the next request from inside this callback.
	OnRecognitionResult.Broadcast(Result);
}

void UVoiceRecognitionComponent::SetVoiceState(const EVoiceRecognitionState NewState)
{
	if (State == NewState)
	{
		return;
	}

	const EVoiceRecognitionState OldState = State;
	State = NewState;
	OnVoiceStateChanged.Broadcast(OldState, NewState);
}

bool UVoiceRecognitionComponent::MatchKeyword(
	const FString& RecognizedText, const TArray<FString>& Keywords, FString& OutMatchedKeyword)
{
	OutMatchedKeyword.Reset();

	const FString Normalized = UHangulTextLibrary::NormalizeAnswer(RecognizedText);
	if (Normalized.IsEmpty())
	{
		return false;
	}

	for (const FString& Keyword : Keywords)
	{
		if (UHangulTextLibrary::NormalizeAnswer(Keyword) == Normalized)
		{
			OutMatchedKeyword = Keyword;
			return true;
		}
	}

	return false;
}

UVoiceRecognitionComponent* UVoiceRecognitionComponent::FindVoiceRecognition(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine
		? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull)
		: nullptr;
	if (!World)
	{
		return nullptr;
	}

	if (const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0))
	{
		if (const APawn* Pawn = PlayerController->GetPawn())
		{
			if (UVoiceRecognitionComponent* PawnVoice = Pawn->FindComponentByClass<UVoiceRecognitionComponent>())
			{
				return PawnVoice;
			}
		}

		if (UVoiceRecognitionComponent* ControllerVoice =
			PlayerController->FindComponentByClass<UVoiceRecognitionComponent>())
		{
			return ControllerVoice;
		}
	}

	for (TActorIterator<AActor> It(const_cast<UWorld*>(World)); It; ++It)
	{
		if (UVoiceRecognitionComponent* ActorVoice = It->FindComponentByClass<UVoiceRecognitionComponent>())
		{
			return ActorVoice;
		}
	}

	return nullptr;
}

bool UVoiceRecognitionComponent::BeginBackendListening_Implementation(const FVoiceRecognitionRequest& Request)
{
	// The base class has no backend. Subclasses open the microphone here.
	return true;
}

void UVoiceRecognitionComponent::EndBackendListening_Implementation()
{
}

void UVoiceRecognitionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopListening();
	Super::EndPlay(EndPlayReason);
}
