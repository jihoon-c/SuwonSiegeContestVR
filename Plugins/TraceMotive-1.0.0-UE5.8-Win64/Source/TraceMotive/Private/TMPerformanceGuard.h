#pragma once



#include "CoreMinimal.h"

#include "HAL/PlatformTime.h"



namespace TMPerf

{

    inline bool& SearchBoostStorage()

    {

        static bool bSearchBoostEnabled = false;

        return bSearchBoostEnabled;

    }



    inline bool IsSearchBoostEnabled()

    {

        return SearchBoostStorage();

    }



    inline void SetSearchBoostEnabled(bool bEnabled)

    {

        SearchBoostStorage() = bEnabled;

    }



    inline void ToggleSearchBoost()

    {

        SearchBoostStorage() = !SearchBoostStorage();

    }



    inline double BoostMultiplier()

    {

        return IsSearchBoostEnabled() ? 3.0 : 1.0;

    }



    inline int32 BoostInt(int32 BaseValue, int32 MaxValue)

    {

        return IsSearchBoostEnabled() ? FMath::Min(MaxValue, FMath::Max(BaseValue, BaseValue * 3)) : BaseValue;

    }



    // Background-friendly defaults. Exhaustive scans should keep the editor responsive

    // even if that means large scans take longer to finish. Fast search can spend a bit

    // more budget because it uses a narrower candidate set.

    inline double VisualSearchTickBudgetSeconds(bool bFastSearch)

    {

        return (bFastSearch ? 0.0040 : 0.0015) * BoostMultiplier();

    }



    inline double CallChainTickBudgetSeconds()

    {

        return 0.0020 * BoostMultiplier();

    }



    inline double OutlinerSearchTickBudgetSeconds()

    {

        return 0.0015 * BoostMultiplier();

    }



    inline double AssetUsageTickBudgetSeconds()

    {

        return 0.0015 * BoostMultiplier();

    }



    inline double VisualSearchTickerIntervalSeconds(bool bFastSearch)

    {

        return IsSearchBoostEnabled() ? (bFastSearch ? 0.010 : 0.025) : (bFastSearch ? 0.025 : 0.06);

    }



    inline double CallChainTickerIntervalSeconds()

    {

        return IsSearchBoostEnabled() ? 0.025 : 0.08;

    }



    inline double OutlinerSearchTickerIntervalSeconds()

    {

        return IsSearchBoostEnabled() ? 0.015 : 0.05;

    }



    inline double AssetUsageTickerIntervalSeconds()

    {

        return IsSearchBoostEnabled() ? 0.020 : 0.06;

    }



    inline int32 MaxConcurrentAsyncBlueprintLoads(bool bFastSearch)

    {

        return IsSearchBoostEnabled() ? (bFastSearch ? 8 : 4) : (bFastSearch ? 3 : 1);

    }



    inline int32 VisualSearchBatchSize(bool bFastSearch)

    {

        return IsSearchBoostEnabled() ? (bFastSearch ? 12 : 4) : (bFastSearch ? 4 : 1);

    }



    inline int32 CallChainPackagesPerTick()

    {

        return BoostInt(1, 4);

    }



    inline int32 CallChainPathsPerTick()

    {

        return BoostInt(2, 8);

    }



    inline int32 OutlinerActorsPerTick()

    {

        return BoostInt(4, 16);

    }



    inline int32 MaxGraphNodesPerBlueprint()

    {

        return IsSearchBoostEnabled() ? 36000 : 12000;

    }



    inline int32 MaxMacroGraphNodesPerMatch()

    {

        return IsSearchBoostEnabled() ? 4096 : 1024;

    }



    inline int32 MaxCallerCacheBlueprintsPerBuild()

    {

        return IsSearchBoostEnabled() ? 7500 : 2500;

    }



    class FTickBudget

    {

    public:

        explicit FTickBudget(double InBudgetSeconds)

            : StartSeconds(FPlatformTime::Seconds())

            , BudgetSeconds(InBudgetSeconds)

        {

        }



        bool HasTime() const

        {

            return FPlatformTime::Seconds() - StartSeconds < BudgetSeconds;

        }



        bool ShouldYield() const

        {

            return !HasTime();

        }



        double GetElapsedSeconds() const

        {

            return FPlatformTime::Seconds() - StartSeconds;

        }



        double GetStartSeconds() const

        {

            return StartSeconds;

        }



    private:

        double StartSeconds = 0.0;

        double BudgetSeconds = 0.0;

    };

}

