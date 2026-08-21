#include "TMTraceSession.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "ToolMenu.h"
#include "ToolMenus.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTMTraceSessionLifecycleTest,
    "TraceMotive.TraceSession.Lifecycle",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTMTraceSessionLifecycleTest::RunTest(const FString&)
{
    FTMTraceSession Session;
    TestFalse(TEXT("A new session is inactive"), Session.IsActive());

    Session.Begin();
    const uint64 FirstGeneration = Session.GetGeneration();
    TestTrue(TEXT("Begin activates the session"), Session.IsActive());
    TestTrue(TEXT("First generation is non-zero"), FirstGeneration > 0);

    TestTrue(TEXT("First UI refresh is allowed"), Session.ShouldRefreshUi(10.0, 0.2));
    TestFalse(TEXT("Refresh is throttled inside the interval"), Session.ShouldRefreshUi(10.1, 0.2));
    TestTrue(TEXT("Refresh is allowed after the interval"), Session.ShouldRefreshUi(10.2, 0.2));

    Session.Cancel();
    TestFalse(TEXT("Cancel deactivates the session"), Session.IsActive());
    TestTrue(TEXT("Cancel invalidates queued work generation"), Session.GetGeneration() > FirstGeneration);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTMWindowMenuRegistrationTest,
    "TraceMotive.Registration.WindowMenuEntries",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTMWindowMenuRegistrationTest::RunTest(const FString&)
{
    UToolMenu* WindowMenu = UToolMenus::Get()->FindMenu(TEXT("LevelEditor.MainMenu.Window"));
    TestNotNull(TEXT("The Level Editor Window menu exists"), WindowMenu);
    if (!WindowMenu)
    {
        return false;
    }

    const TArray<FName> RequiredEntries =
    {
        TEXT("TMOpenAudioPlaybackTrace"),
        TEXT("TMOpenBlueprintRuntimeErrorTrace"),
        TEXT("TMOpenClickDiagnostics"),
        TEXT("TMOpenCollisionPairAnalyzer"),
        TEXT("TMOpenEnhancedOutlinerSearch"),
        TEXT("TMOpenGlobalSpeedControl"),
        TEXT("TMOpenPackageProgress"),
        TEXT("TMOpenToolLauncher"),
        TEXT("TMOpenWidgetLifecycleTrace"),
        TEXT("TMOpenWidgetClickFlowTrace"),
        TEXT("TMOpenVariableValueTrace")
    };

    for (const FName EntryName : RequiredEntries)
    {
        TestTrue(*FString::Printf(TEXT("TraceMotive menu entry %s is registered"), *EntryName.ToString()), WindowMenu->ContainsEntry(EntryName));
    }

    return true;
}

#endif
