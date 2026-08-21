#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSingijeonHwachaAutoFillGridConfigurationTest,
    "SuwonSiegeContestVR.GF_Singijeon.Hwacha.AutoFillGridConfiguration",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSingijeonFirePitTorchIgnitionFlowTest,
    "SuwonSiegeContestVR.GF_Singijeon.Ignition.FirePitTorchFuseOrder",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

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
    TestNotNull(TEXT("Unlit torch is spawned"), Torch);
    TestNotNull(TEXT("Fire Pit is spawned"), FirePit);
    TestNotNull(TEXT("Hwacha Fuse exists"), Fuse);

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
        TestTrue(TEXT("The lit torch can begin Fuse ignition"),
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
        UInstancedStaticMeshComponent* Instances = Hwacha->FindComponentByClass<UInstancedStaticMeshComponent>();
        TestNotNull(TEXT("Hwacha owns an Instanced Static Mesh component"), Instances);
        TestEqual(TEXT("Hwacha starts without loaded ammunition"), Hwacha->GetLoadedAmmunitionCount(), 0);

        ASingijeonProjectileActor* Arrow = World->SpawnActor<ASingijeonProjectileActor>();
        USingijeonAmmoSlotComponent* Slot = Hwacha->FindComponentByClass<USingijeonAmmoSlotComponent>();
        UStaticMesh* TestMesh = LoadObject<UStaticMesh>(
            nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
        UMaterialInterface* TestMaterial = LoadObject<UMaterialInterface>(
            nullptr, TEXT("/Engine/EngineMaterials/DefaultMaterial.DefaultMaterial"));
        UStaticMeshComponent* ArrowMesh = Arrow
            ? Cast<UStaticMeshComponent>(Arrow->GetRootComponent())
            : nullptr;
        TestNotNull(TEXT("Default ammunition slot exists"), Slot);
        TestNotNull(TEXT("Test arrow is spawned"), Arrow);
        TestNotNull(TEXT("Test arrow mesh exists"), ArrowMesh);
        TestNotNull(TEXT("Test Static Mesh loads"), TestMesh);
        TestNotNull(TEXT("Test Material loads"), TestMaterial);
        if (Arrow && Slot && ArrowMesh && TestMesh && TestMaterial && Instances)
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
            TestEqual(TEXT("Auto-fill copies the physical arrow material"),
                Instances->GetMaterial(0), TestMaterial);

            UTwoHandCarryComponent* Carry = Hwacha->FindComponentByClass<UTwoHandCarryComponent>();
            USceneComponent* LeftHand = NewObject<USceneComponent>(Hwacha);
            USceneComponent* RightHand = NewObject<USceneComponent>(Hwacha);
            LeftHand->RegisterComponent();
            RightHand->RegisterComponent();
            LeftHand->SetWorldLocation(FVector(0.0, -30.0, 80.0));
            RightHand->SetWorldLocation(FVector(0.0, 30.0, 80.0));
            UStaticMeshComponent* LeftHighlight = nullptr;
            UStaticMeshComponent* RightHighlight = nullptr;
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
            }
            TestNotNull(TEXT("Two-hand carry component exists"), Carry);
            TestNotNull(TEXT("Left handle highlight exists"), LeftHighlight);
            TestNotNull(TEXT("Right handle highlight exists"), RightHighlight);
            if (Carry && LeftHighlight && RightHighlight)
            {
                TestTrue(TEXT("Loaded Hwacha shows the left handle guide before grabbing"),
                    LeftHighlight->IsVisible());
                TestTrue(TEXT("Loaded Hwacha shows the right handle guide before grabbing"),
                    RightHighlight->IsVisible());
                TestTrue(TEXT("Left hand can grip the loaded Hwacha"),
                    Carry->BeginGrip(ECarryGripSide::Left, LeftHand));
                TestTrue(TEXT("Guide remains until both hands are correctly placed"),
                    LeftHighlight->IsVisible());
                TestTrue(TEXT("Right hand can complete the two-hand grip"),
                    Carry->BeginGrip(ECarryGripSide::Right, RightHand));
                TestFalse(TEXT("Left guide hides after the correct two-hand grab"),
                    LeftHighlight->IsVisible());
                TestFalse(TEXT("Right guide hides after the correct two-hand grab"),
                    RightHighlight->IsVisible());
                Carry->EndGrip(ECarryGripSide::Left, LeftHand);
                TestTrue(TEXT("Left guide returns after releasing before completion"),
                    LeftHighlight->IsVisible());
                TestTrue(TEXT("Right guide returns after releasing before completion"),
                    RightHighlight->IsVisible());

                TestTrue(TEXT("Left hand can retry the aim interaction"),
                    Carry->BeginGrip(ECarryGripSide::Left, LeftHand));
                TestTrue(TEXT("Right hand can retry the aim interaction"),
                    Carry->BeginGrip(ECarryGripSide::Right, RightHand));
                Hwacha->SetActorLocation(Hwacha->GetActorLocation() + FVector(50.0, 0.0, 0.0));
                Hwacha->Tick(0.016f);
                TestTrue(TEXT("Sufficient movement completes the aim interaction"),
                    Hwacha->IsAimInteractionComplete());
                TestFalse(TEXT("Left guide stays hidden after aim completion"),
                    LeftHighlight->IsVisible());
                TestFalse(TEXT("Right guide stays hidden after aim completion"),
                    RightHighlight->IsVisible());
            }

            TestNotNull(TEXT("The physical arrow can be unloaded"), Slot->UnloadAmmunition());
            TestEqual(TEXT("Unloading the physical arrow clears auto-filled ammunition"),
                Hwacha->GetLoadedAmmunitionCount(), 0);
            TestEqual(TEXT("Unloading clears all mesh instances"), Instances->GetInstanceCount(), 0);
        }
    }

    World->DestroyWorld(false);
    GEngine->DestroyWorldContext(World);
    return true;
}

#endif
