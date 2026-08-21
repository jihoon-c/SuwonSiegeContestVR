#include "Gameplay/AI/EnemyBehaviorStateComponent.h"

UEnemyBehaviorStateComponent::UEnemyBehaviorStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEnemyBehaviorStateComponent::SetBehaviorState(const FName NewState)
{
	if (NewState.IsNone() || BehaviorState == NewState)
	{
		return;
	}

	const FName PreviousState = BehaviorState;
	BehaviorState = NewState;
	OnBehaviorStateChanged.Broadcast(PreviousState, BehaviorState);
}
