#pragma once

#include "Core/Scenario/ScenarioDefinition.h"
#include "Main/Education/MainEducationTypes.h"
#include "MainEducationScenarioDefinition.generated.h"

/** Project-owned authoring asset containing the complete Main educational route. */
UCLASS(BlueprintType)
class SUWONSIEGECONTESTVR_API UMainEducationScenarioDefinition : public UScenarioDefinition
{
	GENERATED_BODY()

public:
	UMainEducationScenarioDefinition();

	/** Primary designer surface. Reorder these arrays to change the play flow. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "01 Editor Flow", meta = (TitleProperty = "StageID"))
	TArray<FMainEducationAuthoringStage> EditorFlow;

	/** Rebuilds generated Core Scenario Stages and presentation lookup data from EditorFlow. */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "01 Editor Flow")
	void RebuildScenarioFromEditorFlow();

	/** Replaces the Main route with the Singijeon -> Ongseong flow, quizzes and travel included. */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "01 Editor Flow")
	void ResetToSingijeonAndOngseongFlow();

	/**
	 * Sets the Main-level gate sequence: greeting, then each experience as
	 * explain -> spoken initial-consonant quiz -> travel -> return.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "01 Editor Flow")
	void ResetToMainGatePresentationFlow();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "01 Editor Flow")
	bool bAutoRebuildFromEditorFlow = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generated Runtime", meta = (TitleProperty = "ContentID"))
	TArray<FMainEducationContent> EducationContent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Education", meta = (TitleProperty = "RouteID"))
	TArray<FMainEducationExperienceRoute> ExperienceRoutes;

	UFUNCTION(BlueprintPure, Category = "Education")
	bool FindEducationContent(FName ContentID, FMainEducationContent& OutContent) const;

	UFUNCTION(BlueprintPure, Category = "Education")
	bool FindExperienceRoute(FName RouteID, FMainEducationExperienceRoute& OutRoute) const;

	UFUNCTION(BlueprintPure, Category = "Education|Quiz")
	bool IsAcceptedQuizAnswer(FName QuizID, const FString& Answer) const;

	/** Validates both the Core Scenario graph and Main presentation references. */
	UFUNCTION(BlueprintPure, Category = "Education")
	bool ValidateEducationScenario(FString& OutError) const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	void BuildDefaultContent();
	void BuildEditorFlowFromRuntime();
	void RebuildRuntimeFromEditorFlow();
	static FString NormalizeAnswer(const FString& Answer);
	bool bIsRebuildingEditorFlow = false;
};
