#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "SingijeonAmmoSlotComponent.generated.h"

class USingijeonAmmoSlotComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSingijeonSlotChanged, USingijeonAmmoSlotComponent*, Slot, AActor*, Ammunition);

UCLASS(ClassGroup = (Singijeon), meta = (BlueprintSpawnableComponent))
class GF_SINGIJEON_API USingijeonAmmoSlotComponent : public UBoxComponent
{
    GENERATED_BODY()

public:
    USingijeonAmmoSlotComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Singijeon|Slot")
    bool TryLoadAmmunition(AActor* Candidate);

    UFUNCTION(BlueprintCallable, Category = "Singijeon|Slot")
    AActor* UnloadAmmunition();

    UFUNCTION(BlueprintCallable, Category = "Singijeon|Slot")
    bool LaunchLoadedAmmunition(FVector Direction, float Speed);

    UFUNCTION(BlueprintPure, Category = "Singijeon|Slot")
    bool IsLoaded() const { return IsValid(LoadedAmmunition); }

    UFUNCTION(BlueprintPure, Category = "Singijeon|Slot")
    AActor* GetLoadedAmmunition() const { return LoadedAmmunition; }

    UPROPERTY(BlueprintAssignable, Category = "Singijeon|Slot")
    FOnSingijeonSlotChanged OnAmmunitionLoaded;

    UPROPERTY(BlueprintAssignable, Category = "Singijeon|Slot")
    FOnSingijeonSlotChanged OnAmmunitionRemoved;

protected:
    UFUNCTION()
    void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void HandleAmmunitionDestroyed(AActor* DestroyedActor);

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Singijeon|Slot")
    TObjectPtr<AActor> LoadedAmmunition;

    /** Prevents collision changes made by PrepareForLoading from recursively loading this slot. */
    bool bLoadInProgress = false;

    /** Restores the snapped attachment if a delayed XR Grab release detaches the loaded Actor. */
    void EnforceLoadedAttachment();
};
