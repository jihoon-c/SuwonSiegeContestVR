#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/Quiz/InitialConsonantQuizTypes.h"
#include "Core/Scenario/ScenarioTypes.h"
#include "ScenarioQuizBridgeComponent.generated.h"

class UInitialConsonantQuizComponent;
class UScenarioManagerComponent;

/**
 * Optional adapter that runs an initial-consonant quiz for every Scenario interaction of type Quiz.
 *
 * Put this next to a UScenarioManagerComponent and a UInitialConsonantQuizComponent, then author a
 * Quiz interaction whose TargetID is the Quiz ID. No Blueprint wiring is needed: the bridge starts
 * the quiz and reports the result back to the scenario.
 */
UCLASS(ClassGroup = (Scenario), meta = (BlueprintSpawnableComponent))
class SUWONSIEGECONTESTVR_API UScenarioQuizBridgeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UScenarioQuizBridgeComponent();

	UFUNCTION(BlueprintCallable, Category = "Scenario|Quiz")
	bool InitializeBridge();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Quiz")
	bool bInitializeOnBeginPlay = true;

	/**
	 * Advances the scenario even when the player never answered correctly. Education content
	 * normally continues after the answer is revealed rather than blocking on a wrong answer.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Quiz")
	bool bCompleteInteractionOnWrongAnswer = true;

	/** Optional explicit quiz component. Left empty, the owner's component is used. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Quiz")
	TObjectPtr<UInitialConsonantQuizComponent> QuizComponentOverride;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleInteractionRequested(FScenarioInteraction Interaction);

	UFUNCTION()
	void HandleQuizFinished(FName QuizID, bool bCorrect, EInitialConsonantQuizOutcome Outcome);

	void Unbind();

	UPROPERTY(Transient)
	TObjectPtr<UScenarioManagerComponent> ScenarioManager;

	UPROPERTY(Transient)
	TObjectPtr<UInitialConsonantQuizComponent> QuizComponent;

	FName PendingInteractionID;
	FName PendingQuizID;
};
