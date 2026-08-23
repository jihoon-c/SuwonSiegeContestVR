#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/Scenario/ScenarioDefinition.h"
#include "Core/Scenario/ScenarioManagerActor.h"
#include "Core/Scenario/ScenarioManagerComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "Interaction/FirePitActor.h"
#include "Interaction/IgnitionSourceActor.h"
#include "Interaction/IgnitionSourceInterface.h"
#include "Interaction/TwoHandCarryComponent.h"
#include "Singijeon/FuseIgnitionComponent.h"
#include "Singijeon/SingijeonAmmoSlotComponent.h"
#include "Singijeon/SingijeonAmmunitionInterface.h"
#include "Singijeon/SingijeonHwachaActor.h"
#include "Singijeon/SingijeonProjectileActor.h"
#include "Shared/Characters/EnemySoldierActor.h"
#include "Shared/Combat/LegacyHealthComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSingijeonHwachaAutoFillGridConfigurationTest,
    "SuwonSiegeContestVR.GF_Singijeon.Hwacha.AutoFillGridConfiguration",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSingijeonFirePitTorchIgnitionFlowTest,
    "SuwonSiegeContestVR.GF_Singijeon.Ignition.FirePitTorchFuseOrder",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSingijeonStrictScenarioOrderGateTest,
    "SuwonSiegeContestVR.GF_Singijeon.Interaction.StrictScenarioOrderGate",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSingijeonProjectileDamageTest,
    "SuwonSiegeContestVR.GF_Singijeon.Projectile.StandardDamage",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingijeonProjectileDamageTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    if (!TestNotNull(TEXT("Projectile test world is created"), World))
    {
        return false;
    }
    FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
    WorldContext.SetCurrentWorld(World);
    World->InitializeActorsForPlay(FURL());

    ASingijeonProjectileActor* Arrow = World->SpawnActor<ASingijeonProjectileActor>();
    AEnemySoldierActor* Enemy = World->SpawnActor<AEnemySoldierActor>();
    TestNotNull(TEXT("Launched projectile is spawned"), Arrow);
    TestNotNull(TEXT("Damage target is spawned"), Enemy);
    if (Arrow && Enemy && Enemy->GetHealthComponent())
    {
        Enemy->SetSoldierActive(true);
        ISingijeonAmmunitionInterface::Execute_OnLaunched(
            Arrow, FVector::ForwardVector, 1000.0f);
        TestTrue(TEXT("Launched projectile applies standard Health damage once"),
            Arrow->ApplyImpactDamage(Enemy));
        TestFalse(TEXT("A projectile cannot damage the same target twice"),
            Arrow->ApplyImpactDamage(Enemy));
    }

    World->DestroyWorld(false);
    GEngine->DestroyWorldContext(World);
    return true;
}

bool FSingijeonStrictScenarioOrderGateTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    if (!TestNotNull(TEXT("Test world is created"), World))
    {
        return false;
    }

    FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
    WorldContext.SetCurrentWorld(World);
    World->InitializeActorsForPlay(FURL());

    AScenarioManagerActor* ManagerActor = World->SpawnActor<AScenarioManagerActor>();
    UScenarioManagerComponent* Manager = ManagerActor ? ManagerActor->GetScenarioManager() : nullptr;
    ASingijeonHwachaActor* Hwacha = World->SpawnActor<ASingijeonHwachaActor>();
    ASingijeonProjectileActor* Arrow = World->SpawnActor<ASingijeonProjectileActor>();
    AIgnitionSourceActor* Torch = World->SpawnActor<AIgnitionSourceActor>();
    AFirePitActor* FirePit = World->SpawnActor<AFirePitActor>();
    USingijeonAmmoSlotComponent* Slot = Hwacha
        ? Hwacha->FindComponentByClass<USingijeonAmmoSlotComponent>() : nullptr;
    UFuseIgnitionComponent* Fuse = Hwacha
        ? Hwacha->FindComponentByClass<UFuseIgnitionComponent>() : nullptr;

    FScenarioStageDefinition Stage;
    Stage.StageID = TEXT("STAGE_StrictOrder");
    Stage.StartInteractionID = TEXT("INT_Blocker");
    const auto AddInteraction = [&Stage](const FName ID, const EScenarioInteractionType Type,
        const FName Target, const FName Next)
    {
        FScenarioInteraction Interaction;
        Interaction.InteractionID = ID;
        Interaction.InteractionType = Type;
        Interaction.TargetID = Target;
        Interaction.NextInteractionID = Next;
        Stage.Interactions.Add(Interaction);
    };
    AddInteraction(TEXT("INT_Blocker"), EScenarioInteractionType::Custom,
        TEXT("Order_Blocker"), TEXT("INT_Load"));
    AddInteraction(TEXT("INT_Load"), EScenarioInteractionType::Custom,
        TEXT("Hwacha_Load"), TEXT("INT_Torch"));
    AddInteraction(TEXT("INT_Torch"), EScenarioInteractionType::Trigger,
        TEXT("Torch_Ignite"), TEXT("INT_Fuse"));
    AddInteraction(TEXT("INT_Fuse"), EScenarioInteractionType::Trigger,
        TEXT("Hwacha_Fuse"), TEXT("INT_AfterFuse"));
    AddInteraction(TEXT("INT_AfterFuse"), EScenarioInteractionType::Custom,
        TEXT("Narration_AfterFuse"), TEXT("INT_Fire"));
    AddInteraction(TEXT("INT_Fire"), EScenarioInteractionType::Combat,
        TEXT("Hwacha_Fire"), NAME_None);

    UScenarioDefinition* Scenario = NewObject<UScenarioDefinition>();
    Scenario->ScenarioID = TEXT("SCENARIO_StrictOrder");
    Scenario->StartStageID = Stage.StageID;
    Scenario->Stages.Add(Stage);

    TestNotNull(TEXT("Scenario Manager exists"), Manager);
    TestNotNull(TEXT("Hwacha slot exists"), Slot);
    TestNotNull(TEXT("Hwacha Fuse exists"), Fuse);
    if (Manager && Hwacha && Arrow && Torch && FirePit && Slot && Fuse)
    {
        if (!Hwacha->HasActorBegunPlay())
        {
            Hwacha->DispatchBeginPlay();
        }
        TestTrue(TEXT("Strict-order Scenario starts"), Manager->StartScenario(Scenario));
        TestFalse(TEXT("Arrow cannot load before Hwacha_Load is current"),
            Slot->TryLoadAmmunition(Arrow));
        TestFalse(TEXT("Rejected early load leaves the slot empty"), Slot->IsLoaded());
        TestFalse(TEXT("Torch cannot ignite before Torch_Ignite is current"),
            FirePit->TryIgniteTorch(Torch));
        TestFalse(TEXT("Rejected early Fire Pit contact leaves the torch unlit"),
            IIgnitionSourceInterface::Execute_IsIgnitionActive(Torch));

        TestTrue(TEXT("Blocker completes"), Manager->ReportInteractionResult(
            TEXT("Order_Blocker"), EScenarioInteractionType::Custom));
        TestTrue(TEXT("Arrow loads when Hwacha_Load becomes current"),
            Slot->TryLoadAmmunition(Arrow));
        TestEqual(TEXT("Successful load advances to Torch_Ignite"),
            Manager->GetDebugSnapshot().InteractionID, FName(TEXT("INT_Torch")));

        Torch->SetIgnitionActive(true);
        TestFalse(TEXT("Lit torch cannot start the Fuse before Hwacha_Fuse is current"),
            Fuse->TryBeginIgnition(Torch));
        Torch->SetIgnitionActive(false);
        TestTrue(TEXT("Fire Pit ignites the torch when Torch_Ignite is current"),
            FirePit->TryIgniteTorch(Torch));
        TestEqual(TEXT("Successful Fire Pit ignition advances to Hwacha_Fuse"),
            Manager->GetDebugSnapshot().InteractionID, FName(TEXT("INT_Fuse")));
        TestTrue(TEXT("Lit torch starts the Fuse when Hwacha_Fuse is current"),
            Fuse->TryBeginIgnition(Torch));
        UFunction* FuseIgnitedFunction = Hwacha->FindFunction(TEXT("HandleFuseIgnited"));
        TestNotNull(TEXT("Hwacha exposes its Fuse completion callback"), FuseIgnitedFunction);
        if (FuseIgnitedFunction)
        {
            Hwacha->ProcessEvent(FuseIgnitedFunction, nullptr);
        }
        TestEqual(TEXT("Fuse completion advances into the authored narration"),
            Manager->GetDebugSnapshot().InteractionID, FName(TEXT("INT_AfterFuse")));
        TestTrue(TEXT("Launch request remains queued while narration is current"),
            Hwacha->IsVolleyLaunchPending());
        TestEqual(TEXT("Queued launch does not consume the loaded arrow early"),
            Hwacha->GetLoadedAmmunitionCount(), 1);
        TestTrue(TEXT("Post-fuse narration completes"), Manager->ReportInteractionResult(
            TEXT("Narration_AfterFuse"), EScenarioInteractionType::Custom));
        Hwacha->TryStartPendingVolley();
        TestFalse(TEXT("Queued launch starts when Hwacha_Fire becomes current"),
            Hwacha->IsVolleyLaunchPending());
        TestEqual(TEXT("The single loaded arrow is consumed by the started volley"),
            Hwacha->GetLoadedAmmunitionCount(), 0);
        TestTrue(TEXT("A launched arrow is scheduled for cleanup instead of remaining forever"),
            Arrow->GetLifeSpan() > 0.0f);
    }

    World->DestroyWorld(false);
    GEngine->DestroyWorldContext(World);
    return true;
}

bool FSingijeonFirePitTorchIgnitionFlowTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    if (!TestNotNull(TEXT("Test world is created"), World))
    {
        return false;
    }

    FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
    WorldContext.SetCurrentWorld(World);
    World->InitializeActorsForPlay(FURL());

    AIgnitionSourceActor* Torch = World->SpawnActor<AIgnitionSourceActor>();
    AFirePitActor* FirePit = World->SpawnActor<AFirePitActor>();
    ASingijeonHwachaActor* Hwacha = World->SpawnActor<ASingijeonHwachaActor>();
    UFuseIgnitionComponent* Fuse = Hwacha
        ? Hwacha->FindComponentByClass<UFuseIgnitionComponent>()
        : nullptr;
    UNiagaraComponent* FuseEffect = Hwacha
        ? Hwacha->FindComponentByClass<UNiagaraComponent>()
        : nullptr;
    TestNotNull(TEXT("Unlit torch is spawned"), Torch);
    TestNotNull(TEXT("Fire Pit is spawned"), FirePit);
    TestNotNull(TEXT("Hwacha Fuse exists"), Fuse);
    TestNotNull(TEXT("Hwacha Fuse feedback effect exists"), FuseEffect);
    TestTrue(TEXT("Hwacha Fuse uses a VR-friendly overlap radius"),
        Fuse && Fuse->GetUnscaledSphereRadius() >= 24.0f);
    TestNotNull(TEXT("Hwacha Fuse feedback has a Niagara system"),
        FuseEffect ? FuseEffect->GetAsset() : nullptr);

    UNiagaraComponent* TorchEffect = Torch ? NewObject<UNiagaraComponent>(Torch) : nullptr;
    if (TorchEffect)
    {
        TorchEffect->SetAsset(LoadObject<UNiagaraSystem>(
            nullptr, TEXT("/Game/NiagaraExamples/FX_Misc/NS_Fire.NS_Fire")));
        TorchEffect->RegisterComponent();
        Torch->AddInstanceComponent(TorchEffect);
        Torch->PrepareIgnitionVisuals();
    }
    TestNotNull(TEXT("Test torch fire effect is created"), TorchEffect);
    TestTrue(TEXT("Torch prepares its Niagara instance before interaction"),
        Torch && Torch->AreIgnitionVisualsPrepared());
    TestFalse(TEXT("Unlit prewarmed torch does not render"),
        TorchEffect && TorchEffect->GetRenderingEnabled());

    if (Torch && FirePit && Fuse)
    {
        Fuse->SetIgnitionEnabled(true);
        TestFalse(TEXT("A new torch starts unlit"),
            IIgnitionSourceInterface::Execute_IsIgnitionActive(Torch));
        TestFalse(TEXT("An unlit torch cannot ignite the Fuse"),
            Fuse->TryBeginIgnition(Torch));
        TestTrue(TEXT("Fire Pit ignites the unlit torch"),
            FirePit->TryIgniteTorch(Torch));
        TestTrue(TEXT("Torch is active after touching Fire Pit"),
            IIgnitionSourceInterface::Execute_IsIgnitionActive(Torch));
        TestTrue(TEXT("Ignited torch enables Niagara rendering"),
            TorchEffect && TorchEffect->GetRenderingEnabled());
        TestFalse(TEXT("An unloaded Hwacha rejects even a lit torch"),
            Fuse->TryBeginIgnition(Torch));
    }

    World->DestroyWorld(false);
    GEngine->DestroyWorldContext(World);
    return true;
}

bool FSingijeonHwachaAutoFillGridConfigurationTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    if (!TestNotNull(TEXT("Test world is created"), World))
    {
        return false;
    }

    FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
    WorldContext.SetCurrentWorld(World);
    World->InitializeActorsForPlay(FURL());

    ASingijeonHwachaActor* Hwacha = World->SpawnActor<ASingijeonHwachaActor>();
    TestNotNull(TEXT("Hwacha actor is spawned"), Hwacha);
    if (Hwacha)
    {
        TestEqual(TEXT("Default auto-fill capacity is 6 x 15"), Hwacha->GetAmmunitionCapacity(), 90);
        TestNotNull(TEXT("Hwacha has the project default arrow launch sound"), Hwacha->GetArrowLaunchSound());
        UInstancedStaticMeshComponent* Instances = Hwacha->FindComponentByClass<UInstancedStaticMeshComponent>();
        TestNotNull(TEXT("Hwacha owns an Instanced Static Mesh component"), Instances);
        TestEqual(TEXT("Hwacha starts without loaded ammunition"), Hwacha->GetLoadedAmmunitionCount(), 0);

        ASingijeonProjectileActor* Arrow = World->SpawnActor<ASingijeonProjectileActor>();
        USingijeonAmmoSlotComponent* Slot = Hwacha->FindComponentByClass<USingijeonAmmoSlotComponent>();
        UStaticMesh* TestMesh = LoadObject<UStaticMesh>(
            nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
        UMaterialInterface* TestMaterial = LoadObject<UMaterialInterface>(
            nullptr, TEXT("/Engine/EngineMaterials/DefaultMaterial.DefaultMaterial"));
        UMaterialInterface* RuntimeArrowMaterial = LoadObject<UMaterialInterface>(nullptr,
            TEXT("/GF_Singijeon/Asset/Arrow/arrowb/Materials/M_SingijeonArrow_Runtime.M_SingijeonArrow_Runtime"));
        UStaticMeshComponent* ArrowMesh = Arrow
            ? Cast<UStaticMeshComponent>(Arrow->GetRootComponent())
            : nullptr;
        TestNotNull(TEXT("Default ammunition slot exists"), Slot);
        TestNotNull(TEXT("Test arrow is spawned"), Arrow);
        TestNotNull(TEXT("Test arrow mesh exists"), ArrowMesh);
        TestNotNull(TEXT("Test Static Mesh loads"), TestMesh);
        TestNotNull(TEXT("Test Material loads"), TestMaterial);
        TestNotNull(TEXT("Runtime arrow Material loads"), RuntimeArrowMaterial);
        if (Arrow && Slot && ArrowMesh && TestMesh && TestMaterial && RuntimeArrowMaterial && Instances)
        {
            ArrowMesh->SetStaticMesh(TestMesh);
            ArrowMesh->SetMaterial(0, TestMaterial);
            Hwacha->DispatchBeginPlay();
            TestTrue(TEXT("Test arrow implements ammunition interface"),
                Arrow->GetClass()->ImplementsInterface(USingijeonAmmunitionInterface::StaticClass()));
            TestTrue(TEXT("Test arrow can be loaded"),
                ISingijeonAmmunitionInterface::Execute_CanBeLoaded(Arrow));
            TestTrue(TEXT("Ammunition slot is registered"), Slot->IsRegistered());
            TestTrue(TEXT("Test arrow root is registered"), ArrowMesh->IsRegistered());
            TestTrue(TEXT("Loading one physical arrow succeeds"), Slot->TryLoadAmmunition(Arrow));
            TestEqual(TEXT("One physical arrow auto-fills to 90 loaded arrows"),
                Hwacha->GetLoadedAmmunitionCount(), 90);
            TestEqual(TEXT("Auto-fill creates the remaining 89 mesh instances"),
                Instances->GetInstanceCount(), 89);
            TestEqual(TEXT("Loading restores the physical arrow runtime material after Grab release"),
                ArrowMesh->GetMaterial(0), RuntimeArrowMaterial);
            TestEqual(TEXT("Auto-fill uses the same runtime material as the physical arrow"),
                Instances->GetMaterial(0), RuntimeArrowMaterial);
            TestEqual(TEXT("The loaded physical arrow is attached directly to the ammunition slot"),
                Arrow->GetRootComponent()->GetAttachParent(), static_cast<USceneComponent*>(Slot));

            Arrow->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
            ArrowMesh->SetMaterial(0, nullptr);
            Slot->TickComponent(0.016f, LEVELTICK_All, nullptr);
            TestEqual(TEXT("A delayed XR release cannot leave the loaded arrow detached"),
                Arrow->GetRootComponent()->GetAttachParent(), static_cast<USceneComponent*>(Slot));
            TestEqual(TEXT("Post Physics restores material overwritten by delayed XR release"),
                ArrowMesh->GetMaterial(0), RuntimeArrowMaterial);
            TestFalse(TEXT("The same physical arrow cannot be loaded twice"),
                Slot->TryLoadAmmunition(Arrow));
            TestEqual(TEXT("A repeated load attempt does not duplicate the 6 x 15 grid"),
                Hwacha->GetLoadedAmmunitionCount(), 90);

            UFuseIgnitionComponent* LoadedFuse = Hwacha->FindComponentByClass<UFuseIgnitionComponent>();
            UNiagaraComponent* LoadedFuseEffect = Hwacha->FindComponentByClass<UNiagaraComponent>();
            AIgnitionSourceActor* LitTorch = World->SpawnActor<AIgnitionSourceActor>();
            if (LitTorch)
            {
                LitTorch->SetIgnitionActive(true);
            }
            TestTrue(TEXT("A loaded Hwacha accepts the lit torch at its Fuse"),
                LoadedFuse && LoadedFuse->TryBeginIgnition(LitTorch));
            TestTrue(TEXT("Fuse fire feedback is visible while the torch is held in place"),
                LoadedFuseEffect && Hwacha->IsFuseIgnitionEffectRequested());
            if (LoadedFuse)
            {
                LoadedFuse->CancelIgnition();
            }
            TestFalse(TEXT("Fuse fire feedback stops when the torch is removed"),
                LoadedFuseEffect && Hwacha->IsFuseIgnitionEffectRequested());

            USceneComponent* LeftGrabPoint = nullptr;
            USceneComponent* RightGrabPoint = nullptr;
            TInlineComponentArray<USceneComponent*> SceneComponents(Hwacha);
            for (USceneComponent* SceneComponent : SceneComponents)
            {
                if (SceneComponent->GetFName() == TEXT("LeftHandleGrabPoint"))
                {
                    LeftGrabPoint = SceneComponent;
                }
                else if (SceneComponent->GetFName() == TEXT("RightHandleGrabPoint"))
                {
                    RightGrabPoint = SceneComponent;
                }
            }
            TestNotNull(TEXT("Left native VR grab point exists"), LeftGrabPoint);
            TestNotNull(TEXT("Right native VR grab point exists"), RightGrabPoint);
            TestTrue(TEXT("Left handle is discoverable by the Core VR Pawn"),
                LeftGrabPoint && LeftGrabPoint->ComponentHasTag(TEXT("VRGrab")));
            TestTrue(TEXT("Right handle is discoverable by the Core VR Pawn"),
                RightGrabPoint && RightGrabPoint->ComponentHasTag(TEXT("VRGrab")));

            UTwoHandCarryComponent* Carry = Hwacha->FindComponentByClass<UTwoHandCarryComponent>();
            USceneComponent* LeftHand = NewObject<USceneComponent>(Hwacha);
            USceneComponent* RightHand = NewObject<USceneComponent>(Hwacha);
            LeftHand->RegisterComponent();
            RightHand->RegisterComponent();
            LeftHand->SetWorldLocation(FVector(0.0, -30.0, 80.0));
            RightHand->SetWorldLocation(FVector(0.0, 30.0, 80.0));
            UStaticMeshComponent* LeftHighlight = nullptr;
            UStaticMeshComponent* RightHighlight = nullptr;
            UStaticMeshComponent* MoveTargetMarker = nullptr;
            TInlineComponentArray<UStaticMeshComponent*> MeshComponents(Hwacha);
            for (UStaticMeshComponent* MeshComponent : MeshComponents)
            {
                if (MeshComponent->GetFName() == TEXT("LeftHandleHighlight"))
                {
                    LeftHighlight = MeshComponent;
                }
                else if (MeshComponent->GetFName() == TEXT("RightHandleHighlight"))
                {
                    RightHighlight = MeshComponent;
                }
                else if (MeshComponent->GetFName() == TEXT("MoveTargetMarker"))
                {
                    MoveTargetMarker = MeshComponent;
                }
            }
            TestNotNull(TEXT("Two-hand carry component exists"), Carry);
            TestNotNull(TEXT("Left handle highlight exists"), LeftHighlight);
            TestNotNull(TEXT("Right handle highlight exists"), RightHighlight);
            TestNotNull(TEXT("Move target marker exists"), MoveTargetMarker);
            if (Carry && LeftHighlight && RightHighlight && MoveTargetMarker)
            {
                TestTrue(TEXT("The visible left cylinder is itself grabbable"),
                    LeftHighlight->ComponentHasTag(TEXT("VRGrab")));
                TestTrue(TEXT("The visible right cylinder is itself grabbable"),
                    RightHighlight->ComponentHasTag(TEXT("VRGrab")));
                TestTrue(TEXT("Loaded Hwacha shows the left handle grab guide"),
                    LeftHighlight->IsVisible());
                TestTrue(TEXT("Loaded Hwacha shows the right handle grab guide"),
                    RightHighlight->IsVisible());
                TestTrue(TEXT("Loaded Hwacha shows its world move target"),
                    MoveTargetMarker->IsVisible());

                const FVector InitialLocation = Hwacha->GetActorLocation();
                const FVector InitialArrowOffset = Arrow->GetActorLocation() - InitialLocation;
                FTransform InitialInstanceWorldTransform;
                TestTrue(TEXT("An auto-filled arrow world transform is readable before carry"),
                    Instances->GetInstanceTransform(0, InitialInstanceWorldTransform, true));
                MoveTargetMarker->SetWorldLocation(InitialLocation + FVector(100.0, 0.0, 0.0));
                TestTrue(TEXT("Left hand can grip the loaded Hwacha"),
                    Carry->BeginGrip(ECarryGripSide::Left, LeftHand));
                TestTrue(TEXT("A left-only grip starts carrying"), Carry->IsBeingCarried());
                TestFalse(TEXT("Handle guides hide during a left-only carry"),
                    LeftHighlight->IsVisible() || RightHighlight->IsVisible());
                LeftHand->SetWorldLocation(LeftHand->GetComponentLocation() + FVector(30.0, 0.0, 0.0));
                Carry->TickComponent(1.0f, LEVELTICK_All, nullptr);
                TestTrue(TEXT("Moving only the left hand moves the Hwacha"),
                    Hwacha->GetActorLocation().X > InitialLocation.X + 20.0f);
                TestTrue(TEXT("The physical loaded arrow follows the moved Hwacha without drift"),
                    (Arrow->GetActorLocation() - Hwacha->GetActorLocation()).Equals(
                        InitialArrowOffset, 0.1f));
                FTransform MovedInstanceWorldTransform;
                TestTrue(TEXT("An auto-filled arrow world transform is readable after carry"),
                    Instances->GetInstanceTransform(0, MovedInstanceWorldTransform, true));
                TestTrue(TEXT("Auto-filled arrows follow the same Hwacha carry delta"),
                    (MovedInstanceWorldTransform.GetLocation() - InitialInstanceWorldTransform.GetLocation()).Equals(
                        Hwacha->GetActorLocation() - InitialLocation, 0.1f));
                Carry->EndGrip(ECarryGripSide::Left, LeftHand);
                TestFalse(TEXT("Releasing the only grip stops carrying"), Carry->IsBeingCarried());
                TestTrue(TEXT("Handle guides return after an incomplete release"),
                    LeftHighlight->IsVisible() && RightHighlight->IsVisible());

                const float BeforeRightDragX = Hwacha->GetActorLocation().X;
                TestTrue(TEXT("Right hand can grip the loaded Hwacha by itself"),
                    Carry->BeginGrip(ECarryGripSide::Right, RightHand));
                TestTrue(TEXT("A right-only grip starts carrying"), Carry->IsBeingCarried());
                RightHand->SetWorldLocation(RightHand->GetComponentLocation() + FVector(70.0, 0.0, 0.0));
                Carry->TickComponent(1.0f, LEVELTICK_All, nullptr);
                TestTrue(TEXT("Moving only the right hand moves the Hwacha"),
                    Hwacha->GetActorLocation().X > BeforeRightDragX + 60.0f);
                Hwacha->Tick(0.016f);
                TestTrue(TEXT("Reaching the displayed target completes the aim interaction"),
                    Hwacha->IsAimInteractionComplete());
                TestTrue(TEXT("Successful placement snaps the Hwacha onto the target"),
                    FVector::Dist2D(Hwacha->GetActorLocation(), MoveTargetMarker->GetComponentLocation()) < 1.0f);
                TestFalse(TEXT("Move target hides after aim completion"),
                    MoveTargetMarker->IsVisible());
                TestTrue(TEXT("Aim completion reveals the exact Fuse contact guide"),
                    Hwacha->IsFuseGuideVisible());
            }

            TestNotNull(TEXT("The physical arrow can be unloaded"), Slot->UnloadAmmunition());
            TestEqual(TEXT("Unloading the physical arrow clears auto-filled ammunition"),
                Hwacha->GetLoadedAmmunitionCount(), 0);
            TestEqual(TEXT("Unloading clears all mesh instances"), Instances->GetInstanceCount(), 0);

            TestTrue(TEXT("The arrow can be loaded again after an explicit unload"),
                Slot->TryLoadAmmunition(Arrow));
            Hwacha->LaunchVolley();
            TestEqual(TEXT("Volley duration defaults to ten seconds"),
                Hwacha->GetVolleyDuration(), 10.0f);
            TestTrue(TEXT("A 90-arrow volley spaces launches across ten seconds"),
                FMath::IsNearlyEqual(Hwacha->GetCurrentLaunchInterval(), 10.0f / 89.0f, 0.001f));
            TestEqual(TEXT("The first random arrow launches immediately"),
                Hwacha->GetLoadedAmmunitionCount(), 89);
            UFunction* LaunchNextFunction = Hwacha->FindFunction(TEXT("LaunchNextAmmunition"));
            TestNotNull(TEXT("The sequential launch callback is registered"), LaunchNextFunction);
            if (LaunchNextFunction)
            {
                Hwacha->ProcessEvent(LaunchNextFunction, nullptr);
                TestEqual(TEXT("The second launch removes exactly one more visible arrow"),
                    Hwacha->GetLoadedAmmunitionCount(), 88);
            }
            for (int32 ShotIndex = 0; LaunchNextFunction && ShotIndex < 88; ++ShotIndex)
            {
                Hwacha->ProcessEvent(LaunchNextFunction, nullptr);
            }
            TestEqual(TEXT("Every arrow has launched by the end of the ten-second volley"),
                Hwacha->GetLoadedAmmunitionCount(), 0);
            TestTrue(TEXT("The physical arrow launches in its authored arrowhead direction"),
                FVector::DotProduct(Arrow->GetVelocity().GetSafeNormal(),
                    Arrow->GetArrowTipDirection()) > 0.99f);
            TestTrue(TEXT("A launched arrow ignores its owning Hwacha"),
                ArrowMesh->GetMoveIgnoreActors().Contains(Hwacha));
        }
    }

    World->DestroyWorld(false);
    GEngine->DestroyWorldContext(World);
    return true;
}

#endif
