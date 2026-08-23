#include "Singijeon/SingijeonArrowBoxActor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ASingijeonArrowBoxActor::ASingijeonArrowBoxActor()
{
    Bottom=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowBoxBottom")); SetRootComponent(Bottom);
    LeftWall=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowBoxLeftWall")); LeftWall->SetupAttachment(Bottom);
    RightWall=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowBoxRightWall")); RightWall->SetupAttachment(Bottom);
    FrontWall=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowBoxFrontWall")); FrontWall->SetupAttachment(Bottom);
    BackWall=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowBoxBackWall")); BackWall->SetupAttachment(Bottom);
    ArrowInstances=CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ArrowInstances")); ArrowInstances->SetupAttachment(Bottom);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Arrow(TEXT("/GF_Singijeon/Asset/Arrow/arrowb/StaticMeshes/arrowb.arrowb"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Wood(TEXT("/GF_Singijeon/Gameplay/Props/Materials/M_SingijeonArrowBox_Wood.M_SingijeonArrowBox_Wood"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> ArrowMat(TEXT("/GF_Singijeon/Asset/Arrow/arrowb/Materials/M_SingijeonArrow_Runtime.M_SingijeonArrow_Runtime"));
    for (UStaticMeshComponent* Board : {Bottom,LeftWall,RightWall,FrontWall,BackWall}) { Board->SetStaticMesh(Cube.Object); Board->SetMaterial(0,Wood.Object); Board->SetMobility(EComponentMobility::Static); }
    ArrowInstances->SetStaticMesh(Arrow.Object); ArrowInstances->SetMaterial(0,ArrowMat.Object); ArrowInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision); ArrowInstances->SetCastShadow(false);
}
void ASingijeonArrowBoxActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform); const float L=220,W=136,H=70,T=5;
    Bottom->SetRelativeScale3D(FVector(L/100.f,W/100.f,8.f/100.f));
    LeftWall->SetRelativeLocation(FVector(0,-W/2+T/2,H/2)); LeftWall->SetRelativeScale3D(FVector(L/100.f,T/100.f,H/100.f));
    RightWall->SetRelativeLocation(FVector(0,W/2-T/2,H/2)); RightWall->SetRelativeScale3D(FVector(L/100.f,T/100.f,H/100.f));
    FrontWall->SetRelativeLocation(FVector(-L/2+T/2,0,H/2)); FrontWall->SetRelativeScale3D(FVector(T/100.f,W/100.f,H/100.f));
    BackWall->SetRelativeLocation(FVector(L/2-T/2,0,H/2)); BackWall->SetRelativeScale3D(FVector(T/100.f,W/100.f,H/100.f));
    ArrowInstances->ClearInstances(); for(int32 R=0;R<6;++R) for(int32 C=0;C<15;++C) ArrowInstances->AddInstance(FTransform(FVector(0,-56+C*8,8+R*8)));
}
