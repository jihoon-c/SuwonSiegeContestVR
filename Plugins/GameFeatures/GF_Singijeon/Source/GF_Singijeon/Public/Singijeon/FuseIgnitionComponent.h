#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "FuseIgnitionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFuseIgnitionSourceEvent, AActor*, SourceActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFuseIgnited);

UCLASS(ClassGroup = (Singijeon), meta = (BlueprintSpawnableComponent))
class GF_SINGIJEON_API UFuseIgnitionComponent : public USphereComponent
{
    GENERATED_BODY()

public:
    UFuseIgnitionComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Singijeon|Fuse")
    bool TryBeginIgnition(AActor* SourceActor);

    UFUNCTION(BlueprintCallable, Category = "Singijeon|Fuse")
    void CancelIgnition();

    UFUNCTION(BlueprintCallable, Category = "Singijeon|Fuse")
    void SetIgnitionEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "Singijeon|Fuse")
    float GetIgnitionProgress() const;

    UPROPERTY(BlueprintAssignable, Category = "Singijeon|Fuse")
    FOnFuseIgnitionSourceEvent OnIgnitionStarted;

    UPROPERTY(BlueprintAssignable, Category = "Singijeon|Fuse")
    FOnFuseIgnitionSourceEvent OnIgnitionCanceled;

    UPROPERTY(BlueprintAssignable, Category = "Singijeon|Fuse")
    FOnFuseIgnited OnIgnited;

protected:
    UFUNCTION()
    void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

    void CompleteIgnition();
    bool IsValidActiveSource(AActor* Candidate) const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Singijeon|Fuse", meta = (ClampMin = "0.0"))
    float IgnitionDuration = 0.75f;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Singijeon|Fuse")
    TObjectPtr<AActor> ActiveSource;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Singijeon|Fuse")
    bool bIgnitionEnabled = false;

    float IgnitionStartTime = 0.0f;
};
