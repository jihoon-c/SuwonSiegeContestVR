#include "Core/Quiz/InitialConsonantQuizTypes.h"

#include "Core/Text/HangulTextLibrary.h"

TArray<FString> FInitialConsonantQuizDefinition::GetAcceptedAnswerStrings() const
{
	TArray<FString> Accepted;
	Accepted.Reserve(AcceptedAnswers.Num() + 1);

	if (!Answer.IsEmpty())
	{
		Accepted.Add(Answer.ToString());
	}
	for (const FText& Alias : AcceptedAnswers)
	{
		if (!Alias.IsEmpty())
		{
			Accepted.AddUnique(Alias.ToString());
		}
	}

	return Accepted;
}

FText FInitialConsonantQuizDefinition::GetDisplayConsonants() const
{
	if (!InitialConsonants.IsEmpty())
	{
		return InitialConsonants;
	}

	return UHangulTextLibrary::ExtractInitialConsonantsText(Answer);
}
