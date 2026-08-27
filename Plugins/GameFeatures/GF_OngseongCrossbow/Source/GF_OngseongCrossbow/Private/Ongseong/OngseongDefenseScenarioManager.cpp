#include "Ongseong/OngseongDefenseScenarioManager.h"

#include "GF_OngseongCrossbow.h"

#include "Components/SceneComponent.h"
#include "Components/InputComponent.h"
#include "Core/Experience/ExperienceSubsystem.h"
#include "Core/Quiz/InitialConsonantQuizComponent.h"
#include "Core/VR/VRPlayerPawn.h"
// Explicit: this file is compiled outside the unity blob whenever it is being edited.
#include "Engine/GameInstance.h"
#include "Gameplay/Combat/HealthComponent.h"
#include "Gameplay/Pooling/ActorPool.h"
#include "Gameplay/UI/VRHUDComponent.h"
#include "Gameplay/UI/VRHUDTypes.h"
#include "Components/AudioComponent.h"
#include "InputCoreTypes.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Ongseong/ChongtongCannonActor.h"
#include "Ongseong/ChongtongInteractionTypes.h"
#include "Ongseong/OngseongEnemyWaveManager.h"
#include "Ongseong/OngseongGateActor.h"
#include "Ongseong/OngseongNarrationComponent.h"
#include "Ongseong/OngseongRamActor.h"
#include "Ongseong/OngseongSpawnPointActor.h"
#include "EngineUtils.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "OngseongDefense"

AOngseongDefenseScenarioManager::AOngseongDefenseScenarioManager()
{
	PrimaryActorTick.bCanEverTick = false;
	// Placeable in the editor: the retreat fallback and ram staging read this actor's transform.
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Narration = CreateDefaultSubobject<UOngseongNarrationComponent>(TEXT("OngseongNarration"));
	RamClass = AOngseongRamActor::StaticClass();

	// The quiz runtime is Core; only the question belongs to this experience. Designers can edit or
	// replace this entry, point the component at a shared quiz set, or turn the step off entirely.
	IntroQuiz = CreateDefaultSubobject<UInitialConsonantQuizComponent>(TEXT("IntroQuiz"));
	FInitialConsonantQuizDefinition OngseongQuiz;
	OngseongQuiz.QuizID = IntroQuizID;
	OngseongQuiz.QuestionText = LOCTEXT("IntroQuizQuestion", "성문 바깥을 둘러싸 지키는 이 방어시설의 이름은?");
	// InitialConsonants is left empty on purpose: "옹성" derives "ㅇ ㅅ".
	OngseongQuiz.Answer = LOCTEXT("IntroQuizAnswer", "옹성");
	OngseongQuiz.HintText = LOCTEXT("IntroQuizHint", "지금 서 있는 이 성벽을 떠올려 보세요");
	IntroQuiz->Quizzes.Add(OngseongQuiz);
}

void AOngseongDefenseScenarioManager::BeginPlay()
{
	Super::BeginPlay();
	if (!IsValid(RamSpawnPoint))
	{
		for (TActorIterator<AOngseongSpawnPointActor> It(GetWorld()); It; ++It)
		{
			if (It->MatchesRole(EOngseongSpawnPointRole::Ram))
			{
				RamSpawnPoint = *It;
				break;
			}
		}
	}
	if (GateActor) GateActor->OnGateDestroyed.AddUniqueDynamic(this, &AOngseongDefenseScenarioManager::HandleGateDestroyed);
	if (WaveManager)
	{
		WaveManager->OnAllEnemiesRetreated.AddUniqueDynamic(this, &AOngseongDefenseScenarioManager::HandleAllEnemiesRetreated);
	}
	Narration->GateActor = GateActor;
	Narration->WaveManager = WaveManager;
	Narration->InitializeNarrationBindings();
	SetupNarrationSkipInput();
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (APawn* Pawn = PC->GetPawn()) VRHUD = Pawn->FindComponentByClass<UVRHUDComponent>();
	}
	ApplyPlayerLocomotionPolicy();
	if (bStartAfterChongtongLoaded)
	{
		ArmTrainingGate();
	}
	else if (bAutoStart)
	{
		StartDefense();
	}
}

void AOngseongDefenseScenarioManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
	StopBattleMusic();
	if (TrainingCannon)
	{
		TrainingCannon->OnLoadingStateChanged.RemoveDynamic(this, &AOngseongDefenseScenarioManager::HandleTrainingLoadingStateChanged);
	}
	if (Narration)
	{
		Narration->OnNarrationIdle.RemoveDynamic(this, &AOngseongDefenseScenarioManager::HandleBriefingNarrationIdle);
	}
	if (IntroQuiz)
	{
		IntroQuiz->OnQuizFinished.RemoveDynamic(this, &AOngseongDefenseScenarioManager::HandleIntroQuizFinished);
		// Releases the microphone with the level; a quiz never outlives the experience that ran it.
		IntroQuiz->CancelQuiz();
	}
	if (ActiveRam) ActiveRam->StopRam();
	if (WaveManager)
	{
		WaveManager->OnAllEnemiesRetreated.RemoveDynamic(this, &AOngseongDefenseScenarioManager::HandleAllEnemiesRetreated);
	}
	if (GateActor) GateActor->OnGateDestroyed.RemoveDynamic(this, &AOngseongDefenseScenarioManager::HandleGateDestroyed);
	if (APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
	{
		DisableInput(PlayerController);
	}
	Super::EndPlay(EndPlayReason);
}

void AOngseongDefenseScenarioManager::ApplyPlayerLocomotionPolicy()
{
	if (!bLockPlayerToBattlement || !GetWorld()) return;
	const APlayerController* PC = GetWorld()->GetFirstPlayerController();
	// The non-VR combat test pawn is not a VR pawn, so this simply does nothing there.
	if (AVRPlayerPawn* VRPawn = PC ? Cast<AVRPlayerPawn>(PC->GetPawn()) : nullptr)
	{
		// Keep free/smooth movement locked for scenario staging, but retain the stock XR
		// template teleport locomotion requested for the Ongseong battlement.
		VRPawn->SetLocomotionEnabled(false, true);
		UE_LOG(LogOngseong, Display, TEXT("Player smooth movement locked; teleport remains enabled on the battlement."));
	}
}

void AOngseongDefenseScenarioManager::ResolveTrainingCannon()
{
	if (IsValid(TrainingCannon) || !GetWorld()) return;
	// The trainee operates the playable cannon nearest their spawn position; every other cannon
	// on the wall is crewed by an ally. Do not rely on TActorIterator order here.
	FVector ReferenceLocation = GetActorLocation();
	bool bFoundPlayerReference = false;
	if (const APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (const APawn* PlayerPawn = PlayerController->GetPawn())
		{
			ReferenceLocation = PlayerPawn->GetActorLocation();
			bFoundPlayerReference = true;
		}
	}
	if (!bFoundPlayerReference)
	{
		for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
		{
			ReferenceLocation = It->GetActorLocation();
			break;
		}
	}

	AChongtongCannonActor* FirstCannon = nullptr;
	AChongtongCannonActor* NearestPlayerCannon = nullptr;
	float NearestPlayerCannonDistanceSquared = TNumericLimits<float>::Max();
	for (TActorIterator<AChongtongCannonActor> It(GetWorld()); It; ++It)
	{
		AChongtongCannonActor* Cannon = *It;
		if (!FirstCannon) FirstCannon = Cannon;
		if (Cannon->IsPlayerOperable())
		{
			const float DistanceSquared = FVector::DistSquared(Cannon->GetActorLocation(), ReferenceLocation);
			if (DistanceSquared < NearestPlayerCannonDistanceSquared)
			{
				NearestPlayerCannon = Cannon;
				NearestPlayerCannonDistanceSquared = DistanceSquared;
			}
		}
	}
	TrainingCannon = NearestPlayerCannon ? NearestPlayerCannon : FirstCannon;
}

void AOngseongDefenseScenarioManager::SetupNarrationSkipInput()
{
#if !UE_BUILD_SHIPPING
	if (!bEnableSpacebarNarrationSkip || bNarrationSkipInputBound)
	{
		return;
	}

	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		EnableInput(PlayerController);
		if (InputComponent)
		{
			FInputKeyBinding& Binding = InputComponent->BindKey(
				EKeys::SpaceBar, IE_Pressed, this, &ThisClass::HandleSpacebarNarrationSkip);
			Binding.bConsumeInput = false;
			bNarrationSkipInputBound = true;
		}
	}
#endif
}

void AOngseongDefenseScenarioManager::HandleSpacebarNarrationSkip()
{
	SkipNarration();
}

void AOngseongDefenseScenarioManager::ArmTrainingGate()
{
	ResolveTrainingCannon();
	if (!IsValid(TrainingCannon))
	{
		UE_LOG(LogOngseong, Warning,
			TEXT("Loading-gated start is on but no chongtong was found. Starting the defense immediately instead."));
		StartDefense();
		return;
	}
	TrainingCannon->OnLoadingStateChanged.AddUniqueDynamic(this, &AOngseongDefenseScenarioManager::HandleTrainingLoadingStateChanged);
	TrainingCannon->PrepareForImmediatePlayerFire();
	UE_LOG(LogOngseong, Display, TEXT("Narration-gated assault armed for combat-ready cannon %s."), *GetNameSafe(TrainingCannon));
}

void AOngseongDefenseScenarioManager::HandleTrainingLoadingStateChanged(const EChongtongLoadingState NewState, int32)
{
	// The player cannon is made ready immediately; the assault starts once the queued narration ends.
	if (bTrainingComplete || NewState != EChongtongLoadingState::ReadyToAim) return;
	bTrainingComplete = true;
	if (TrainingCannon)
	{
		TrainingCannon->OnLoadingStateChanged.RemoveDynamic(this, &AOngseongDefenseScenarioManager::HandleTrainingLoadingStateChanged);
	}
	ScheduleAssaultAfterBriefing();
}

void AOngseongDefenseScenarioManager::ScheduleAssaultAfterBriefing()
{
	bWaitingForBriefing = true;
	Narration->OnNarrationIdle.AddUniqueDynamic(this, &AOngseongDefenseScenarioManager::HandleBriefingNarrationIdle);
	Narration->ReportScenarioEvent(BriefingNarrationEvent, TrainingCannon);
	// A level without a narration player never reports idle, so keep a deadline on the wait.
	GetWorldTimerManager().SetTimer(BriefingTimeoutHandle, this, &AOngseongDefenseScenarioManager::HandleBriefingNarrationIdle,
		FMath::Max(1.0f, BriefingTimeout), false);
	if (!Narration->IsNarrationBusy())
	{
		HandleBriefingNarrationIdle();
	}
}

void AOngseongDefenseScenarioManager::HandleBriefingNarrationIdle()
{
	if (!bWaitingForBriefing) return;
	bWaitingForBriefing = false;
	GetWorldTimerManager().ClearTimer(BriefingTimeoutHandle);
	Narration->OnNarrationIdle.RemoveDynamic(this, &AOngseongDefenseScenarioManager::HandleBriefingNarrationIdle);
	// The quiz sits between the briefing and the horn, so the enemy never arrives mid-question.
	if (StartIntroQuiz()) return;
	ScheduleAssault();
}

void AOngseongDefenseScenarioManager::ScheduleAssault()
{
	if (AssaultStartDelay <= 0.0f)
	{
		BeginAssault();
		return;
	}
	GetWorldTimerManager().SetTimer(AssaultDelayHandle, this, &AOngseongDefenseScenarioManager::BeginAssault, AssaultStartDelay, false);
}

bool AOngseongDefenseScenarioManager::StartIntroQuiz()
{
	if (!bRunIntroQuiz || bIntroQuizComplete || !IsValid(IntroQuiz) || IntroQuizID.IsNone()) return false;

	IntroQuiz->OnQuizFinished.AddUniqueDynamic(this, &AOngseongDefenseScenarioManager::HandleIntroQuizFinished);
	if (!IntroQuiz->StartQuiz(IntroQuizID))
	{
		// A missing or malformed quiz must never strand the experience before the assault.
		IntroQuiz->OnQuizFinished.RemoveDynamic(this, &AOngseongDefenseScenarioManager::HandleIntroQuizFinished);
		UE_LOG(LogOngseong, Warning, TEXT("Intro quiz %s could not start; continuing to the assault."),
			*IntroQuizID.ToString());
		return false;
	}

	UE_LOG(LogOngseong, Display, TEXT("Intro quiz %s is running before the assault."), *IntroQuizID.ToString());
	return true;
}

void AOngseongDefenseScenarioManager::HandleIntroQuizFinished(
	const FName QuizID, const bool bCorrect, const EInitialConsonantQuizOutcome Outcome)
{
	if (QuizID != IntroQuizID) return;
	if (IntroQuiz)
	{
		IntroQuiz->OnQuizFinished.RemoveDynamic(this, &AOngseongDefenseScenarioManager::HandleIntroQuizFinished);
	}
	// A cancel comes from a teardown or a restart, so it must not open the assault by itself.
	if (Outcome == EInitialConsonantQuizOutcome::Canceled) return;

	bIntroQuizComplete = true;
	UE_LOG(LogOngseong, Display, TEXT("Intro quiz finished (correct=%s). Starting the assault countdown."),
		bCorrect ? TEXT("true") : TEXT("false"));
	ScheduleAssault();
}

void AOngseongDefenseScenarioManager::BeginAssault()
{
	if (DefenseState == EOngseongDefenseState::Defending) return;
	StartDefense();
}

bool AOngseongDefenseScenarioManager::SkipNarration()
{
	return Narration && Narration->SkipNarration();
}

void AOngseongDefenseScenarioManager::PlayAssaultAudio()
{
	if (AssaultHornSound)
	{
		UGameplayStatics::PlaySound2D(this, AssaultHornSound, AssaultHornVolume);
	}
	if (!BattleMusic) return;
	if (!BattleMusicComponent)
	{
		BattleMusicComponent = UGameplayStatics::SpawnSound2D(this, BattleMusic, BattleMusicVolume, 1.0f, 0.0f, nullptr, false, false);
	}
	if (!BattleMusicComponent) return;
	if (BattleMusicFadeInTime > 0.0f)
	{
		BattleMusicComponent->FadeIn(BattleMusicFadeInTime, BattleMusicVolume);
	}
	else
	{
		BattleMusicComponent->Play();
	}
}

void AOngseongDefenseScenarioManager::StopBattleMusic()
{
	if (!BattleMusicComponent) return;
	if (BattleMusicFadeOutTime > 0.0f)
	{
		BattleMusicComponent->FadeOut(BattleMusicFadeOutTime, 0.0f);
	}
	else
	{
		BattleMusicComponent->Stop();
	}
	BattleMusicComponent = nullptr;
}

bool AOngseongDefenseScenarioManager::StartDefense()
{
	if (DefenseState == EOngseongDefenseState::Defending || !IsValid(GateActor) || !IsValid(WaveManager)) return false;
	RemainingDefenseTime = bUseDefenseTimeLimit ? FMath::Max(1.0f, DefenseDuration) : 0.0f;
	bRamDestroyed = false;
	if (!SpawnAndActivateRam()) return false;
	PlayAssaultAudio();
	WaveManager->ResetWave();
	SetDefenseState(EOngseongDefenseState::Defending);
	WaveManager->SetObjectiveTarget(GateActor);
	WaveManager->StartSpawning();
	if (bUseDefenseTimeLimit)
	{
		GetWorldTimerManager().SetTimer(DefenseTimerHandle, this, &AOngseongDefenseScenarioManager::TickDefenseTimer, 1.0f, true);
	}
	// The battle is timeboxed: the horn starts the cap that hands the player back to Main.
	if (BattleTimeLimit > 0.0f)
	{
		GetWorldTimerManager().SetTimer(BattleTimeLimitHandle, this,
			&AOngseongDefenseScenarioManager::HandleBattleTimeLimitReached, BattleTimeLimit, false);
	}
	Narration->ReportScenarioEvent(TEXT("ScenarioStarted"), this);
	UpdateHUDTime();
	UE_LOG(LogOngseong, Display, TEXT("Defense started. Ram=%s, enemy slots=%d, time limit=%s, battle cap=%s"),
		*GetNameSafe(ActiveRam),
		WaveManager->GetMaxConcurrentEnemies(),
		bUseDefenseTimeLimit ? *FString::Printf(TEXT("%.0fs"), DefenseDuration) : TEXT("off"),
		BattleTimeLimit > 0.0f ? *FString::Printf(TEXT("%.0fs"), BattleTimeLimit) : TEXT("off"));
	return true;
}

float AOngseongDefenseScenarioManager::GetRemainingBattleTime() const
{
	if (BattleTimeLimit <= 0.0f || !GetWorld()) return 0.0f;
	const float Remaining = GetWorldTimerManager().GetTimerRemaining(BattleTimeLimitHandle);
	return Remaining > 0.0f ? Remaining : 0.0f;
}

int32 AOngseongDefenseScenarioManager::GetTotalDefeatedEnemies() const
{
	return IsValid(WaveManager) ? WaveManager->GetTotalDefeatedEnemies() : 0;
}

void AOngseongDefenseScenarioManager::RetryDefense()
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
	ReleaseActiveRam();
	if (WaveManager) WaveManager->ResetWave();
	if (GateActor) GateActor->ResetGate();
	bRamDestroyed = false;
	bCompletionRequested = false;
	SetDefenseState(EOngseongDefenseState::Idle);
	StartDefense();
}

bool AOngseongDefenseScenarioManager::SpawnAndActivateRam()
{
	if (!IsValid(GateActor) || !IsValid(WaveManager) || ActiveRam) return false;
	const FTransform SpawnTransform = IsValid(RamSpawnPoint) ? RamSpawnPoint->GetActorTransform() : WaveManager->GetActorTransform();

	if (IsValid(RamPool))
	{
		ActiveRam = Cast<AOngseongRamActor>(RamPool->AcquireActor(SpawnTransform));
	}
	else if (RamClass)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		ActiveRam = GetWorld()->SpawnActor<AOngseongRamActor>(RamClass, SpawnTransform, Params);
	}

	if (!ActiveRam)
	{
		UE_LOG(LogOngseong, Warning, TEXT("Could not send in the ram. Check the ram pool size and RamClass."));
		return false;
	}
	if (UHealthComponent* RamHealth = ActiveRam->FindComponentByClass<UHealthComponent>())
	{
		RamHealth->OnDeath.AddUniqueDynamic(this, &AOngseongDefenseScenarioManager::HandleRamDefeated);
	}
	ActiveRam->ActivateRam(GateActor);
	// Archers that cannot get an attack slot on a cannon rally on their own siege engine.
	WaveManager->SetArcherEscortTarget(ActiveRam);
	return true;
}

void AOngseongDefenseScenarioManager::ReleaseActiveRam()
{
	if (!ActiveRam) return;
	AOngseongRamActor* RamToRelease = ActiveRam;
	ActiveRam = nullptr;
	if (IsValid(WaveManager)) WaveManager->SetArcherEscortTarget(nullptr);
	RamToRelease->StopRam();
	if (UHealthComponent* RamHealth = RamToRelease->FindComponentByClass<UHealthComponent>())
	{
		RamHealth->OnDeath.RemoveDynamic(this, &AOngseongDefenseScenarioManager::HandleRamDefeated);
	}
	if (IsValid(RamPool) && RamPool->ReleaseActor(RamToRelease))
	{
		return;
	}
	RamToRelease->Destroy();
}

void AOngseongDefenseScenarioManager::TickDefenseTimer()
{
	if (DefenseState != EOngseongDefenseState::Defending || !bUseDefenseTimeLimit) return;
	RemainingDefenseTime = FMath::Max(0.0f, RemainingDefenseTime - 1.0f);
	UpdateHUDTime();
	if (RemainingDefenseTime <= 0.0f)
	{
		FailDefense(
			TEXT("DefenseTimedOut"),
			LOCTEXT("TimeoutHeadline", "옹성 방어 실패"),
			LOCTEXT("TimeoutDetail", "제한 시간 안에 적 충차를 파괴하지 못했습니다"),
			LOCTEXT("TimeoutNotification", "시간이 초과되었습니다"));
	}
}

void AOngseongDefenseScenarioManager::SucceedDefense()
{
	if (DefenseState != EOngseongDefenseState::Defending) return;
	GetWorldTimerManager().ClearTimer(DefenseTimerHandle);
	SetDefenseState(EOngseongDefenseState::Succeeded);
	UE_LOG(LogOngseong, Display, TEXT("Defense succeeded: the ram was destroyed after %d enemies were defeated."), GetTotalDefeatedEnemies());
	Narration->ReportScenarioEvent(TEXT("DefenseSucceeded"), this);
	if (VRHUD)
	{
		VRHUD->SetObjective(LOCTEXT("SuccessHeadline", "옹성 방어 성공"), LOCTEXT("SuccessDetail", "적이 퇴각하고 있습니다"));
		VRHUD->ShowNotification(LOCTEXT("SuccessNotification", "적 충차를 파괴했습니다"), EVRHUDNotificationType::Success, 5.0f);
	}
	StopBattleMusic();
	if (WaveManager)
	{
		const FVector RetreatLocation = RetreatPoint ? RetreatPoint->GetActorLocation() : GetActorLocation();
		WaveManager->RetreatAllEnemies(RetreatLocation);
	}
	if (SuccessCompletionDelay > 0.0f)
	{
		// Give the mission-clear narration room to finish before the Experience takes the player away.
		GetWorldTimerManager().SetTimer(SuccessCompletionHandle, this, &AOngseongDefenseScenarioManager::FinishSuccessfulRetreat,
			SuccessCompletionDelay, false);
	}
	else if (!WaveManager)
	{
		FinishSuccessfulRetreat();
	}
}

void AOngseongDefenseScenarioManager::FailDefense(const FName NarrationEvent, const FText& Headline, const FText& Detail, const FText& Notification)
{
	if (DefenseState != EOngseongDefenseState::Defending) return;
	GetWorldTimerManager().ClearTimer(DefenseTimerHandle);
	if (ActiveRam) ActiveRam->StopRam();
	if (WaveManager) WaveManager->ReleaseAllEnemies();
	StopBattleMusic();
	SetDefenseState(EOngseongDefenseState::Failed);
	UE_LOG(LogOngseong, Warning, TEXT("Defense failed (%s)."), *NarrationEvent.ToString());
	Narration->ReportScenarioEvent(NarrationEvent, GateActor);
	if (VRHUD)
	{
		const FText RetryDetail = bAutoRetryOnFailure
			? FText::Format(LOCTEXT("AutoRetryDetail", "{0} · {1}초 후 자동으로 다시 시작합니다"), Detail, FText::AsNumber(FMath::CeilToInt(AutoRetryDelay)))
			: Detail;
		VRHUD->SetObjective(Headline, RetryDetail);
		VRHUD->ShowNotification(Notification, EVRHUDNotificationType::Error, 5.0f);
	}
	if (bAutoRetryOnFailure)
	{
		GetWorldTimerManager().SetTimer(AutoRetryTimerHandle, this, &AOngseongDefenseScenarioManager::HandleAutoRetry, FMath::Max(1.0f, AutoRetryDelay), false);
	}
}

void AOngseongDefenseScenarioManager::FinishSuccessfulRetreat()
{
	if (!bReturnToMainOnSuccess) return;
	RequestReturnToMain();
}

void AOngseongDefenseScenarioManager::HandleBattleTimeLimitReached()
{
	if (bCompletionRequested) return;
	UE_LOG(LogOngseong, Display,
		TEXT("Battle time cap of %.0fs reached in state %d; returning to Main."),
		BattleTimeLimit, static_cast<int32>(DefenseState));
	if (VRHUD)
	{
		VRHUD->SetObjective(LOCTEXT("TimeUpHeadline", "옹성 체험 종료"),
			LOCTEXT("TimeUpDetail", "성문 앞으로 돌아갑니다"));
		VRHUD->ShowNotification(LOCTEXT("TimeUpNotification", "체험 시간이 끝났습니다"),
			EVRHUDNotificationType::Info, 4.0f);
	}
	FinishExperienceNow();
}

bool AOngseongDefenseScenarioManager::FinishExperienceNow()
{
	if (bCompletionRequested) return false;

	// Quiet the battle before the handoff so nothing keeps fighting through the level transition.
	GetWorldTimerManager().ClearTimer(BattleTimeLimitHandle);
	GetWorldTimerManager().ClearTimer(DefenseTimerHandle);
	GetWorldTimerManager().ClearTimer(AutoRetryTimerHandle);
	GetWorldTimerManager().ClearTimer(SuccessCompletionHandle);
	if (ActiveRam) ActiveRam->StopRam();
	if (WaveManager) WaveManager->ReleaseAllEnemies();
	StopBattleMusic();
	return RequestReturnToMain();
}

bool AOngseongDefenseScenarioManager::RequestReturnToMain()
{
	if (bCompletionRequested) return false;
	bCompletionRequested = true;
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UExperienceSubsystem* Experience = GameInstance->GetSubsystem<UExperienceSubsystem>())
		{
			// Main resumes at its return checkpoint, which leads into the closing greeting.
			return Experience->CompleteCurrentExperience(true);
		}
	}
	return false;
}

void AOngseongDefenseScenarioManager::HandleGateDestroyed()
{
	FailDefense(
		TEXT("GateDestroyed"),
		LOCTEXT("GateLostHeadline", "옹성 방어 실패"),
		LOCTEXT("GateLostDetail", "성문이 파괴되었습니다"),
		LOCTEXT("GateLostNotification", "성문이 파괴되었습니다"));
}

void AOngseongDefenseScenarioManager::HandleAllEnemiesRetreated()
{
	// With a completion delay the timer owns the handoff, so the clear line is never cut short.
	if (DefenseState == EOngseongDefenseState::Succeeded && SuccessCompletionDelay <= 0.0f) FinishSuccessfulRetreat();
}

void AOngseongDefenseScenarioManager::HandleRamDefeated(UHealthComponent* DeadHealth, const FCombatDamageSpec&)
{
	if (!IsValid(DeadHealth) || DeadHealth->GetOwner() != ActiveRam)
	{
		return;
	}

	// Destroying the ram is the clear condition. It is never replaced.
	bRamDestroyed = true;
	ReleaseActiveRam();
	SucceedDefense();
}

void AOngseongDefenseScenarioManager::SetDefenseState(const EOngseongDefenseState NewState)
{
	if (DefenseState == NewState) return;
	DefenseState = NewState;
	OnDefenseStateChanged.Broadcast(NewState);
}

void AOngseongDefenseScenarioManager::UpdateHUDTime()
{
	const int32 Seconds = FMath::CeilToInt(RemainingDefenseTime);
	OnDefenseTimeChanged.Broadcast(Seconds);
	if (!VRHUD)
	{
		return;
	}

	const FText Objective = LOCTEXT("DefenseObjective", "적 충차를 파괴하십시오");
	if (bUseDefenseTimeLimit)
	{
		const FString TimeText = FString::Printf(TEXT("%02d:%02d"), Seconds / 60, Seconds % 60);
		VRHUD->SetObjective(Objective, FText::Format(LOCTEXT("DefenseRemaining", "남은 시간 {0}"), FText::FromString(TimeText)));
	}
	else
	{
		VRHUD->SetObjective(Objective, LOCTEXT("DefenseDetail", "충차가 성문에 닿기 전에 총통으로 파괴하십시오"));
	}
}

void AOngseongDefenseScenarioManager::HandleAutoRetry()
{
	if (DefenseState == EOngseongDefenseState::Failed) RetryDefense();
}

#undef LOCTEXT_NAMESPACE
