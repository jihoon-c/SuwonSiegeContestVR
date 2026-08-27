#pragma once

#include "Components/WidgetComponent.h"
#include "MainEducationWidgetComponent.generated.h"

/** World widget component that supports both desktop mouse testing and VR WidgetInteraction. */
UCLASS(ClassGroup = (UI))
class SUWONSIEGECONTESTVR_API UMainEducationWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UMainEducationWidgetComponent()
	{
		bReceiveHardwareInput = true;
	}
};
