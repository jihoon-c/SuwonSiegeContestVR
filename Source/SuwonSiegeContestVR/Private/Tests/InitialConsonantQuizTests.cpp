#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/Quiz/InitialConsonantQuizComponent.h"
#include "Core/Quiz/InitialConsonantQuizTypes.h"
#include "Core/Text/HangulTextLibrary.h"
#include "Core/Voice/MockVoiceRecognitionComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHangulInitialConsonantsTest,
	"Suwon.Core.Hangul.InitialConsonants",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHangulInitialConsonantsTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("옹성 shows two consonants"),
		UHangulTextLibrary::ExtractInitialConsonants(TEXT("옹성")), FString(TEXT("ㅇ ㅅ")));
	TestEqual(TEXT("공심돈 shows three consonants"),
		UHangulTextLibrary::ExtractInitialConsonants(TEXT("공심돈")), FString(TEXT("ㄱ ㅅ ㄷ")));
	TestEqual(TEXT("거중기 shows three consonants"),
		UHangulTextLibrary::ExtractInitialConsonants(TEXT("거중기")), FString(TEXT("ㄱ ㅈ ㄱ")));
	TestEqual(TEXT("Double consonants are kept as written"),
		UHangulTextLibrary::ExtractInitialConsonants(TEXT("까치")), FString(TEXT("ㄲ ㅊ")));
	TestEqual(TEXT("Whitespace in the source does not produce empty groups"),
		UHangulTextLibrary::ExtractInitialConsonants(TEXT("옹 성")), FString(TEXT("ㅇ ㅅ")));
	TestEqual(TEXT("Non-Hangul characters survive"),
		UHangulTextLibrary::ExtractInitialConsonants(TEXT("VR성")), FString(TEXT("V R ㅅ")));
	TestEqual(TEXT("A custom separator is used"),
		UHangulTextLibrary::ExtractInitialConsonants(TEXT("옹성"), TEXT("")), FString(TEXT("ㅇㅅ")));

	TestEqual(TEXT("Spacing and punctuation are dropped when normalizing"),
		UHangulTextLibrary::NormalizeAnswer(TEXT(" 공 심 돈! ")), FString(TEXT("공심돈")));

	const TArray<FString> Accepted = { TEXT("옹성"), TEXT("옹성문") };
	TestTrue(TEXT("The canonical answer matches"), UHangulTextLibrary::DoesAnswerMatch(TEXT("옹성"), Accepted));
	TestTrue(TEXT("A spaced answer matches"), UHangulTextLibrary::DoesAnswerMatch(TEXT("옹 성"), Accepted));
	TestTrue(TEXT("An alias matches"), UHangulTextLibrary::DoesAnswerMatch(TEXT("옹성문"), Accepted));
	TestFalse(TEXT("A different word is rejected"), UHangulTextLibrary::DoesAnswerMatch(TEXT("공심돈"), Accepted));
	TestFalse(TEXT("A partial answer is rejected"), UHangulTextLibrary::DoesAnswerMatch(TEXT("옹"), Accepted));
	TestFalse(TEXT("An empty answer is rejected"), UHangulTextLibrary::DoesAnswerMatch(TEXT("   "), Accepted));

	FInitialConsonantQuizDefinition Quiz;
	Quiz.QuizID = TEXT("QUIZ_ONGSEONG");
	Quiz.Answer = FText::FromString(TEXT("옹성"));
	TestEqual(TEXT("Consonants are derived from the answer"),
		Quiz.GetDisplayConsonants().ToString(), FString(TEXT("ㅇ ㅅ")));
	Quiz.InitialConsonants = FText::FromString(TEXT("ㅇ-ㅅ"));
	TestEqual(TEXT("Authored consonants win over the derived ones"),
		Quiz.GetDisplayConsonants().ToString(), FString(TEXT("ㅇ-ㅅ")));
	TestEqual(TEXT("The answer is the first accepted spelling"),
		Quiz.GetAcceptedAnswerStrings()[0], FString(TEXT("옹성")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInitialConsonantQuizFlowTest,
	"Suwon.Core.Quiz.VoiceLifetime",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInitialConsonantQuizFlowTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("Test world is created"), World)) return false;
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	// Dynamic delegates are dropped until the world reports its actors as initialized.
	FURL URL;
	World->InitializeActorsForPlay(URL);
	World->BeginPlay();

	AActor* Host = World->SpawnActor<AActor>();
	UMockVoiceRecognitionComponent* Voice =
		NewObject<UMockVoiceRecognitionComponent>(Host, TEXT("TestVoice"));
	// ManualOnly keeps the test deterministic: nothing is recognized until it submits text.
	Voice->MockMode = EMockVoiceRecognitionMode::ManualOnly;
	Voice->RegisterComponent();

	UInitialConsonantQuizComponent* QuizComponent =
		NewObject<UInitialConsonantQuizComponent>(Host, TEXT("TestQuiz"));
	QuizComponent->bShowQuizPanel = false;
	QuizComponent->VoiceRecognitionOverride = Voice;

	FInitialConsonantQuizDefinition Quiz;
	Quiz.QuizID = TEXT("QUIZ_ONGSEONG");
	Quiz.Answer = FText::FromString(TEXT("옹성"));
	Quiz.MaxAttempts = 2;
	Quiz.ResultDisplayDuration = 0.0f;
	QuizComponent->Quizzes.Add(Quiz);
	QuizComponent->RegisterComponent();

	TestFalse(TEXT("The recognizer is idle before the quiz"), Voice->IsListening());

	TestTrue(TEXT("The quiz starts"), QuizComponent->StartQuiz(TEXT("QUIZ_ONGSEONG")));
	TestTrue(TEXT("The quiz is active"), QuizComponent->IsQuizActive());
	TestTrue(TEXT("Speech capture starts with the quiz"), Voice->IsListening());
	TestEqual(TEXT("The request carries the accepted answers"),
		Voice->GetActiveRequest().Keywords.Num(), 1);

	// A wrong answer spends one attempt and re-opens the microphone for the next one.
	TestFalse(TEXT("A wrong answer is rejected"), Voice->SubmitMockSpeech(TEXT("공심돈")));
	TestTrue(TEXT("The quiz is still running after a wrong answer"), QuizComponent->IsQuizActive());
	TestEqual(TEXT("One attempt was spent"), QuizComponent->GetAttemptCount(), 1);
	TestTrue(TEXT("Speech capture resumes for the retry"), Voice->IsListening());

	TestTrue(TEXT("The correct answer is accepted"), Voice->SubmitMockSpeech(TEXT(" 옹 성 ")));
	TestFalse(TEXT("The quiz finished"), QuizComponent->IsQuizActive());
	TestFalse(TEXT("Speech capture stops with the quiz"), Voice->IsListening());

	// Attempts run out: the flow must still finish rather than block the experience.
	TestTrue(TEXT("The quiz restarts"), QuizComponent->StartQuiz(TEXT("QUIZ_ONGSEONG")));
	Voice->SubmitMockSpeech(TEXT("녹로"));
	TestTrue(TEXT("The quiz survives the first miss"), QuizComponent->IsQuizActive());
	Voice->SubmitMockSpeech(TEXT("녹로"));
	TestFalse(TEXT("The quiz ends once attempts run out"), QuizComponent->IsQuizActive());
	TestFalse(TEXT("Speech capture stops when attempts run out"), Voice->IsListening());

	// Cancel releases the microphone as well.
	QuizComponent->StartQuiz(TEXT("QUIZ_ONGSEONG"));
	TestTrue(TEXT("Speech capture is on again"), Voice->IsListening());
	QuizComponent->CancelQuiz();
	TestFalse(TEXT("Cancel stops speech capture"), Voice->IsListening());
	TestFalse(TEXT("Cancel ends the quiz"), QuizComponent->IsQuizActive());

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
