#include "Gameplay/AI/EnemyAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BrainComponent.h"

AEnemyAIController::AEnemyAIController()
{
	bStartAILogicOnPossess = false;
}

void AEnemyAIController::SetHighDetailAIEnabled(const bool bEnabled)
{
	if (bHighDetailAIEnabled == bEnabled)
	{
		return;
	}

	bHighDetailAIEnabled = bEnabled;
	if (bHighDetailAIEnabled && HighDetailBehaviorTree)
	{
		RunBehaviorTree(HighDetailBehaviorTree);
	}
	else if (!bHighDetailAIEnabled && BrainComponent)
	{
		BrainComponent->StopLogic(TEXT("Far AI LOD"));
	}

	OnHighDetailAIChanged.Broadcast(bHighDetailAIEnabled);
}

bool AEnemyAIController::HasHighDetailAIImplementation() const
{
	return HighDetailBehaviorTree != nullptr || OnHighDetailAIChanged.IsBound();
}
