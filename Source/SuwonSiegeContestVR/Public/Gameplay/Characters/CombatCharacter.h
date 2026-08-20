#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Gameplay/Combat/DamageReceiverInterface.h"
#include "CombatCharacter.generated.h"

class UCombatFactionComponent;
class UHealthComponent;

/** Shared non-player combat pawn base. Feature content supplies meshes, animation, AI, and presentation. */
UCLASS(Abstract, Blueprintable)
class SUWONSIEGECONTESTVR_API ACombatCharacter : public ACharacter, public IDamageReceiverInterface
{
	GENERATED_BODY()

public:
	ACombatCharacter();

	virtual bool ReceiveCombatDamage_Implementation(const FCombatDamageSpec& DamageSpec) override;

	UFUNCTION(BlueprintPure, Category = "Combat")
	UHealthComponent* GetHealthComponent() const { return HealthComponent; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	UCombatFactionComponent* GetFactionComponent() const { return FactionComponent; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCombatFactionComponent> FactionComponent;
};
