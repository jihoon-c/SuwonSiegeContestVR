#include "Ongseong/OngseongSpawnPointActor.h"

#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"

AOngseongSpawnPointActor::AOngseongSpawnPointActor()
{
	PrimaryActorTick.bCanEverTick = false;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	DirectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("DirectionArrow"));
	DirectionArrow->SetupAttachment(Root);
	DirectionArrow->SetHiddenInGame(true);
	DirectionArrow->ArrowSize = 2.0f;

	RoleLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("RoleLabel"));
	RoleLabel->SetupAttachment(Root);
	RoleLabel->SetRelativeLocation(FVector(0.0f, 0.0f, 80.0f));
	RoleLabel->SetHorizontalAlignment(EHTA_Center);
	RoleLabel->SetWorldSize(28.0f);
	RoleLabel->SetHiddenInGame(true);

	UpdateEditorPreview();
}

void AOngseongSpawnPointActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateEditorPreview();
}

void AOngseongSpawnPointActor::UpdateEditorPreview()
{
	FText Label;
	FLinearColor Color;
	switch (SpawnPointRole)
	{
	case EOngseongSpawnPointRole::SoldierRespawn:
		Label = FText::FromString(TEXT("SOLDIER RESPAWN"));
		Color = FLinearColor(0.15f, 0.65f, 1.0f);
		break;
	case EOngseongSpawnPointRole::Ram:
		Label = FText::FromString(TEXT("BATTERING RAM SPAWN"));
		Color = FLinearColor(1.0f, 0.35f, 0.05f);
		break;
	default:
		Label = FText::FromString(TEXT("ENEMY INITIAL SPAWN"));
		Color = FLinearColor(0.9f, 0.1f, 0.1f);
		break;
	}
	if (DirectionArrow)
	{
		DirectionArrow->ArrowColor = Color.ToFColor(true);
	}
	if (RoleLabel)
	{
		RoleLabel->SetText(Label);
		RoleLabel->SetTextRenderColor(Color.ToFColor(true));
	}
}
