#include "Ongseong/OngseongGateActor.h"

#include "Components/StaticMeshComponent.h"
#include "Gameplay/Combat/CombatFactionComponent.h"
#include "Gameplay/Combat/HealthComponent.h"

AOngseongGateActor::AOngseongGateActor()
{
	PrimaryActorTick.bCanEverTick = false;
	GateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GateMesh"));
	SetRootComponent(GateMesh);
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	FactionComponent = CreateDefaultSubobject<UCombatFactionComponent>(TEXT("FactionComponent"));
}

void AOngseongGateActor::BeginPlay()
{
	Super::BeginPlay();
	FactionComponent->SetFaction(ECombatFaction::Ally);
	HealthComponent->OnHealthChanged.AddUniqueDynamic(this, &AOngseongGateActor::HandleHealthChanged);
	HealthComponent->OnDeath.AddUniqueDynamic(this, &AOngseongGateActor::HandleDeath);
	HandleHealthChanged(HealthComponent, HealthComponent->GetCurrentHealth(), HealthComponent->GetCurrentHealth(), 0.0f);
}

bool AOngseongGateActor::ReceiveCombatDamage_Implementation(const FCombatDamageSpec& DamageSpec)
{
	return HealthComponent && HealthComponent->ApplyDamage(DamageSpec);
}

void AOngseongGateActor::ResetGate()
{
	if (HealthComponent) HealthComponent->ResetHealth();
}

void AOngseongGateActor::HandleHealthChanged(UHealthComponent* ChangedHealth, float PreviousHealth, float NewHealth, float AppliedDelta)
{
	if (!ChangedHealth) return;
	const float Ratio = ChangedHealth->GetMaxHealth() > 0.0f ? NewHealth / ChangedHealth->GetMaxHealth() : 0.0f;
	OnGateHealthRatioChanged.Broadcast(FMath::Clamp(Ratio, 0.0f, 1.0f));
}

void AOngseongGateActor::HandleDeath(UHealthComponent* DeadHealth, const FCombatDamageSpec& KillingDamage)
{
	OnGateDestroyed.Broadcast();
}
