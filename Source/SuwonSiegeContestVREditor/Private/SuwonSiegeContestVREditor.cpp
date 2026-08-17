#include "SuwonSiegeContestVREditor.h"

#include "ContentBrowserMenuContexts.h"
#include "Core/Narration/NarrationTypes.h"
#include "DataTableEditorUtils.h"
#include "Editor.h"
#include "Engine/DataTable.h"
#include "Sound/SoundWave.h"
#include "Subsystems/EditorAssetSubsystem.h"
#include "ToolMenus.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "ScopedTransaction.h"

#define LOCTEXT_NAMESPACE "SuwonSiegeContestVREditor"

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
	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(
			this, &FSuwonSiegeContestVREditorModule::RegisterMenus));
}

void FSuwonSiegeContestVREditorModule::ShutdownModule()
{
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
