#include "SuwonSiegeContestVREditor.h"

#include "Algo/MaxElement.h"
#include "Algo/MinElement.h"
#include "ContentBrowserMenuContexts.h"
#include "ComponentVisualizer.h"
#include "Core/Narration/NarrationTypes.h"
#include "Core/Scenario/ScenarioInteractableComponent.h"
#include "DataTableEditorUtils.h"
#include "Editor.h"
#include "Engine/DataTable.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "Landscape.h"
#include "LandscapeDataAccess.h"
#include "LandscapeEdit.h"
#include "LandscapeEditLayer.h"
#include "LandscapeLayerInfoObject.h"
#include "LandscapeStreamingProxy.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundWave.h"
#include "Subsystems/EditorAssetSubsystem.h"
#include "ToolMenus.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "ScopedTransaction.h"
#include "ScenarioGuideComponentVisualizer.h"
#include "Editor/UnrealEdEngine.h"
#include "EditorLevelUtils.h"
#include "Engine/Font.h"
#include "Engine/FontFace.h"
#include "Engine/LevelStreamingAlwaysLoaded.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UnrealEdGlobals.h"

#define LOCTEXT_NAMESPACE "SuwonSiegeContestVREditor"

namespace SuwonLandscapeTransfer
{
	constexpr const TCHAR* SourceMap = TEXT("/Game/Namhansanseong/Maps/Demo_Namhansanseong");
	constexpr const TCHAR* TargetMap = TEXT("/GF_Singijeon/Maps/LV_Singijeon");
	constexpr const TCHAR* LandscapeMaterialPath = TEXT("/Game/Namhansanseong/Materials/Landscape/MI_Landscape.MI_Landscape");

	bool GetProxyHeightData(ALandscapeProxy* Proxy, TArray<uint16>& OutData, FIntRect& OutRect)
	{
		if (!Proxy || !Proxy->GetLandscapeInfo())
		{
			return false;
		}

		OutRect = Proxy->GetBoundingRect() + Proxy->GetSectionBase();
		const int32 Width = OutRect.Width() + 1;
		const int32 Height = OutRect.Height() + 1;
		if (Width <= 1 || Height <= 1)
		{
			return false;
		}

		OutData.SetNumUninitialized(Width * Height);
		FLandscapeEditDataInterface LandscapeEdit(Proxy->GetLandscapeInfo());
		LandscapeEdit.GetHeightDataFast(
			OutRect.Min.X,
			OutRect.Min.Y,
			OutRect.Max.X,
			OutRect.Max.Y,
			OutData.GetData(),
			Width);
		return true;
	}

	TArray<uint16> ResampleHeightData(
		const TArray<uint16>& Source,
		const int32 SourceWidth,
		const int32 SourceHeight,
		const int32 TargetWidth,
		const int32 TargetHeight)
	{
		TArray<uint16> Result;
		Result.SetNumUninitialized(TargetWidth * TargetHeight);

		for (int32 Y = 0; Y < TargetHeight; ++Y)
		{
			const double SourceY = TargetHeight > 1
				? static_cast<double>(Y) * static_cast<double>(SourceHeight - 1) / static_cast<double>(TargetHeight - 1)
				: 0.0;
			const int32 Y0 = FMath::FloorToInt(SourceY);
			const int32 Y1 = FMath::Min(Y0 + 1, SourceHeight - 1);
			const double YAlpha = SourceY - static_cast<double>(Y0);

			for (int32 X = 0; X < TargetWidth; ++X)
			{
				const double SourceX = TargetWidth > 1
					? static_cast<double>(X) * static_cast<double>(SourceWidth - 1) / static_cast<double>(TargetWidth - 1)
					: 0.0;
				const int32 X0 = FMath::FloorToInt(SourceX);
				const int32 X1 = FMath::Min(X0 + 1, SourceWidth - 1);
				const double XAlpha = SourceX - static_cast<double>(X0);

				const double H00 = Source[Y0 * SourceWidth + X0];
				const double H10 = Source[Y0 * SourceWidth + X1];
				const double H01 = Source[Y1 * SourceWidth + X0];
				const double H11 = Source[Y1 * SourceWidth + X1];
				const double H0 = FMath::Lerp(H00, H10, XAlpha);
				const double H1 = FMath::Lerp(H01, H11, XAlpha);
				Result[Y * TargetWidth + X] = static_cast<uint16>(
					FMath::Clamp(FMath::RoundToInt(FMath::Lerp(H0, H1, YAlpha)), 0, 65535));
			}
		}

		return Result;
	}

	TArray<uint8> ResampleWeightData(
		const TArray<uint8>& Source,
		const int32 SourceWidth,
		const int32 SourceHeight,
		const int32 TargetWidth,
		const int32 TargetHeight)
	{
		TArray<uint8> Result;
		Result.SetNumUninitialized(TargetWidth * TargetHeight);

		for (int32 Y = 0; Y < TargetHeight; ++Y)
		{
			const double SourceY = TargetHeight > 1
				? static_cast<double>(Y) * static_cast<double>(SourceHeight - 1) / static_cast<double>(TargetHeight - 1)
				: 0.0;
			const int32 Y0 = FMath::FloorToInt(SourceY);
			const int32 Y1 = FMath::Min(Y0 + 1, SourceHeight - 1);
			const double YAlpha = SourceY - static_cast<double>(Y0);

			for (int32 X = 0; X < TargetWidth; ++X)
			{
				const double SourceX = TargetWidth > 1
					? static_cast<double>(X) * static_cast<double>(SourceWidth - 1) / static_cast<double>(TargetWidth - 1)
					: 0.0;
				const int32 X0 = FMath::FloorToInt(SourceX);
				const int32 X1 = FMath::Min(X0 + 1, SourceWidth - 1);
				const double XAlpha = SourceX - static_cast<double>(X0);

				const double W00 = Source[Y0 * SourceWidth + X0];
				const double W10 = Source[Y0 * SourceWidth + X1];
				const double W01 = Source[Y1 * SourceWidth + X0];
				const double W11 = Source[Y1 * SourceWidth + X1];
				const double W0 = FMath::Lerp(W00, W10, XAlpha);
				const double W1 = FMath::Lerp(W01, W11, XAlpha);
				Result[Y * TargetWidth + X] = static_cast<uint8>(
					FMath::Clamp(FMath::RoundToInt(FMath::Lerp(W0, W1, YAlpha)), 0, 255));
			}
		}

		return Result;
	}

	struct FLandscapeLayerTransferData
	{
		TObjectPtr<ULandscapeLayerInfoObject> LayerInfo;
		TArray<uint8> Weights;
	};

	void TransferNamhansanseongLandscape()
	{
		UE_LOG(LogTemp, Display, TEXT("NAMHANSANSEONG_LANDSCAPE_TRANSFER starting"));
		if (!FEditorFileUtils::LoadMap(SourceMap, false, true))
		{
			UE_LOG(LogTemp, Error, TEXT("NAMHANSANSEONG_LANDSCAPE_TRANSFER could not load source map: %s"), SourceMap);
			return;
		}

		UWorld* SourceWorld = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
		ALandscapeStreamingProxy* SourceProxy = nullptr;
		double BestDistanceSquared = TNumericLimits<double>::Max();
		for (TActorIterator<ALandscapeStreamingProxy> It(SourceWorld); It; ++It)
		{
			const FVector Center = It->GetProxyBounds().GetCenter();
			const double DistanceSquared = FMath::Square(Center.X) + FMath::Square(Center.Y);
			if (DistanceSquared < BestDistanceSquared)
			{
				BestDistanceSquared = DistanceSquared;
				SourceProxy = *It;
			}
		}

		TArray<uint16> SourceData;
		FIntRect SourceRect;
		if (!GetProxyHeightData(SourceProxy, SourceData, SourceRect))
		{
			UE_LOG(LogTemp, Error, TEXT("NAMHANSANSEONG_LANDSCAPE_TRANSFER could not read central source proxy"));
			return;
		}

		const int32 SourceWidth = SourceRect.Width() + 1;
		const int32 SourceHeight = SourceRect.Height() + 1;
		TSet<ULandscapeLayerInfoObject*> SourceLayers;
		for (ULandscapeComponent* Component : SourceProxy->LandscapeComponents)
		{
			for (const FWeightmapLayerAllocationInfo& Allocation : Component->GetWeightmapLayerAllocations())
			{
				if (Allocation.LayerInfo)
				{
					SourceLayers.Add(Allocation.LayerInfo);
				}
			}
		}

		TArray<FLandscapeLayerTransferData> LayerTransfers;
		for (ULandscapeLayerInfoObject* LayerInfo : SourceLayers)
		{
			FLandscapeLayerTransferData& Transfer = LayerTransfers.AddDefaulted_GetRef();
			Transfer.LayerInfo = LayerInfo;
			Transfer.Weights.SetNumUninitialized(SourceWidth * SourceHeight);
			FLandscapeEditDataInterface LandscapeEdit(SourceProxy->GetLandscapeInfo());
			LandscapeEdit.GetWeightDataFast(
				LayerInfo,
				SourceRect.Min.X,
				SourceRect.Min.Y,
				SourceRect.Max.X,
				SourceRect.Max.Y,
				Transfer.Weights.GetData(),
				SourceWidth);
		}
		UE_LOG(
			LogTemp,
			Display,
			TEXT("NAMHANSANSEONG_LANDSCAPE_TRANSFER source proxy=%s samples=%dx%d min=%u max=%u layers=%d"),
			*GetNameSafe(SourceProxy),
			SourceWidth,
			SourceHeight,
			*Algo::MinElement(SourceData),
			*Algo::MaxElement(SourceData),
			LayerTransfers.Num());

		if (!FEditorFileUtils::LoadMap(TargetMap, false, true))
		{
			UE_LOG(LogTemp, Error, TEXT("NAMHANSANSEONG_LANDSCAPE_TRANSFER could not load target map: %s"), TargetMap);
			return;
		}

		UWorld* TargetWorld = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
		ALandscape* TargetLandscape = nullptr;
		for (TActorIterator<ALandscape> It(TargetWorld); It; ++It)
		{
			TargetLandscape = *It;
			break;
		}

		if (!TargetLandscape)
		{
			UE_LOG(LogTemp, Error, TEXT("NAMHANSANSEONG_LANDSCAPE_TRANSFER target Landscape was not found"));
			return;
		}

		const FIntRect TargetRect = TargetLandscape->GetBoundingRect() + TargetLandscape->GetSectionBase();
		const int32 TargetWidth = TargetRect.Width() + 1;
		const int32 TargetHeight = TargetRect.Height() + 1;
		const TArray<uint16> TargetData = ResampleHeightData(
			SourceData,
			SourceWidth,
			SourceHeight,
			TargetWidth,
			TargetHeight);

		TargetLandscape->Modify();
		TargetLandscape->SetActorScale3D(FVector(100.0, 100.0, 60.0));
		TargetLandscape->MaxLODLevel = 2;
		TargetLandscape->Tags.AddUnique(TEXT("NamhansanseongTerrain"));
		TargetLandscape->SetActorLabel(TEXT("Landscape_Namhansanseong_Singijeon"));

		UMaterialInterface* LandscapeMaterial = LoadObject<UMaterialInterface>(nullptr, LandscapeMaterialPath);
		if (!LandscapeMaterial)
		{
			UE_LOG(LogTemp, Error, TEXT("NAMHANSANSEONG_LANDSCAPE_TRANSFER material missing: %s"), LandscapeMaterialPath);
			return;
		}
		TargetLandscape->LandscapeMaterial = LandscapeMaterial;

		ULandscapeEditLayerBase* TargetEditLayer = TargetLandscape->GetEditLayer(0);
		{
			FHeightmapAccessor<false> HeightmapAccessor(TargetLandscape->GetLandscapeInfo());
			if (TargetEditLayer)
			{
				HeightmapAccessor.SetEditLayer(TargetEditLayer->GetGuid());
			}
			HeightmapAccessor.SetData(
				TargetRect.Min.X,
				TargetRect.Min.Y,
				TargetRect.Max.X,
				TargetRect.Max.Y,
				TargetData.GetData());
		}
		for (const FLandscapeLayerTransferData& Transfer : LayerTransfers)
		{
			const TArray<uint8> TargetWeights = ResampleWeightData(
				Transfer.Weights,
				SourceWidth,
				SourceHeight,
				TargetWidth,
				TargetHeight);
			TAlphamapAccessor<false> WeightmapAccessor(TargetLandscape->GetLandscapeInfo(), Transfer.LayerInfo);
			if (TargetEditLayer)
			{
				WeightmapAccessor.SetEditLayer(TargetEditLayer->GetGuid());
			}
			WeightmapAccessor.SetData(
				TargetRect.Min.X,
				TargetRect.Min.Y,
				TargetRect.Max.X,
				TargetRect.Max.Y,
				TargetWeights.GetData(),
				ELandscapeLayerPaintingRestriction::None);
			UE_LOG(
				LogTemp,
				Display,
				TEXT("NAMHANSANSEONG_LANDSCAPE_TRANSFER layer=%s min=%u max=%u"),
				*GetNameSafe(Transfer.LayerInfo),
				*Algo::MinElement(TargetWeights),
				*Algo::MaxElement(TargetWeights));
		}
		TargetLandscape->RequestLayersContentUpdateForceAll(ELandscapeLayerUpdateMode::Update_All, true);
		TargetLandscape->ForceUpdateLayersContent();

		const uint16 CenterHeight = TargetData[(TargetHeight / 2) * TargetWidth + TargetWidth / 2];
		FVector Location = TargetLandscape->GetActorLocation();
		Location.Z = -LandscapeDataAccess::GetLocalHeight(CenterHeight) * TargetLandscape->GetActorScale3D().Z;
		TargetLandscape->SetActorLocation(Location, false, nullptr, ETeleportType::TeleportPhysics);
		TargetLandscape->PostEditChange();
		TargetLandscape->MarkPackageDirty();

		if (!FEditorFileUtils::SaveCurrentLevel())
		{
			UE_LOG(LogTemp, Error, TEXT("NAMHANSANSEONG_LANDSCAPE_TRANSFER failed to save target map"));
			return;
		}

		UE_LOG(
			LogTemp,
			Display,
			TEXT("NAMHANSANSEONG_LANDSCAPE_TRANSFER SUCCESS target=%dx%d edit_layer=%s center_height=%u landscape_z=%.3f"),
			TargetWidth,
			TargetHeight,
			*GetNameSafe(TargetEditLayer),
			CenterHeight,
			Location.Z);
	}

	FAutoConsoleCommand TransferCommand(
		TEXT("Suwon.TransferNamhansanseongLandscape"),
		TEXT("Transfers the central Demo_Namhansanseong terrain height data into LV_Singijeon."),
		FConsoleCommandDelegate::CreateStatic(&TransferNamhansanseongLandscape));
}

namespace SuwonLandscapeReuse
{
	constexpr const TCHAR* SharedLandscapeMap = TEXT("/Game/Maps/Main/L_NamhansanseongLandscape");

	bool HasSharedLandscapeStreamingLevel(const UWorld* World)
	{
		if (!World)
		{
			return false;
		}

		for (const ULevelStreaming* StreamingLevel : World->GetStreamingLevels())
		{
			if (StreamingLevel && StreamingLevel->GetWorldAssetPackageName() == FName(SharedLandscapeMap))
			{
				return true;
			}
		}
		return false;
	}

	void AttachSharedLandscapeToCurrentLevel()
	{
		UWorld* TargetWorld = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
		if (!TargetWorld)
		{
			UE_LOG(LogTemp, Error, TEXT("NAMHANSANSEONG_LANDSCAPE_REUSE no editor world is open"));
			return;
		}

		if (TargetWorld->GetOutermost()->GetName() == SharedLandscapeMap)
		{
			UE_LOG(LogTemp, Error, TEXT("NAMHANSANSEONG_LANDSCAPE_REUSE cannot stream the Landscape map into itself"));
			return;
		}

		if (HasSharedLandscapeStreamingLevel(TargetWorld))
		{
			UE_LOG(
				LogTemp,
				Display,
				TEXT("NAMHANSANSEONG_LANDSCAPE_REUSE already attached target=%s"),
				*TargetWorld->GetOutermost()->GetName());
			return;
		}

		const FScopedTransaction Transaction(LOCTEXT("AttachNamhansanseongLandscape", "Attach Namhansanseong Landscape"));
		TargetWorld->Modify();
		ULevelStreaming* StreamingLevel = UEditorLevelUtils::AddLevelToWorld(
			TargetWorld,
			SharedLandscapeMap,
			ULevelStreamingAlwaysLoaded::StaticClass());
		if (!StreamingLevel)
		{
			UE_LOG(
				LogTemp,
				Error,
				TEXT("NAMHANSANSEONG_LANDSCAPE_REUSE failed to attach target=%s"),
				*TargetWorld->GetOutermost()->GetName());
			return;
		}

		TargetWorld->MarkPackageDirty();
		UE_LOG(
			LogTemp,
			Display,
			TEXT("NAMHANSANSEONG_LANDSCAPE_REUSE SUCCESS target=%s streaming=%s"),
			*TargetWorld->GetOutermost()->GetName(),
			*StreamingLevel->GetPathName());
	}

	FAutoConsoleCommand AttachSharedLandscapeCommand(
		TEXT("Suwon.AttachNamhansanseongLandscape"),
		TEXT("Adds the shared Namhansanseong Landscape as an always-loaded streaming level to the currently open level."),
		FConsoleCommandDelegate::CreateStatic(&AttachSharedLandscapeToCurrentLevel));
}

namespace SuwonFlatLandscape
{
	constexpr int32 ComponentCount = 4;
	constexpr int32 QuadsPerSection = 63;
	constexpr int32 SectionsPerComponent = 1;
	constexpr uint16 FlatHeight = 32768;
	const FVector AerialPlatformCenter(0.0, 0.0, 100000.0);
	const FName FlatLandscapeTag(TEXT("FlatLandscape4x4"));
	constexpr const TCHAR* SharedLandscapeMaterialPath = TEXT("/Game/Namhansanseong/Materials/Landscape/MI_Landscape.MI_Landscape");

	void CreateFlatLandscape4x4()
	{
		UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
		if (!World)
		{
			UE_LOG(LogTemp, Error, TEXT("FLAT_LANDSCAPE_4X4 no editor world is open"));
			return;
		}

		for (TActorIterator<ALandscape> It(World); It; ++It)
		{
			if (It->Tags.Contains(FlatLandscapeTag))
			{
				UE_LOG(LogTemp, Display, TEXT("FLAT_LANDSCAPE_4X4 already exists: %s"), *It->GetPathName());
				return;
			}
		}

		const int32 QuadsPerComponent = QuadsPerSection * SectionsPerComponent;
		const int32 Resolution = ComponentCount * QuadsPerComponent + 1;
		const FVector Scale(100.0, 100.0, 100.0);
		const FVector SpawnLocation = AerialPlatformCenter - FVector(
			ComponentCount * QuadsPerComponent * Scale.X * 0.5,
			ComponentCount * QuadsPerComponent * Scale.Y * 0.5,
			0.0);

		const FScopedTransaction Transaction(LOCTEXT("CreateFlatLandscape4x4", "Create Flat Landscape 4x4"));
		ALandscape* Landscape = World->SpawnActor<ALandscape>(SpawnLocation, FRotator::ZeroRotator);
		if (!Landscape)
		{
			UE_LOG(LogTemp, Error, TEXT("FLAT_LANDSCAPE_4X4 could not spawn Landscape actor"));
			return;
		}

		Landscape->Modify();
		Landscape->SetActorScale3D(Scale);
		Landscape->MaxLODLevel = 2;
		Landscape->Tags.Add(FlatLandscapeTag);
		Landscape->SetActorLabel(TEXT("Landscape_Flat4x4_Aerial"));

		const FGuid ImportLayerGuid;
		TMap<FGuid, TArray<uint16>> HeightDataPerLayers;
		TArray<uint16>& HeightData = HeightDataPerLayers.Add(ImportLayerGuid);
		HeightData.Init(FlatHeight, Resolution * Resolution);
		TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayers;
		MaterialLayerDataPerLayers.Add(ImportLayerGuid);
		Landscape->Import(
			FGuid::NewGuid(),
			0,
			0,
			Resolution - 1,
			Resolution - 1,
			SectionsPerComponent,
			QuadsPerSection,
			HeightDataPerLayers,
			TEXT(""),
			MaterialLayerDataPerLayers,
			ELandscapeImportAlphamapType::Additive,
			TArrayView<const FLandscapeLayer>());
		Landscape->PostEditChange();
		Landscape->MarkPackageDirty();

		if (!FEditorFileUtils::SaveCurrentLevel())
		{
			UE_LOG(LogTemp, Error, TEXT("FLAT_LANDSCAPE_4X4 failed to save current level"));
			return;
		}

		UE_LOG(
			LogTemp,
			Display,
			TEXT("FLAT_LANDSCAPE_4X4 SUCCESS actor=%s components=%dx%d resolution=%dx%d center=%s"),
			*Landscape->GetPathName(),
			ComponentCount,
			ComponentCount,
			Resolution,
			Resolution,
			*AerialPlatformCenter.ToString());
	}

	void ApplyNearbyLandscapeMaterialToFlat4x4()
	{
		UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
		if (!World)
		{
			UE_LOG(LogTemp, Error, TEXT("FLAT_LANDSCAPE_MATERIAL no editor world is open"));
			return;
		}

		ALandscape* TargetLandscape = nullptr;
		for (TActorIterator<ALandscape> It(World); It; ++It)
		{
			if (It->Tags.Contains(FlatLandscapeTag))
			{
				TargetLandscape = *It;
				break;
			}
		}
		if (!TargetLandscape)
		{
			UE_LOG(LogTemp, Error, TEXT("FLAT_LANDSCAPE_MATERIAL target Landscape_Flat4x4_Aerial was not found"));
			return;
		}

		ALandscapeProxy* SourceLandscape = nullptr;
		TMap<ULandscapeLayerInfoObject*, int32> LayerUseCounts;
		for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
		{
			ALandscapeProxy* Candidate = *It;
			if (Candidate == TargetLandscape || !Candidate->LandscapeMaterial)
			{
				continue;
			}
			if (!SourceLandscape)
			{
				SourceLandscape = Candidate;
			}
			for (ULandscapeComponent* Component : Candidate->LandscapeComponents)
			{
				for (const FWeightmapLayerAllocationInfo& Allocation : Component->GetWeightmapLayerAllocations())
				{
					if (Allocation.LayerInfo)
					{
						LayerUseCounts.FindOrAdd(Allocation.LayerInfo)++;
					}
				}
			}
		}
		if (!SourceLandscape)
		{
			UE_LOG(LogTemp, Error, TEXT("FLAT_LANDSCAPE_MATERIAL no placed landscape material was found"));
			return;
		}

		ULandscapeLayerInfoObject* PrimaryLayer = nullptr;
		int32 HighestUseCount = INDEX_NONE;
		for (const TPair<ULandscapeLayerInfoObject*, int32>& Pair : LayerUseCounts)
		{
			if (Pair.Value > HighestUseCount)
			{
				PrimaryLayer = Pair.Key;
				HighestUseCount = Pair.Value;
			}
		}
		if (!PrimaryLayer)
		{
			UE_LOG(LogTemp, Error, TEXT("FLAT_LANDSCAPE_MATERIAL source landscape has no paint layers"));
			return;
		}

		const FScopedTransaction Transaction(LOCTEXT("ApplyFlatLandscapeMaterial", "Apply Nearby Landscape Material to Flat Landscape"));
		TargetLandscape->Modify();
		TargetLandscape->LandscapeMaterial = SourceLandscape->LandscapeMaterial;
		const FIntRect TargetRect = TargetLandscape->GetBoundingRect() + TargetLandscape->GetSectionBase();
		const int32 SampleCount = (TargetRect.Width() + 1) * (TargetRect.Height() + 1);
		TArray<uint8> PrimaryLayerWeights;
		PrimaryLayerWeights.Init(255, SampleCount);
		TAlphamapAccessor<false> WeightAccessor(TargetLandscape->GetLandscapeInfo(), PrimaryLayer);
		if (ULandscapeEditLayerBase* EditLayer = TargetLandscape->GetEditLayer(0))
		{
			WeightAccessor.SetEditLayer(EditLayer->GetGuid());
		}
		WeightAccessor.SetData(
			TargetRect.Min.X,
			TargetRect.Min.Y,
			TargetRect.Max.X,
			TargetRect.Max.Y,
			PrimaryLayerWeights.GetData(),
			ELandscapeLayerPaintingRestriction::None);
		TargetLandscape->RequestLayersContentUpdateForceAll(ELandscapeLayerUpdateMode::Update_All, true);
		TargetLandscape->ForceUpdateLayersContent();
		TargetLandscape->PostEditChange();
		TargetLandscape->MarkPackageDirty();

		if (!FEditorFileUtils::SaveCurrentLevel())
		{
			UE_LOG(LogTemp, Error, TEXT("FLAT_LANDSCAPE_MATERIAL could not save current level"));
			return;
		}
		UE_LOG(LogTemp, Display, TEXT("FLAT_LANDSCAPE_MATERIAL SUCCESS target=%s material=%s primary_layer=%s source=%s"),
			*TargetLandscape->GetPathName(),
			*GetPathNameSafe(TargetLandscape->LandscapeMaterial),
			*GetPathNameSafe(PrimaryLayer),
			*SourceLandscape->GetPathName());
	}

	FAutoConsoleCommand CreateFlatLandscape4x4Command(
		TEXT("Suwon.CreateFlatLandscape4x4"),
		TEXT("Creates a flat 4x4-component Landscape 1km above the current level origin."),
		FConsoleCommandDelegate::CreateStatic(&CreateFlatLandscape4x4));

	FAutoConsoleCommand ApplyNearbyLandscapeMaterialToFlat4x4Command(
		TEXT("Suwon.ApplyNearbyLandscapeMaterialToFlat4x4"),
		TEXT("Applies the placed landscape material and its dominant paint layer to Landscape_Flat4x4_Aerial."),
		FConsoleCommandDelegate::CreateStatic(&ApplyNearbyLandscapeMaterialToFlat4x4));
}

namespace SuwonGmarketSans
{
	constexpr const TCHAR* FontFacePath = TEXT("/Game/UI/Fonts/GmarketSansBold.GmarketSansBold");
	constexpr const TCHAR* FontPackagePath = TEXT("/Game/UI/Fonts/GmarketSansBold_Font");
	constexpr const TCHAR* FontAssetName = TEXT("GmarketSansBold_Font");

	void CreateRuntimeTitleFont()
	{
		UFontFace* FontFace = LoadObject<UFontFace>(nullptr, FontFacePath);
		if (!FontFace)
		{
			UE_LOG(LogTemp, Error, TEXT("GMARKET_SANS_TITLE_FONT missing FontFace: %s"), FontFacePath);
			return;
		}

		UFont* Font = LoadObject<UFont>(nullptr, *FString::Printf(TEXT("%s.%s"), FontPackagePath, FontAssetName));
		if (!Font)
		{
			UPackage* Package = CreatePackage(FontPackagePath);
			Font = NewObject<UFont>(Package, UFont::StaticClass(), FontAssetName, RF_Public | RF_Standalone);
			Font->FontCacheType = EFontCacheType::Runtime;
			FTypefaceEntry& TypefaceEntry = Font->GetMutableInternalCompositeFont().DefaultTypeface.Fonts.AddDefaulted_GetRef();
			TypefaceEntry.Name = TEXT("Default");
			TypefaceEntry.Font = FFontData(FontFace);
			FAssetRegistryModule::AssetCreated(Font);
			Font->MarkPackageDirty();
		}

		if (GEditor)
		{
			if (UEditorAssetSubsystem* AssetSubsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>())
			{
				AssetSubsystem->SaveLoadedAsset(Font, true);
			}
		}
		UE_LOG(LogTemp, Display, TEXT("GMARKET_SANS_TITLE_FONT SUCCESS font=%s face=%s"),
			*GetPathNameSafe(Font), *GetPathNameSafe(FontFace));
	}

	FAutoConsoleCommand CreateRuntimeTitleFontCommand(
		TEXT("Suwon.CreateGmarketSansTitleFont"),
		TEXT("Creates the Runtime UFont used by the Main intro title from its imported Gmarket Sans FontFace."),
		FConsoleCommandDelegate::CreateStatic(&CreateRuntimeTitleFont));
}

namespace SuwonNarrationImporter
{
	constexpr const TCHAR* NarrationFolder = TEXT("/GF_Singijeon/Asset/Narration");
	constexpr const TCHAR* NarrationTablePath = TEXT("/Game/Data/DT_Narration.DT_Narration");

	struct FNumberedSound
	{
		int32 Number = INDEX_NONE;
		FString Subtitle;
		USoundWave* Sound = nullptr;
	};

	bool ParseAssetName(const FString& AssetName, int32& OutNumber, FString& OutSubtitle)
	{
		int32 SeparatorIndex = INDEX_NONE;
		if (!AssetName.FindChar(TEXT('_'), SeparatorIndex) || SeparatorIndex <= 0)
		{
			return false;
		}

		const FString NumberText = AssetName.Left(SeparatorIndex);
		if (!NumberText.IsNumeric())
		{
			return false;
		}

		OutNumber = FCString::Atoi(*NumberText);
		if (OutNumber <= 0)
		{
			return false;
		}

		OutSubtitle = AssetName.Mid(SeparatorIndex + 1).Replace(TEXT("_"), TEXT(" "));
		OutSubtitle.TrimStartAndEndInline();
		return !OutSubtitle.IsEmpty();
	}

	bool ParseRowName(const FName RowName, int32& OutNumber)
	{
		const FString RowString = RowName.ToString();
		if (!RowString.StartsWith(TEXT("NA_")))
		{
			return false;
		}

		const FString NumberText = RowString.Mid(3);
		if (!NumberText.IsNumeric())
		{
			return false;
		}

		OutNumber = FCString::Atoi(*NumberText);
		return OutNumber > 0;
	}

	bool IsSupportedAsset(const FAssetData& Asset)
	{
		if (Asset.AssetClassPath != USoundWave::StaticClass()->GetClassPathName() ||
			!Asset.PackagePath.ToString().StartsWith(NarrationFolder))
		{
			return false;
		}

		int32 Number = INDEX_NONE;
		FString Subtitle;
		return ParseAssetName(Asset.AssetName.ToString(), Number, Subtitle);
	}

	void ShowNotification(const FText& Message, const SNotificationItem::ECompletionState State)
	{
		FNotificationInfo Info(Message);
		Info.ExpireDuration = 5.0f;
		Info.bUseLargeFont = false;
		if (const TSharedPtr<SNotificationItem> Item = FSlateNotificationManager::Get().AddNotification(Info))
		{
			Item->SetCompletionState(State);
		}
	}

	void RelinkNumberedRows(UDataTable* DataTable)
	{
		TArray<TPair<int32, FName>> NumberedRows;
		for (const TPair<FName, uint8*>& RowPair : DataTable->GetRowMap())
		{
			int32 Number = INDEX_NONE;
			if (ParseRowName(RowPair.Key, Number))
			{
				NumberedRows.Emplace(Number, RowPair.Key);
			}
		}

		NumberedRows.Sort([](const TPair<int32, FName>& A, const TPair<int32, FName>& B)
		{
			return A.Key < B.Key;
		});

		for (int32 Index = 0; Index < NumberedRows.Num(); ++Index)
		{
			const FName RowName = NumberedRows[Index].Value;
			const FNarrationSequenceRow* ExistingRow = DataTable->FindRow<FNarrationSequenceRow>(
				RowName, TEXT("RelinkNarrationRows"), false);
			if (!ExistingRow)
			{
				continue;
			}

			FNarrationSequenceRow UpdatedRow = *ExistingRow;
			const bool bIsLast = Index == NumberedRows.Num() - 1;
			UpdatedRow.NextRow = bIsLast ? NAME_None : NumberedRows[Index + 1].Value;
			UpdatedRow.AdvanceMode = bIsLast
				? ENarrationAdvanceMode::Stop
				: ENarrationAdvanceMode::Auto;
			DataTable->AddRow(RowName, UpdatedRow);
		}
	}

	void ImportSelectedAssets(const FToolMenuContext& MenuContext)
	{
		const UContentBrowserAssetContextMenuContext* Context =
			MenuContext.FindContext<UContentBrowserAssetContextMenuContext>();
		if (!Context)
		{
			return;
		}

		TArray<FNumberedSound> NumberedSounds;
		TSet<int32> UsedNumbers;
		for (const FAssetData& Asset : Context->SelectedAssets)
		{
			if (!IsSupportedAsset(Asset))
			{
				ShowNotification(
					LOCTEXT("InvalidNarrationSelection", "Narration 폴더의 번호_문장 형식 SoundWave만 선택할 수 있습니다."),
					SNotificationItem::CS_Fail);
				return;
			}

			FNumberedSound Entry;
			ParseAssetName(Asset.AssetName.ToString(), Entry.Number, Entry.Subtitle);
			if (UsedNumbers.Contains(Entry.Number))
			{
				ShowNotification(
					FText::Format(LOCTEXT("DuplicateNarrationNumber", "중복된 나레이션 번호가 있습니다: {0}"), Entry.Number),
					SNotificationItem::CS_Fail);
				return;
			}

			Entry.Sound = Cast<USoundWave>(Asset.GetAsset());
			if (!Entry.Sound)
			{
				ShowNotification(
					LOCTEXT("NarrationLoadFailed", "선택한 SoundWave를 불러오지 못했습니다."),
					SNotificationItem::CS_Fail);
				return;
			}

			UsedNumbers.Add(Entry.Number);
			NumberedSounds.Add(MoveTemp(Entry));
		}

		if (NumberedSounds.IsEmpty())
		{
			return;
		}

		NumberedSounds.Sort([](const FNumberedSound& A, const FNumberedSound& B)
		{
			return A.Number < B.Number;
		});

		UDataTable* DataTable = LoadObject<UDataTable>(nullptr, NarrationTablePath);
		if (!DataTable || DataTable->GetRowStruct() != FNarrationSequenceRow::StaticStruct())
		{
			ShowNotification(
				LOCTEXT("NarrationTableInvalid", "DT_Narration을 찾지 못했거나 Row Struct가 NarrationSequenceRow가 아닙니다."),
				SNotificationItem::CS_Fail);
			return;
		}

		const FScopedTransaction Transaction(LOCTEXT("ImportNarrationTransaction", "Import Narration Sounds"));
		DataTable->Modify();
		FDataTableEditorUtils::BroadcastPreChange(DataTable, FDataTableEditorUtils::EDataTableChangeInfo::RowList);

		int32 AddedCount = 0;
		int32 UpdatedCount = 0;
		for (const FNumberedSound& Entry : NumberedSounds)
		{
			const FName RowName(*FString::Printf(TEXT("NA_%02d"), Entry.Number));
			const FNarrationSequenceRow* ExistingRow = DataTable->FindRow<FNarrationSequenceRow>(
				RowName, TEXT("ImportNarrationSounds"), false);
			FNarrationSequenceRow UpdatedRow = ExistingRow ? *ExistingRow : FNarrationSequenceRow();
			UpdatedRow.NarrationSound = Entry.Sound;
			if (!ExistingRow || UpdatedRow.Subtitle.IsEmpty())
			{
				UpdatedRow.Subtitle = FText::FromString(Entry.Subtitle);
			}

			DataTable->AddRow(RowName, UpdatedRow);
			ExistingRow ? ++UpdatedCount : ++AddedCount;
		}

		RelinkNumberedRows(DataTable);
		DataTable->MarkPackageDirty();
		FDataTableEditorUtils::BroadcastPostChange(DataTable, FDataTableEditorUtils::EDataTableChangeInfo::RowList);

		bool bSaved = false;
		if (GEditor)
		{
			if (UEditorAssetSubsystem* AssetSubsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>())
			{
				bSaved = AssetSubsystem->SaveLoadedAsset(DataTable, true);
			}
		}

		ShowNotification(
			FText::Format(
				bSaved
					? LOCTEXT("NarrationImportSuccess", "DT_Narration 저장 완료: {0}개 추가, {1}개 갱신")
					: LOCTEXT("NarrationImportDirty", "DT_Narration 반영 완료: {0}개 추가, {1}개 갱신 (저장 필요)"),
				AddedCount,
				UpdatedCount),
			bSaved ? SNotificationItem::CS_Success : SNotificationItem::CS_Pending);
	}
}

void FSuwonSiegeContestVREditorModule::StartupModule()
{
	if (GUnrealEd)
	{
		ScenarioGuideVisualizer = MakeShared<FScenarioGuideComponentVisualizer>();
		GUnrealEd->RegisterComponentVisualizer(
			UScenarioInteractableComponent::StaticClass()->GetFName(),
			ScenarioGuideVisualizer);
		ScenarioGuideVisualizer->OnRegister();
	}

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(
			this, &FSuwonSiegeContestVREditorModule::RegisterMenus));
}

void FSuwonSiegeContestVREditorModule::ShutdownModule()
{
	if (GUnrealEd)
	{
		GUnrealEd->UnregisterComponentVisualizer(
			UScenarioInteractableComponent::StaticClass()->GetFName());
	}
	ScenarioGuideVisualizer.Reset();

	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
}

void FSuwonSiegeContestVREditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(TEXT("ContentBrowser.AssetContextMenu.SoundWave"));
	FToolMenuSection& Section = Menu->FindOrAddSection(TEXT("GetAssetActions"));
	Section.AddDynamicEntry(TEXT("AddToNarrationTable"), FNewToolMenuSectionDelegate::CreateLambda(
		[](FToolMenuSection& DynamicSection)
		{
			const UContentBrowserAssetContextMenuContext* Context =
				DynamicSection.FindContext<UContentBrowserAssetContextMenuContext>();
			if (!Context || !Context->bCanBeModified || Context->SelectedAssets.IsEmpty())
			{
				return;
			}

			for (const FAssetData& Asset : Context->SelectedAssets)
			{
				if (!SuwonNarrationImporter::IsSupportedAsset(Asset))
				{
					return;
				}
			}

			DynamicSection.AddMenuEntry(
				TEXT("AddToNarrationTable"),
				LOCTEXT("AddToNarrationTableLabel", "DT_Narration에 추가/갱신"),
				LOCTEXT("AddToNarrationTableTooltip", "파일명 번호로 NA_행을 만들고 음원과 순서를 DT_Narration에 연결합니다."),
				FSlateIcon(),
				FToolMenuExecuteAction::CreateStatic(&SuwonNarrationImporter::ImportSelectedAssets));
		}));
}

IMPLEMENT_MODULE(FSuwonSiegeContestVREditorModule, SuwonSiegeContestVREditor)

#undef LOCTEXT_NAMESPACE
