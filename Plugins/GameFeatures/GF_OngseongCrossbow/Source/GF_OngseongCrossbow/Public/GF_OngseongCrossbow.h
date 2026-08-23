#pragma once

#include "Modules/ModuleManager.h"

/** Scenario-level progress logging for the ongseong defense. Keep it sparse enough for shipping logs. */
GF_ONGSEONGCROSSBOW_API DECLARE_LOG_CATEGORY_EXTERN(LogOngseong, Log, All);

class FGF_OngseongCrossbowModule : public IModuleInterface
{
};
