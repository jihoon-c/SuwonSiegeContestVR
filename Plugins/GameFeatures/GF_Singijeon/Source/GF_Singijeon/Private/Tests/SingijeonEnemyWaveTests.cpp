#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Enemy/SingijeonEnemyWaveActor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/InstancedSkinnedMeshComponent.h"
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
        TestEqual(TEXT("Only three foreground enemies use full Actors"), Wave->GetInteractiveEnemyCount(), 3);
        TestEqual(TEXT("Remaining 42 enemies use GPU-skinned characters"), Wave->GetProxyEnemyCount(), 42);
        UInstancedSkinnedMeshComponent* CharacterInstances =
            Wave->FindComponentByClass<UInstancedSkinnedMeshComponent>();
        TestNotNull(TEXT("Wave owns one instanced skeletal character renderer"), CharacterInstances);
        if (CharacterInstances)
        {
            TestEqual(TEXT("All background enemies have full character instances"),
                CharacterInstances->GetInstanceCount(), 42);

            TSet<int32> QuantizedLateralLocations;
            for (int32 InstanceIndex = 0; InstanceIndex < CharacterInstances->GetInstanceCount(); ++InstanceIndex)
            {
                FTransform InstanceTransform;
                if (CharacterInstances->GetInstanceTransform(
                    CharacterInstances->GetInstanceId(InstanceIndex), InstanceTransform, true))
                {
                    QuantizedLateralLocations.Add(FMath::RoundToInt(InstanceTransform.GetLocation().Y / 10.0f));
                }
            }
            TestTrue(TEXT("Seeded formation jitter breaks the five-column grid"),
                QuantizedLateralLocations.Num() > 12);
        }
        UHierarchicalInstancedStaticMeshComponent* LegacyTargetInstances =
            Wave->FindComponentByClass<UHierarchicalInstancedStaticMeshComponent>();
        TestTrue(TEXT("Legacy circular target proxy has no mesh or instances"),
            LegacyTargetInstances && LegacyTargetInstances->GetStaticMesh() == nullptr &&
            LegacyTargetInstances->GetInstanceCount() == 0);
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
        Wave->SetProcedureApproachPhase(ESingijeonEnemyApproachPhase::Loaded);
        Wave->StartWave();
        Wave->Tick(1.0f);
        const float LoadedGateDistance = Wave->GetWaveDistance();
        TestTrue(TEXT("Loaded procedure caps the charge at 35 percent"),
            FMath::IsNearlyEqual(LoadedGateDistance, Wave->GetRouteLength() * 0.35f, 1.0f));
        Wave->Tick(1.0f);
        TestTrue(TEXT("Enemies wait at the loaded gate instead of reaching the target"),
            FMath::IsNearlyEqual(Wave->GetWaveDistance(), LoadedGateDistance, 1.0f));
        TestEqual(TEXT("Waiting at a procedure gate keeps the Wave charging"),
            Wave->GetWaveState(), ESingijeonEnemyWaveState::Charging);

        Wave->SetProcedureApproachPhase(ESingijeonEnemyApproachPhase::Aimed);
        Wave->Tick(1.0f);
        TestTrue(TEXT("Aiming opens the route to 60 percent"),
            FMath::IsNearlyEqual(Wave->GetWaveDistance(), Wave->GetRouteLength() * 0.60f, 1.0f));
        Wave->SetProcedureApproachPhase(ESingijeonEnemyApproachPhase::Igniting);
        Wave->Tick(1.0f);
        TestTrue(TEXT("Fuse ignition opens the route to 82 percent"),
            FMath::IsNearlyEqual(Wave->GetWaveDistance(), Wave->GetRouteLength() * 0.82f, 1.0f));
        Wave->SetProcedureApproachPhase(ESingijeonEnemyApproachPhase::Firing);
        Wave->Tick(1.0f);
        TestTrue(TEXT("Volley in progress holds enemies at 95 percent"),
            FMath::IsNearlyEqual(Wave->GetWaveDistance(), Wave->GetRouteLength() * 0.95f, 1.0f));
        TestEqual(TEXT("Enemies cannot reach their final destinations before volley completion"),
            Wave->GetWaveState(), ESingijeonEnemyWaveState::Charging);

        Wave->SetProcedureApproachPhase(ESingijeonEnemyApproachPhase::Unrestricted);
        Wave->Tick(1.0f);
        TestEqual(TEXT("Survivors can reach their fixed destinations after volley completion"),
            Wave->GetWaveState(), ESingijeonEnemyWaveState::ReachedTarget);
    }

    World->DestroyWorld(false);
    GEngine->DestroyWorldContext(World);
    return true;
}

#endif

