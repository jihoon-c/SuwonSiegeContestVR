#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_OngseongFireArrow.generated.h"

/** Behavior Tree leaf that fires once through the possessed archer's combat component. */
UCLASS()
class GF_ONGSEONGCROSSBOW_API UBTTask_OngseongFireArrow : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_OngseongFireArrow();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
