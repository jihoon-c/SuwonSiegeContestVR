#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemySoldierActor.generated.h"

class UFactionComponent;
class UHealthComponent;

/** Reusable enemy soldier shell. Feature groups own its authored movement. */
UCLASS(Blueprintable)
class SUWONSIEGECONTESTVR_API AEnemySoldierActor : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemySoldierActor();

	UFUNCTION(BlueprintPure, Category = "Combat")
	UFactionComponent* GetFactionComponent() const { return Faction; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	UHealthComponent* GetHealthComponent() const { return Health; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetSoldierActive(bool bActive);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UFactionComponent> Faction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UHealthComponent> Health;
};
