#include "Core/Quiz/InitialConsonantQuizSet.h"

bool UInitialConsonantQuizSet::FindQuiz(const FName QuizID, FInitialConsonantQuizDefinition& OutQuiz) const
{
	const FInitialConsonantQuizDefinition* Found = Quizzes.FindByPredicate(
		[QuizID](const FInitialConsonantQuizDefinition& Quiz)
		{
			return Quiz.QuizID == QuizID;
		});

	if (!Found)
	{
		return false;
	}

	OutQuiz = *Found;
	return true;
}

bool UInitialConsonantQuizSet::ValidateQuizSet(FString& OutError) const
{
	TSet<FName> SeenIDs;
	for (const FInitialConsonantQuizDefinition& Quiz : Quizzes)
	{
		if (Quiz.QuizID.IsNone())
		{
			OutError = TEXT("A quiz entry has no Quiz ID.");
			return false;
		}
		if (Quiz.Answer.IsEmpty())
		{
			OutError = FString::Printf(TEXT("Quiz %s has no answer."), *Quiz.QuizID.ToString());
			return false;
		}
		bool bAlreadySeen = false;
		SeenIDs.Add(Quiz.QuizID, &bAlreadySeen);
		if (bAlreadySeen)
		{
			OutError = FString::Printf(TEXT("Quiz ID %s is used more than once."), *Quiz.QuizID.ToString());
			return false;
		}
	}

	OutError.Reset();
	return true;
}
