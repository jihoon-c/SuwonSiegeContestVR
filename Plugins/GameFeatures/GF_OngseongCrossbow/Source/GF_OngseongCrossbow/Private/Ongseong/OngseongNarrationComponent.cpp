#include "Ongseong/OngseongNarrationComponent.h"

#include "Core/Narration/NarrationSequenceComponent.h"
#include "Gameplay/UI/VRHUDComponent.h"
#include "Engine/DataTable.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Gameplay/Characters/AllyCombatCharacter.h"
#include "Gameplay/Characters/EnemyCombatCharacter.h"
#include "Gameplay/Combat/HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Ongseong/ChongtongCannonActor.h"
#include "Ongseong/OngseongEnemyWaveManager.h"

#define LOCTEXT_NAMESPACE "OngseongVRUI"

namespace OngseongNarrationEvents
{
	const FName ScenarioStarted(TEXT("ScenarioStarted"));
	const FName WaveStarted(TEXT("WaveStarted"));
	const FName EnemyAssault(TEXT("EnemyAssault"));
	const FName PowderLoaded(TEXT("PowderLoaded"));
	const FName RammingCompleted(TEXT("RammingCompleted"));
	const FName ReadyToAim(TEXT("ReadyToAim"));
	const FName ReloadRequired(TEXT("ReloadRequired"));
	const FName AlliesUnderAttack(TEXT("AlliesUnderAttack"));
	const FName GateUnderAttack(TEXT("GateUnderAttack"));
	const FName DefenseSucceeded(TEXT("DefenseSucceeded"));
	const FName GateDestroyed(TEXT("GateDestroyed"));
}

UOngseongNarrationComponent::UOngseongNarrationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	NarrationTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/GF_OngseongCrossbow/Data/DT_OngseongNarration.DT_OngseongNarration")));

	const auto AddBinding = [this](const FName Event, const TCHAR* Row, const bool bOnce = true)
	{
		FOngseongNarrationEventBinding& Binding = EventBindings.AddDefaulted_GetRef();
		Binding.EventName = Event;
		Binding.NarrationRow = FName(Row);
		Binding.bPlayOnce = bOnce;
	};
	AddBinding(OngseongNarrationEvents::ScenarioStarted, TEXT("ON_01"));
	AddBinding(OngseongNarrationEvents::WaveStarted, TEXT("ON_10"));
	AddBinding(OngseongNarrationEvents::EnemyAssault, TEXT("ON_19"));
	AddBinding(OngseongNarrationEvents::PowderLoaded, TEXT("ON_14"), false);
	AddBinding(OngseongNarrationEvents::RammingCompleted, TEXT("ON_15"), false);
	AddBinding(OngseongNarrationEvents::ReadyToAim, TEXT("ON_16"), false);
	AddBinding(OngseongNarrationEvents::ReloadRequired, TEXT("ON_18"), false);
	AddBinding(OngseongNarrationEvents::AlliesUnderAttack, TEXT("ON_20"));
	AddBinding(OngseongNarrationEvents::GateUnderAttack, TEXT("ON_21"));
	AddBinding(OngseongNarrationEvents::DefenseSucceeded, TEXT("ON_22"));
	AddBinding(OngseongNarrationEvents::GateDestroyed, TEXT("ON_23"));
}

void UOngseongNarrationComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeNarrationBindings();
	if (bPlayIntroduction)
	{
		ReportScenarioEvent(OngseongNarrationEvents::ScenarioStarted, GetOwner());
	}
}

bool UOngseongNarrationComponent::InitializeNarrationBindings()
{
	Cannon = Cast<AChongtongCannonActor>(GetOwner());
	if (Cannon)
	{
		Cannon->OnLoadingStateChanged.AddUniqueDynamic(this, &ThisClass::HandleLoadingStateChanged);
		Cannon->OnRammingProgress.AddUniqueDynamic(this, &ThisClass::HandleRammingProgress);
	}

	if (!WaveManager && GetWorld())
	{
		for (TActorIterator<AOngseongEnemyWaveManager> It(GetWorld()); It; ++It)
		{
			WaveManager = *It;
			break;
		}
	}
	if (WaveManager)
	{
		WaveManager->OnWaveStarted.AddUniqueDynamic(this, &ThisClass::HandleWaveStarted);
		WaveManager->OnEnemySpawned.AddUniqueDynamic(this, &ThisClass::HandleEnemySpawned);
		WaveManager->OnWaveProgress.AddUniqueDynamic(this, &ThisClass::HandleWaveProgress);
		WaveManager->OnAllEnemiesDefeated.AddUniqueDynamic(this, &ThisClass::HandleAllEnemiesDefeated);
		if (!GateActor)
		{
			GateActor = WaveManager->GetObjectiveTarget();
		}
	}

	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		NarrationSequence = PlayerPawn->FindComponentByClass<UNarrationSequenceComponent>();
		VRHUD = PlayerPawn->FindComponentByClass<UVRHUDComponent>();
	}
	if (NarrationSequence)
	{
		NarrationSequence->OnSequenceFinished.AddUniqueDynamic(this, &ThisClass::HandleNarrationFinished);
	}

	BindHealthActor(GateActor, true);
	if (AlliedDefenseActors.IsEmpty() && GetWorld())
	{
		for (TActorIterator<AAllyCombatCharacter> It(GetWorld()); It; ++It)
		{
			AlliedDefenseActors.Add(*It);
		}
	}
	for (AActor* Ally : AlliedDefenseActors)
	{
		BindHealthActor(Ally, false);
	}
	if (VRHUD)
	{
		VRHUD->SetObjective(
			LOCTEXT("DefenseObjective", "총통을 운용해 성문을 방어하십시오"),
			LOCTEXT("DefenseObjectiveDetail", "장전 순서를 지키고 접근하는 적을 저지하십시오"));
		if (Cannon)
		{
			HandleLoadingStateChanged(Cannon->GetLoadingState(), Cannon->GetCompletedShots());
		}
	}

	if (WaveManager && WaveManager->HasWaveStarted())
	{
		HandleWaveStarted(WaveManager->GetTotalEnemiesToSpawn());
		HandleWaveProgress(WaveManager->GetDefeatedEnemyCount(), WaveManager->GetTotalEnemiesToSpawn());
	}
	return NarrationSequence != nullptr;
}

void UOngseongNarrationComponent::ReportScenarioEvent(const FName EventName, AActor* SourceActor)
{
	if (EventName.IsNone())
	{
		return;
	}
	OnScenarioEvent.Broadcast(EventName, SourceActor);
	if (const FOngseongNarrationEventBinding* Binding = FindEventBinding(EventName))
	{
		if (Binding->bPlayOnce && PlayedOnceEvents.Contains(EventName))
		{
			return;
		}
		if (Binding->bPlayOnce)
		{
			PlayedOnceEvents.Add(EventName);
		}
		QueueNarration(Binding->NarrationRow);
	}
}

const FOngseongNarrationEventBinding* UOngseongNarrationComponent::FindEventBinding(const FName EventName) const
{
	return EventBindings.FindByPredicate([EventName](const FOngseongNarrationEventBinding& Binding)
	{
		return Binding.EventName == EventName;
	});
}

void UOngseongNarrationComponent::QueueNarration(const FName RowName)
{
	if (!RowName.IsNone())
	{
		PendingRows.Add(RowName);
		TryPlayNextNarration();
	}
}

void UOngseongNarrationComponent::TryPlayNextNarration()
{
	if (!NarrationSequence)
	{
		InitializeNarrationBindings();
	}
	if (!NarrationSequence || bOwnsCurrentNarration || NarrationSequence->IsNarrationPlaying() || PendingRows.IsEmpty())
	{
		return;
	}

	UDataTable* Table = NarrationTable.LoadSynchronous();
	const FName Row = PendingRows[0];
	PendingRows.RemoveAt(0);
	bOwnsCurrentNarration = Table && NarrationSequence->PlaySequence(Table, Row);
	if (!bOwnsCurrentNarration)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not play Ongseong narration row %s."), *Row.ToString());
		TryPlayNextNarration();
	}
}

void UOngseongNarrationComponent::HandleNarrationFinished()
{
	if (bOwnsCurrentNarration)
	{
		bOwnsCurrentNarration = false;
		TryPlayNextNarration();
	}
}

void UOngseongNarrationComponent::HandleLoadingStateChanged(const EChongtongLoadingState NewState, const int32 CompletedShots)
{
	if (VRHUD)
	{
		const int32 RequiredShots = Cannon ? Cannon->GetRequiredShotsToComplete() : 5;
		switch (NewState)
		{
		case EChongtongLoadingState::NeedsPowder:
			VRHUD->ShowPrompt(FText::Format(LOCTEXT("LoadPowderPrompt", "화약을 넣으십시오 · 발사 {0}/{1}"),
				FText::AsNumber(CompletedShots), FText::AsNumber(RequiredShots)));
			break;
		case EChongtongLoadingState::NeedsRamming:
			VRHUD->ShowPrompt(LOCTEXT("RamPrompt", "쑤시개로 화약을 다지십시오"));
			break;
		case EChongtongLoadingState::NeedsCannonball:
			VRHUD->ShowPrompt(LOCTEXT("LoadBallPrompt", "대포알을 넣으십시오"));
			break;
		case EChongtongLoadingState::ReadyToAim:
			VRHUD->ShowPrompt(LOCTEXT("AimPrompt", "양손으로 손잡이를 잡고 방아쇠를 당기십시오"));
			break;
		case EChongtongLoadingState::Completed:
			VRHUD->ClearPrompt();
			VRHUD->ShowNotification(LOCTEXT("CannonComplete", "총통 운용을 완료했습니다"), EVRHUDNotificationType::Success, 4.0f);
			break;
		default:
			VRHUD->ClearPrompt();
			break;
		}
	}

	switch (NewState)
	{
	case EChongtongLoadingState::NeedsRamming:
		ReportScenarioEvent(OngseongNarrationEvents::PowderLoaded, Cannon);
		break;
	case EChongtongLoadingState::NeedsCannonball:
		ReportScenarioEvent(OngseongNarrationEvents::RammingCompleted, Cannon);
		break;
	case EChongtongLoadingState::ReadyToAim:
		ReportScenarioEvent(OngseongNarrationEvents::ReadyToAim, Cannon);
		break;
	case EChongtongLoadingState::NeedsPowder:
		if (CompletedShots > 0)
		{
			ReportScenarioEvent(OngseongNarrationEvents::ReloadRequired, Cannon);
		}
		break;
	default:
		break;
	}
}

void UOngseongNarrationComponent::HandleRammingProgress(const int32 CompletedRams, const int32 RequiredRams)
{
	if (VRHUD)
	{
		VRHUD->ShowPrompt(FText::Format(LOCTEXT("RammingProgress", "화약 다지기 {0}/{1}"),
			FText::AsNumber(CompletedRams), FText::AsNumber(RequiredRams)));
	}
}

void UOngseongNarrationComponent::HandleWaveStarted(const int32 TotalEnemies)
{
	if (VRHUD && TotalEnemies > 0)
	{
		VRHUD->SetProgress(LOCTEXT("DefenseProgress", "적 저지"), 0, TotalEnemies);
	}
	ReportScenarioEvent(OngseongNarrationEvents::WaveStarted, WaveManager);
}

void UOngseongNarrationComponent::HandleEnemySpawned(AEnemyCombatCharacter* Enemy, const int32 SpawnedEnemies, const int32 TotalEnemies)
{
	if (SpawnedEnemies == 1)
	{
		ReportScenarioEvent(OngseongNarrationEvents::EnemyAssault, Enemy);
	}
}

void UOngseongNarrationComponent::HandleWaveProgress(const int32 DefeatedEnemies, const int32 TotalEnemies)
{
	if (VRHUD)
	{
		VRHUD->SetProgress(LOCTEXT("DefenseProgress", "적 저지"), DefeatedEnemies, TotalEnemies);
	}
}

void UOngseongNarrationComponent::HandleAllEnemiesDefeated(const int32 TotalEnemies)
{
	if (VRHUD)
	{
		VRHUD->SetProgress(LOCTEXT("DefenseProgress", "적 저지"), TotalEnemies, TotalEnemies);
		VRHUD->ShowNotification(LOCTEXT("DefenseSucceeded", "성문 방어에 성공했습니다"), EVRHUDNotificationType::Success, 5.0f);
	}
	ReportScenarioEvent(OngseongNarrationEvents::DefenseSucceeded, WaveManager);
}

void UOngseongNarrationComponent::BindHealthActor(AActor* Actor, const bool bIsGate)
{
	if (!Actor)
	{
		return;
	}
	if (UHealthComponent* Health = Actor->FindComponentByClass<UHealthComponent>())
	{
		BoundHealthComponents.AddUnique(Health);
		if (bIsGate)
		{
			Health->OnDamaged.AddUniqueDynamic(this, &ThisClass::HandleGateDamaged);
			Health->OnDeath.AddUniqueDynamic(this, &ThisClass::HandleGateDestroyed);
		}
		else
		{
			Health->OnDamaged.AddUniqueDynamic(this, &ThisClass::HandleAllyDamaged);
		}
	}
}

void UOngseongNarrationComponent::HandleGateDamaged(UHealthComponent* HealthComponent, const FCombatDamageSpec& DamageSpec)
{
	if (VRHUD)
	{
		VRHUD->ShowNotification(LOCTEXT("GateUnderAttack", "성문이 공격받고 있습니다"), EVRHUDNotificationType::Warning, 3.0f);
	}
	ReportScenarioEvent(OngseongNarrationEvents::GateUnderAttack, HealthComponent ? HealthComponent->GetOwner() : nullptr);
}

void UOngseongNarrationComponent::HandleGateDestroyed(UHealthComponent* HealthComponent, const FCombatDamageSpec& KillingDamage)
{
	if (VRHUD)
	{
		VRHUD->ShowNotification(LOCTEXT("GateDestroyed", "성문이 파괴되었습니다"), EVRHUDNotificationType::Error, 0.0f);
	}
	ReportScenarioEvent(OngseongNarrationEvents::GateDestroyed, HealthComponent ? HealthComponent->GetOwner() : nullptr);
}

void UOngseongNarrationComponent::HandleAllyDamaged(UHealthComponent* HealthComponent, const FCombatDamageSpec& DamageSpec)
{
	ReportScenarioEvent(OngseongNarrationEvents::AlliesUnderAttack, HealthComponent ? HealthComponent->GetOwner() : nullptr);
}

void UOngseongNarrationComponent::UnbindSources()
{
	if (Cannon)
	{
		Cannon->OnLoadingStateChanged.RemoveDynamic(this, &ThisClass::HandleLoadingStateChanged);
		Cannon->OnRammingProgress.RemoveDynamic(this, &ThisClass::HandleRammingProgress);
	}
	if (WaveManager)
	{
		WaveManager->OnWaveStarted.RemoveDynamic(this, &ThisClass::HandleWaveStarted);
		WaveManager->OnEnemySpawned.RemoveDynamic(this, &ThisClass::HandleEnemySpawned);
		WaveManager->OnWaveProgress.RemoveDynamic(this, &ThisClass::HandleWaveProgress);
		WaveManager->OnAllEnemiesDefeated.RemoveDynamic(this, &ThisClass::HandleAllEnemiesDefeated);
	}
	for (UHealthComponent* Health : BoundHealthComponents)
	{
		if (Health)
		{
			Health->OnDamaged.RemoveDynamic(this, &ThisClass::HandleGateDamaged);
			Health->OnDamaged.RemoveDynamic(this, &ThisClass::HandleAllyDamaged);
			Health->OnDeath.RemoveDynamic(this, &ThisClass::HandleGateDestroyed);
		}
	}
	if (NarrationSequence)
	{
		NarrationSequence->OnSequenceFinished.RemoveDynamic(this, &ThisClass::HandleNarrationFinished);
	}
	if (VRHUD)
	{
		VRHUD->ClearAll();
	}
	BoundHealthComponents.Reset();
}

#undef LOCTEXT_NAMESPACE

void UOngseongNarrationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindSources();
	PendingRows.Reset();
	Super::EndPlay(EndPlayReason);
}
