#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HangulTextLibrary.generated.h"

/**
 * Korean text helpers shared by the quiz and voice-recognition systems.
 * Feature-agnostic: nothing here knows about a specific experience.
 */
UCLASS()
class SUWONSIEGECONTESTVR_API UHangulTextLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Returns the leading consonants of every Hangul syllable, e.g. "옹성" -> "ㅇ ㅅ".
	 * Non-syllable characters are kept as-is so "3층 누각" still reads sensibly.
	 * Whitespace in the source collapses into the separator.
	 */
	UFUNCTION(BlueprintPure, Category = "Core|Hangul")
	static FString ExtractInitialConsonants(const FString& Source, const FString& Separator = TEXT(" "));

	UFUNCTION(BlueprintPure, Category = "Core|Hangul", meta = (DisplayName = "Extract Initial Consonants (Text)"))
	static FText ExtractInitialConsonantsText(const FText& Source, const FString& Separator = TEXT(" "));

	/** Drops whitespace and punctuation and lowercases the rest so spoken answers compare cleanly. */
	UFUNCTION(BlueprintPure, Category = "Core|Hangul")
	static FString NormalizeAnswer(const FString& Answer);

	/** True when Answer normalizes to any of the accepted spellings. Partial matches are rejected. */
	UFUNCTION(BlueprintPure, Category = "Core|Hangul")
	static bool DoesAnswerMatch(const FString& Answer, const TArray<FString>& AcceptedAnswers);

	/** True for the U+AC00..U+D7A3 precomposed syllable block. */
	UFUNCTION(BlueprintPure, Category = "Core|Hangul")
	static bool IsHangulSyllable(int32 Character);

	/** Returns the single leading consonant of one syllable, or 0 when the character is not a syllable. */
	static TCHAR GetInitialConsonant(TCHAR Character);
};
