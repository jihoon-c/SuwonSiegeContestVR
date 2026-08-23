#include "Ongseong/BTTask_OngseongFireArrow.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Ongseong/OngseongArcherCombatComponent.h"

UBTTask_OngseongFireArrow::UBTTask_OngseongFireArrow()
{
	NodeName = TEXT("Fire Ongseong Arrow");
}

EBTNodeResult::Type UBTTask_OngseongFireArrow::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* Controller = OwnerComp.GetAIOwner();
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	UOngseongArcherCombatComponent* ArcherCombat = Pawn ? Pawn->FindComponentByClass<UOngseongArcherCombatComponent>() : nullptr;
	return ArcherCombat && ArcherCombat->TryFireArrow() ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}
