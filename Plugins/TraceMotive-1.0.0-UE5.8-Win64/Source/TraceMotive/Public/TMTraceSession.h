#pragma once

#include "CoreMinimal.h"

// Shared lifecycle state for time-sliced editor traces.
class FTMTraceSession
{
public:
    void Begin()
    {
        bActive = true;
        ++Generation;
        LastUiRefreshSeconds = 0.0;
    }

    void Cancel()
    {
        bActive = false;
        ++Generation;
    }

    void Complete()
    {
        bActive = false;
    }

    bool IsActive() const
    {
        return bActive;
    }

    uint64 GetGeneration() const
    {
        return Generation;
    }

    bool ShouldRefreshUi(const double NowSeconds, const double IntervalSeconds)
    {
        if (NowSeconds - LastUiRefreshSeconds < IntervalSeconds)
        {
            return false;
        }

        LastUiRefreshSeconds = NowSeconds;
        return true;
    }

private:
    bool bActive = false;
    uint64 Generation = 0;
    double LastUiRefreshSeconds = 0.0;
};
