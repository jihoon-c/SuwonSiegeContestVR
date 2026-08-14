#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SingijeonHwachaActor.generated.h"

class UFuseIgnitionComponent;
class USceneComponent;
class UScenarioInteractableComponent;
class USingijeonAmmoSlotComponent;
class UStaticMeshComponent;
class UTwoHandCarryComponent;

UENUM(BlueprintType)
enum class ESingijeonHwachaState : uint8
{
    Empty,
    Loaded,
    Igniting,
    Fired
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHwachaStateChanged, ESingijeonHwachaState, OldState, ESingijeonHwachaState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHwachaLoadCountChanged, int32, LoadedCount, int32, TotalSlots);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHwachaVolleyLaunched);

UCLASS(Blueprintable)
class GF_SINGIJEON_API ASingijeonHwachaActor : public AActor
{
    GENERATED_BODY()

public:
    ASingijeonHwachaActor();

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintPure, Category = "Singijeon|Hwacha")
    int32 GetLoadedAmmunitionCount() const;

    UFUNCTION(BlueprintPure, Category = "Singijeon|Hwacha")
    bool IsReadyToIgnite() const;

    UFUNCTION(BlueprintCallable, Category = "Singijeon|Hwacha")
    void LaunchVolley();

    UFUNCTION(BlueprintCallable, Category = "Singijeon|Hwacha")
    void ResetHwacha();

    UFUNCTION(BlueprintPure, Category = "Singijeon|Hwacha")
    ESingijeonHwachaState GetHwachaState() const { return HwachaState; }

    UPROPERTY(BlueprintAssignable, Category = "Singijeon|Hwacha")
    FOnHwachaStateChanged OnHwachaStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Singijeon|Hwacha")
    FOnHwachaLoadCountChanged OnLoadCountChanged;

    UPROPERTY(BlueprintAssignable, Category = "Singijeon|Hwacha")
    FOnHwachaVolleyLaunched OnVolleyLaunched;

protected:
    UFUNCTION()
    void HandleSlotChanged(USingijeonAmmoSlotComponent* Slot, AActor* Ammunition);

    UFUNCTION()
    void HandleIgnitionStarted(AActor* SourceActor);

    UFUNCTION()
    void HandleIgnitionCanceled(AActor* SourceActor);

    UFUNCTION()
    void HandleFuseIgnited();

    void RefreshLoadState();
    void SetHwachaState(ESingijeonHwachaState NewState);
    void LaunchNextAmmunition();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> BodyMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> RackRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USingijeonAmmoSlotComponent> DefaultAmmoSlot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UFuseIgnitionComponent> Fuse;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UTwoHandCarryComponent> TwoHandCarry;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UScenarioInteractableComponent> LoadScenarioInteractor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UScenarioInteractableComponent> IgniteScenarioInteractor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UScenarioInteractableComponent> FireScenarioInteractor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Launch", meta = (ClampMin = "1"))
    int32 MinimumLoadedAmmunition = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Launch", meta = (ClampMin = "1.0"))
    float LaunchSpeed = 3500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Launch", meta = (ClampMin = "0.0"))
    float LaunchInterval = 0.08f;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Singijeon|Hwacha")
    ESingijeonHwachaState HwachaState = ESingijeonHwachaState::Empty;

    UPROPERTY(Transient)
    TArray<TObjectPtr<USingijeonAmmoSlotComponent>> AmmoSlots;

    UPROPERTY(Transient)
    TArray<TObjectPtr<USingijeonAmmoSlotComponent>> PendingLaunchSlots;

    int32 NextLaunchIndex = 0;
    FTimerHandle LaunchTimerHandle;
};
