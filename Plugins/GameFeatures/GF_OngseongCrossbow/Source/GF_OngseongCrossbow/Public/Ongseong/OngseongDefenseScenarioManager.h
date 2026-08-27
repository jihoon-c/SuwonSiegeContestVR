#pragma once

#include "CoreMinimal.h"
#include "Core/Quiz/InitialConsonantQuizTypes.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Combat/CombatTypes.h"
#include "Ongseong/ChongtongInteractionTypes.h"
#include "OngseongDefenseScenarioManager.generated.h"

class AActorPool;
class AChongtongCannonActor;
class UInitialConsonantQuizComponent;
class UAudioComponent;
class USoundBase;
class AOngseongEnemyWaveManager;
class AOngseongGateActor;
class AOngseongRamActor;
class UHealthComponent;
class UOngseongNarrationComponent;
class UVRHUDComponent;

UENUM(BlueprintType)
enum class EOngseongDefenseState : uint8
{
	Idle,
	Defending,
	Succeeded,
	Failed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOngseongDefenseStateChanged, EOngseongDefenseState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOngseongDefenseTimeChanged, int32, RemainingSeconds);

/**
 * Owns the ram assault, terminal states, retreat and Experience handoff.
 * The experience is cleared by destroying the enemy ram; losing the gate fails it.
 */
UCLASS(Blueprintable)
class GF_ONGSEONGCROSSBOW_API AOngseongDefenseScenarioManager : public AActor
{
	GENERATED_BODY()

public:
	AOngseongDefenseScenarioManager();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category="Ongseong|Scenario")
	bool StartDefense();
	UFUNCTION(BlueprintCallable, Category="Ongseong|Scenario")
	void RetryDefense();
	/** Skips the active Ongseong narration line only. */
	UFUNCTION(BlueprintCallable, Category="Ongseong|Scenario|Narration")
	bool SkipNarration();

	/**
	 * Sounds the horn, brings up the battle music and starts the defense.
	 * This is what the loading-gated flow calls once the instructor has finished the briefing.
	 */
	UFUNCTION(BlueprintCallable, Category="Ongseong|Scenario")
	void BeginAssault();

	/**
	 * Puts the reusable Core initial-consonant quiz in front of the player. The briefing calls this
	 * before the horn; the assault waits until the quiz reports a result.
	 */
	UFUNCTION(BlueprintCallable, Category="Ongseong|Scenario|Quiz")
	bool StartIntroQuiz();

	UFUNCTION(BlueprintPure, Category="Ongseong|Scenario|Quiz")
	bool IsIntroQuizComplete() const { return bIntroQuizComplete; }

	/** Off by default because Main asks the 옹성 quiz; turn it on for a standalone run of this level. */
	UFUNCTION(BlueprintCallable, Category="Ongseong|Scenario|Quiz")
	void SetRunIntroQuiz(bool bNewRunIntroQuiz) { bRunIntroQuiz = bNewRunIntroQuiz; }

	/**
	 * Stops the battle and hands the player straight back to Main, whatever the current state is.
	 * The battle time cap uses this; a Blueprint can call it to end the experience early.
	 * Returns true when this call requested the handoff.
	 */
	UFUNCTION(BlueprintCallable, Category="Ongseong|Scenario")
	bool FinishExperienceNow();

	UFUNCTION(BlueprintPure, Category="Ongseong|Scenario")
	float GetRemainingBattleTime() const;

	UFUNCTION(BlueprintPure, Category="Ongseong|Scenario|Quiz")
	UInitialConsonantQuizComponent* GetIntroQuiz() const { return IntroQuiz; }

	UFUNCTION(BlueprintCallable, Category="Ongseong|Scenario|Audio")
	void StopBattleMusic();

	UFUNCTION(BlueprintCallable, Category="Ongseong|Scenario")
	void SetGateActor(AOngseongGateActor* NewGateActor) { GateActor = NewGateActor; }
	UFUNCTION(BlueprintCallable, Category="Ongseong|Scenario")
	void SetWaveManager(AOngseongEnemyWaveManager* NewWaveManager) { WaveManager = NewWaveManager; }
	UFUNCTION(BlueprintCallable, Category="Ongseong|Scenario")
	void SetRamPool(AActorPool* NewRamPool) { RamPool = NewRamPool; }
	/** Must be set before BeginPlay: the gate is armed there. */
	UFUNCTION(BlueprintCallable, Category="Ongseong|Scenario|Start")
	void SetStartAfterChongtongLoaded(bool bNewStartAfterChongtongLoaded) { bStartAfterChongtongLoaded = bNewStartAfterChongtongLoaded; }
	UFUNCTION(BlueprintCallable, Category="Ongseong|Scenario|Start")
	void SetTrainingCannon(AChongtongCannonActor* NewTrainingCannon) { TrainingCannon = NewTrainingCannon; }
	/** Starts listening for the loading drill. BeginPlay calls this when the gated start is on. */
	UFUNCTION(BlueprintCallable, Category="Ongseong|Scenario|Start")
	void ArmTrainingGate();

	UFUNCTION(BlueprintPure, Category="Ongseong|Scenario")
	EOngseongDefenseState GetDefenseState() const { return DefenseState; }
	UFUNCTION(BlueprintPure, Category="Ongseong|Scenario")
	float GetRemainingDefenseTime() const { return RemainingDefenseTime; }
	UFUNCTION(BlueprintPure, Category="Ongseong|Scenario|Ram")
	AOngseongRamActor* GetActiveRam() const { return ActiveRam; }
	/** The ram is a single objective target. It is never replaced once destroyed. */
	UFUNCTION(BlueprintPure, Category="Ongseong|Scenario|Ram")
	bool IsRamDestroyed() const { return bRamDestroyed; }
	UFUNCTION(BlueprintPure, Category="Ongseong|Scenario")
	bool IsDefenseTimeLimited() const { return bUseDefenseTimeLimit; }
	/** True once the trainee has finished loading the cannon in the gated flow. */
	UFUNCTION(BlueprintPure, Category="Ongseong|Scenario|Start")
	bool IsTrainingComplete() const { return bTrainingComplete; }
	/** Cumulative enemies defeated during the current defense attempt. */
	UFUNCTION(BlueprintPure, Category="Ongseong|Scenario")
	int32 GetTotalDefeatedEnemies() const;

	UPROPERTY(BlueprintAssignable, Category="Ongseong|Scenario")
	FOnOngseongDefenseStateChanged OnDefenseStateChanged;
	UPROPERTY(BlueprintAssignable, Category="Ongseong|Scenario")
	FOnOngseongDefenseTimeChanged OnDefenseTimeChanged;
protected:
	void TickDefenseTimer();
	bool SpawnAndActivateRam();
	void ReleaseActiveRam();
	void SucceedDefense();
	void FailDefense(FName NarrationEvent, const FText& Headline, const FText& Detail, const FText& Notification);
	void FinishSuccessfulRetreat();
	void HandleBattleTimeLimitReached();
	/** Single handoff point back to Main. Runs once per experience. */
	bool RequestReturnToMain();
	void SetDefenseState(EOngseongDefenseState NewState);
	void UpdateHUDTime();
	void HandleAutoRetry();
	void ApplyPlayerLocomotionPolicy();

	void ResolveTrainingCannon();
	void SetupNarrationSkipInput();
	void HandleSpacebarNarrationSkip();
	void ScheduleAssaultAfterBriefing();
	void ScheduleAssault();
	void PlayAssaultAudio();

	UFUNCTION()
	void HandleIntroQuizFinished(FName QuizID, bool bCorrect, EInitialConsonantQuizOutcome Outcome);

	UFUNCTION()
	void HandleTrainingLoadingStateChanged(EChongtongLoadingState NewState, int32 CompletedShots);
	UFUNCTION()
	void HandleBriefingNarrationIdle();
	UFUNCTION()
	void HandleGateDestroyed();
	UFUNCTION()
	void HandleAllEnemiesRetreated();
	UFUNCTION()
	void HandleRamDefeated(UHealthComponent* DeadHealth, const FCombatDamageSpec& KillingDamage);

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Ongseong|Scenario")
	TObjectPtr<AOngseongGateActor> GateActor;
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Ongseong|Scenario")
	TObjectPtr<AOngseongEnemyWaveManager> WaveManager;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Scenario|Ram")
	TSubclassOf<AOngseongRamActor> RamClass;
	/** Optional. When set, rams are acquired and released instead of spawned and destroyed. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Ongseong|Scenario|Ram")
	TObjectPtr<AActorPool> RamPool;
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Ongseong|Scenario")
	TObjectPtr<AActor> RamSpawnPoint;
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Ongseong|Scenario")
	TObjectPtr<AActor> RetreatPoint;
	/** Optional time limit. The experience is normally cleared by destroying the ram, not by surviving. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Scenario")
	bool bUseDefenseTimeLimit = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Scenario", meta=(ClampMin="1.0", EditCondition="bUseDefenseTimeLimit"))
	float DefenseDuration = 180.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Scenario")
	bool bAutoStart = true;
	/** Development VR/PC shortcut: Space skips the active instructor narration. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Scenario|Narration", meta=(AdvancedDisplay))
	bool bEnableSpacebarNarrationSkip = true;
	/**
	 * Gated start used by the main level: nothing spawns until the trainee has loaded the cannon
	 * once. The briefing line plays first, then the assault begins after AssaultStartDelay.
	 * When this is on, bAutoStart is ignored.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Scenario|Start")
	bool bStartAfterChongtongLoaded = false;
	/** Optional. The player-operable cannon that gates the start. Auto-found when left empty. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Ongseong|Scenario|Start")
	TObjectPtr<AChongtongCannonActor> TrainingCannon;
	/** Pause between the end of the briefing line and the horn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Scenario|Start", meta=(ClampMin="0.0"))
	float AssaultStartDelay = 2.0f;
	/** Safety net so a missing or silent briefing line cannot stall the experience. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Scenario|Start", meta=(ClampMin="1.0"))
	float BriefingTimeout = 30.0f;
	/** Instructor line announcing that the drill is starting. Queued when loading completes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Scenario|Start")
	FName BriefingNarrationEvent = FName(TEXT("TrainingCompleted"));

	/**
	 * Runs the initial-consonant quiz between the briefing line and the horn.
	 * Off by default since 2026-08-27: the Main education flow asks the 옹성 quiz before travelling
	 * here, so asking again inside the level would repeat the same question. Turn it on to run this
	 * level standalone, or when Main stops asking it.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Scenario|Quiz")
	bool bRunIntroQuiz = false;
	/** Entry to run from the quiz component. The default 옹성 quiz is authored in the constructor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Scenario|Quiz")
	FName IntroQuizID = FName(TEXT("QUIZ_ONGSEONG"));
	/** Core quiz runtime. Feature code only supplies the data and the moment it runs. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ongseong|Scenario|Quiz")
	TObjectPtr<UInitialConsonantQuizComponent> IntroQuiz;

	/** War horn that opens the assault. Assign a Sound Cue in the Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Scenario|Audio")
	TObjectPtr<USoundBase> AssaultHornSound;
	/** Battle music that runs for the length of the assault. Assign a Sound Cue in the Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Scenario|Audio")
	TObjectPtr<USoundBase> BattleMusic;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Scenario|Audio", meta=(ClampMin="0.0"))
	float AssaultHornVolume = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Scenario|Audio", meta=(ClampMin="0.0"))
	float BattleMusicVolume = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Scenario|Audio", meta=(ClampMin="0.0"))
	float BattleMusicFadeInTime = 1.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Scenario|Audio", meta=(ClampMin="0.0"))
	float BattleMusicFadeOutTime = 3.0f;
	/**
	 * The player defends from a fixed post on the battlement, so locomotion stays off for this
	 * experience. Core keeps the flags; the Feature only states the policy.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Scenario|Player")
	bool bLockPlayerToBattlement = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Scenario")
	bool bReturnToMainOnSuccess = true;
	/**
	 * Hard cap on the battle, counted from the assault horn. When it runs out the experience hands
	 * the player back to Main immediately — no clear line, no retry — and the Main flow resumes at
	 * the closing greeting. 0 turns the cap off and the experience ends on the ram instead.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Scenario", meta=(ClampMin="0.0", Units="s"))
	float BattleTimeLimit = 30.0f;
	/**
	 * Time held after the ram falls so the mission-clear line can finish before the handoff.
	 * Set to 0 to end as soon as the enemies have retreated, as the experience did before.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Scenario", meta=(ClampMin="0.0"))
	float SuccessCompletionDelay = 10.0f;
	/** Educational fallback: restart cleanly after showing the failure reason. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Scenario")
	bool bAutoRetryOnFailure = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Scenario", meta=(ClampMin="1.0"))
	float AutoRetryDelay = 6.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ongseong|Scenario")
	TObjectPtr<UOngseongNarrationComponent> Narration;

	UPROPERTY(Transient)
	TObjectPtr<UVRHUDComponent> VRHUD;
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> BattleMusicComponent;
	UPROPERTY(Transient)
	TObjectPtr<AOngseongRamActor> ActiveRam;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Ongseong|Scenario")
	EOngseongDefenseState DefenseState = EOngseongDefenseState::Idle;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Ongseong|Scenario|Ram")
	bool bRamDestroyed = false;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Ongseong|Scenario|Start")
	bool bTrainingComplete = false;
	/** Stays true across a retry so the player is not quizzed again after a failed defense. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Ongseong|Scenario|Quiz")
	bool bIntroQuizComplete = false;
	bool bNarrationSkipInputBound = false;
	bool bWaitingForBriefing = false;
	bool bCompletionRequested = false;
	float RemainingDefenseTime = 0.0f;
	FTimerHandle DefenseTimerHandle;
	FTimerHandle AutoRetryTimerHandle;
	FTimerHandle AssaultDelayHandle;
	FTimerHandle BriefingTimeoutHandle;
	FTimerHandle SuccessCompletionHandle;
	FTimerHandle BattleTimeLimitHandle;
};
