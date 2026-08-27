#include "Core/Text/HangulTextLibrary.h"

namespace
{
	constexpr TCHAR HangulSyllableFirst = 0xAC00;
	constexpr TCHAR HangulSyllableLast = 0xD7A3;
	constexpr int32 HangulMedialCount = 21;
	constexpr int32 HangulFinalCount = 28;

	/** The 19 standard leading consonants, in Unicode syllable order. */
	const TCHAR* GetLeadingConsonants()
	{
		static const TCHAR Leads[] = {
			0x3131, 0x3132, 0x3134, 0x3137, 0x3138, 0x3139, 0x3141, 0x3142, 0x3143, 0x3145,
			0x3146, 0x3147, 0x3148, 0x3149, 0x314A, 0x314B, 0x314C, 0x314D, 0x314E, 0
		};
		return Leads;
	}
}

TCHAR UHangulTextLibrary::GetInitialConsonant(const TCHAR Character)
{
	if (Character < HangulSyllableFirst || Character > HangulSyllableLast)
	{
		return 0;
	}

	const int32 SyllableIndex = static_cast<int32>(Character) - static_cast<int32>(HangulSyllableFirst);
	const int32 LeadIndex = SyllableIndex / (HangulMedialCount * HangulFinalCount);
	return GetLeadingConsonants()[LeadIndex];
}

bool UHangulTextLibrary::IsHangulSyllable(const int32 Character)
{
	return Character >= static_cast<int32>(HangulSyllableFirst)
		&& Character <= static_cast<int32>(HangulSyllableLast);
}

FString UHangulTextLibrary::ExtractInitialConsonants(const FString& Source, const FString& Separator)
{
	FString Result;
	Result.Reserve(Source.Len() * (1 + Separator.Len()));

	for (const TCHAR Character : Source)
	{
		if (FChar::IsWhitespace(Character))
		{
			// Spoken answers are single words; a space in the source only separates display groups.
			continue;
		}

		if (!Result.IsEmpty())
		{
			Result.Append(Separator);
		}

		const TCHAR Lead = GetInitialConsonant(Character);
		Result.AppendChar(Lead != 0 ? Lead : Character);
	}

	return Result;
}

FText UHangulTextLibrary::ExtractInitialConsonantsText(const FText& Source, const FString& Separator)
{
	return FText::FromString(ExtractInitialConsonants(Source.ToString(), Separator));
}

FString UHangulTextLibrary::NormalizeAnswer(const FString& Answer)
{
	FString Normalized;
	Normalized.Reserve(Answer.Len());

	for (const TCHAR Character : Answer)
	{
		if (FChar::IsWhitespace(Character) || FChar::IsPunct(Character))
		{
			continue;
		}
		Normalized.AppendChar(FChar::ToLower(Character));
	}

	return Normalized;
}

bool UHangulTextLibrary::DoesAnswerMatch(const FString& Answer, const TArray<FString>& AcceptedAnswers)
{
	const FString Normalized = NormalizeAnswer(Answer);
	if (Normalized.IsEmpty())
	{
		return false;
	}

	return AcceptedAnswers.ContainsByPredicate(
		[&Normalized](const FString& Accepted)
		{
			return NormalizeAnswer(Accepted) == Normalized;
		});
}
