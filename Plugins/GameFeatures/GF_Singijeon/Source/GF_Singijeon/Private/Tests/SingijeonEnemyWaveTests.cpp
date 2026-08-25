#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Enemy/SingijeonEnemyWaveActor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/InstancedSkinnedMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Shared/Characters/EnemySoldierActor.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSingijeonEnemyWaveScaleAndVolleyTest,
    "SuwonSiegeContestVR.GF_Singijeon.EnemyWave.ScaleAndVolley",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSingijeonEnemyWaveEditorPreviewTest,
    "SuwonSiegeContestVR.GF_Singijeon.EnemyWave.EditorPreview",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingijeonEnemyWaveEditorPreviewTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Editor, false);
	if (!TestNotNull(TEXT("Editor preview world is created"), World))
	{
		return false;
	}
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Editor);
	WorldContext.SetCurrentWorld(World);

	ASingijeonEnemyWaveActor* Wave = World->SpawnActor<ASingijeonEnemyWaveActor>();
	TestNotNull(TEXT("Enemy Wave is spawned in the editor world"), Wave);
	TInlineComponentArray<USkeletalMeshComponent*> PreviewMeshes(Wave);
	TestEqual(TEXT("Editor preview displays all 45 authored enemy positions"),
		PreviewMeshes.Num(), 45);
	for (USkeletalMeshComponent* Preview : PreviewMeshes)
	{
		TestTrue(TEXT("Preview mesh is editor-only"),
			Preview && Preview->IsEditorOnly());
		TestTrue(TEXT("Preview mesh has viewport visibility enabled"),
			Preview && Preview->GetVisibleFlag() && !Preview->bHiddenInGame);
	}

	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}

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
            TestEqual(TEXT("Experimental GPU crowd remains disabled by default"),
                CharacterInstances->GetInstanceCount(), 0);

            TSet<int32> QuantizedLateralLocations;
            TArray<AEnemySoldierActor*> OwnedEnemyActors;
            for (TActorIterator<AEnemySoldierActor> It(World); It; ++It)
            {
                if (It->GetOwner() == Wave)
                {
                    OwnedEnemyActors.Add(*It);
                }
            }
            TestEqual(TEXT("Reliable path creates all 45 named enemy Actors"),
                OwnedEnemyActors.Num(), 45);
            for (AEnemySoldierActor* ProxyActor : OwnedEnemyActors)
            {
                USkeletalMeshComponent* Proxy = ProxyActor
                    ? ProxyActor->GetMesh()
                    : nullptr;
                if (Proxy)
                {
					TestTrue(TEXT("Reliable enemy proxy has its own Quest-safe Actor owner"),
						Proxy->GetOwner() == ProxyActor && ProxyActor->GetOwner() == Wave);
					TestTrue(TEXT("Reliable enemy proxy is registered with the runtime world"),
						Proxy->IsRegistered());
                    TestTrue(TEXT("Prepared reliable enemy proxy is visible"),
                        Proxy->IsVisible() && !Proxy->bHiddenInGame);
					TestTrue(TEXT("Prepared reliable enemy proxy renders in the main pass"),
						Proxy->ShouldRender());
                    QuantizedLateralLocations.Add(
                        FMath::RoundToInt(Proxy->GetComponentLocation().Y / 10.0f));
                    TestTrue(TEXT("Manny crowd remains at a valid human-scale range"),
                        Proxy->GetComponentScale().Z > 0.5f &&
                        Proxy->GetComponentScale().Z < 1.5f);
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
        Wave->PanicDuration = 0.25f;
        Wave->BeginPanic(Wave->GetActorLocation(), Wave->GetActorForwardVector());
        TestTrue(TEXT("Volley first enters the panic state"), Wave->IsPanicking());
        Wave->Tick(0.1f);
        TestEqual(TEXT("Enemies remain visible during the panic window"), Wave->GetAliveEnemyCount(), 45);
        Wave->Tick(0.2f);
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

