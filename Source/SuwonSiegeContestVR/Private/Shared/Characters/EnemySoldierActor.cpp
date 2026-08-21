#include "Shared/Characters/EnemySoldierActor.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Shared/Combat/FactionComponent.h"
#include "Shared/Combat/LegacyHealthComponent.h"
#include "UObject/ConstructorHelpers.h"

AEnemySoldierActor::AEnemySoldierActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Faction = CreateDefaultSubobject<UFactionComponent>(TEXT("Faction"));
	Faction->Faction = ELegacyCombatFaction::Enemy;
	Health = CreateDefaultSubobject<ULegacyHealthComponent>(TEXT("Health"));

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 92.0f);
	GetCharacterMovement()->DefaultLandMovementMode = MOVE_None;
	GetCharacterMovement()->GravityScale = 0.0f;

	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -92.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SoldierMeshFinder(
		TEXT("/Game/NiagaraExamples/Gallery/SkeletalMesh/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	if (SoldierMeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMeshAsset(SoldierMeshFinder.Object);
	}

}

void AEnemySoldierActor::SetSoldierActive(const bool bActive)
{
	SetActorHiddenInGame(!bActive);
	SetActorEnableCollision(bActive);
	GetMesh()->SetVisibility(bActive, true);
	if (bActive && Health)
	{
		Health->ResetHealth();
	}
}
