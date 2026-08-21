#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Enemy/SingijeonEnemyWaveActor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSingijeonEnemyWaveScaleAndVolleyTest,
    "SuwonSiegeContestVR.GF_Singijeon.EnemyWave.ScaleAndVolley",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingijeonEnemyWaveScaleAndVolleyTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    if (!TestNotNull(TEXT("Test world is created"), World))
    {
        return false;
    }

    FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
    WorldContext.SetCurrentWorld(World);
    World->InitializeActorsForPlay(FURL());

    ASingijeonEnemyWaveActor* Wave = World->SpawnActor<ASingijeonEnemyWaveActor>();
    TestNotNull(TEXT("Enemy Wave is spawned"), Wave);
    if (Wave)
    {
        TestTrue(TEXT("Wave prepares without authored splines or a NavMesh"), Wave->PrepareWave());
        TestEqual(TEXT("Default logical force contains 45 enemies"), Wave->GetLogicalEnemyCount(), 45);
        TestEqual(TEXT("Only ten foreground enemies use full Actors"), Wave->GetInteractiveEnemyCount(), 10);
        TestEqual(TEXT("Remaining 35 enemies use HISM proxies"), Wave->GetProxyEnemyCount(), 35);
        TestTrue(TEXT("Automatic fallback route has usable length"), Wave->GetRouteLength() > 9000.0f);
        TestEqual(TEXT("Prepared Wave is ready"), Wave->GetWaveState(), ESingijeonEnemyWaveState::Ready);

        Wave->StartWave();
        TestEqual(TEXT("StartWave begins the charge"), Wave->GetWaveState(), ESingijeonEnemyWaveState::Charging);
        TestTrue(TEXT("One 90-arrow volley resolves the full default force"),
            Wave->ResolveVolley(Wave->GetActorLocation(), Wave->GetActorForwardVector()) == 45);
        TestEqual(TEXT("No logical enemies remain"), Wave->GetAliveEnemyCount(), 0);
        TestEqual(TEXT("Wave enters Defeated state"), Wave->GetWaveState(), ESingijeonEnemyWaveState::Defeated);

        Wave->ResetWave();
        Wave->ChargeSpeed = 20000.0f;
        Wave->StartWave();
        Wave->Tick(1.0f);
        TestEqual(TEXT("A surviving front rank reaching the target stops the Wave"),
            Wave->GetWaveState(), ESingijeonEnemyWaveState::ReachedTarget);
    }

    World->DestroyWorld(false);
    GEngine->DestroyWorldContext(World);
    return true;
}

#endif

