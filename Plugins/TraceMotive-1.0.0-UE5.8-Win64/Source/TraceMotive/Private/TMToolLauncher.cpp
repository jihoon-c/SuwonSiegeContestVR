#include "TMToolLauncher.h"
#include "TMStyle.h"

#include "TMAudioPlaybackTrace.h"
#include "TMBlueprintRuntimeErrorTrace.h"
#include "TMClassFavorites.h"
#include "TMClickDiagnostics.h"
#include "TMCollisionPairAnalyzer.h"
#include "TMContextShortcutHelper.h"
#include "TMDockTabHelper.h"
#include "TMEnhancedOutlinerSearch.h"
#include "TMGlobalSpeedControl.h"
#include "TMLocalization.h"
#include "TMPackageProgress.h"
#include "TMPluginGuide.h"
#include "TMVariableValueTrace.h"
#include "TMWidgetLifecycleTrace.h"
#include "TMWidgetClickFlowTrace.h"
#include "SInstanceReferenceTracker.h"

#include "Editor.h"
#include "Engine/Selection.h"
#include "GameFramework/Actor.h"
#include "Framework/Docking/TabManager.h"
#include "Styling/AppStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
    const FName ToolLauncherTabId(TEXT("TraceMotive.ToolLauncher"));
    bool bToolLauncherTabRegistered = false;
    TWeakPtr<SDockTab> ExistingToolLauncherTab;

    FText LauncherText(const TCHAR* English, const TCHAR* Korean = nullptr)
    {
        return TMLoc::Text(English, Korean ? Korean : English);
    }

    FSlateFontInfo LauncherFont(const FName Style, int32 Size)
    {
        return FCoreStyle::GetDefaultFontStyle(Style, Size);
    }

    AActor* GetFirstSelectedActor()
    {
        if (!GEditor)
        {
            return nullptr;
        }

        if (USelection* Selection = GEditor->GetSelectedActors())
        {
            for (FSelectionIterator It(*Selection); It; ++It)
            {
                if (AActor* Actor = Cast<AActor>(*It))
                {
                    return Actor;
                }
            }
        }
        return nullptr;
    }

    void OpenMessageWindow(const FString& Title, const FString& Body)
    {
        TMDockTab::OpenDockTab(
            TEXT("ToolLauncherMessage"),
            FText::FromString(Title),
            SNew(SBorder)
            .BorderImage(FAppStyle::GetBrush(TEXT("Brushes.Panel")))
            .Padding(16.0f)
            [
                SNew(STextBlock)
                .Text(FText::FromString(Body))
                .AutoWrapText(true)
            ]);
    }

    void OpenInstanceTraceForSelection()
    {
        if (AActor* Actor = GetFirstSelectedActor())
        {
            TMDockTab::OpenDockTab(
                TEXT("InstanceReferenceTrace"),
                FText::FromString(FString::Printf(TEXT("Instance Reference Trace - %s"), *Actor->GetActorLabel())),
                SNew(SInstanceReferenceTracker, Actor));
            return;
        }

        OpenMessageWindow(TEXT("Instance Reference Trace"), TEXT("Select an actor in the level, then open Instance Reference Trace again."));
    }

    void OpenVisualReferenceHelp()
    {
        OpenMessageWindow(
            TEXT("Visual Reference Search"),
            TEXT("Visual Reference Search is context-based. Open a Blueprint graph, right-click a variable/function/event/dispatcher, then choose Visual Find References or Visual Find Function References."));
    }

    struct FLauncherTile
    {
        FName Icon;
        FText Name;
        FText Desc;
        FLinearColor Color;
        TFunction<void()> Action;
    };

    const FSlateBrush* GetLauncherIcon(const FName IconName)
    {
        if (const ISlateStyle* Style = FSlateStyleRegistry::FindSlateStyle(TMStyle::GetStyleSetName()))
        {
            const FString LauncherIconName = IconName.ToString().Replace(TEXT("TraceMotive."), TEXT("TraceMotive.Launcher."));
            return Style->GetBrush(FName(*LauncherIconName));
        }
        return FAppStyle::GetBrush(TEXT("Icons.Help"));
    }

    TSharedRef<SWidget> BuildTile(const FLauncherTile& Tile)
    {
        return SNew(SBox)
            .WidthOverride(170.0f)
            .HeightOverride(146.0f)
            [
                SNew(SButton)
                .ButtonStyle(FAppStyle::Get(), TEXT("SimpleButton"))
                .OnClicked_Lambda([Action = Tile.Action]()
                {
                    if (Action)
                    {
                        Action();
                    }
                    return FReply::Handled();
                })
                [
                    SNew(SBorder)
                    .BorderImage(FAppStyle::GetBrush(TEXT("Brushes.Panel")))
                    .BorderBackgroundColor(FLinearColor(0.045f, 0.048f, 0.055f, 1.0f))
                    .Padding(10.0f)
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 6)
                        [
                            SNew(SBorder)
                            .BorderImage(FAppStyle::GetBrush(TEXT("WhiteBrush")))
                            .BorderBackgroundColor(Tile.Color.CopyWithNewOpacity(0.10f))
                            .Padding(9.0f)
                            [
                                SNew(SImage)
                                .Image(GetLauncherIcon(Tile.Icon))
                                .ColorAndOpacity(Tile.Color)
                            ]
                        ]
                        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 4)
                        [
                            SNew(SBox)
                            .HeightOverride(30.0f)
                            .VAlign(VAlign_Center)
                            [
                                SNew(STextBlock)
                                .Text(Tile.Name)
                                .Justification(ETextJustify::Center)
                                .AutoWrapText(true)
                                .WrapTextAt(138.0f)
                                .Font(LauncherFont(TEXT("Bold"), 10))
                            ]
                        ]
                        + SVerticalBox::Slot().FillHeight(1.0f).VAlign(VAlign_Top)
                        [
                            SNew(STextBlock)
                            .Text(Tile.Desc)
                            .Justification(ETextJustify::Center)
                            .AutoWrapText(true)
                            .Font(LauncherFont(TEXT("Regular"), 8))
                            .ColorAndOpacity(FLinearColor(0.68f, 0.72f, 0.78f, 1.0f))
                        ]
                    ]
                ]
            ];
    }

    TSharedRef<SWidget> BuildCategory(const FText& Title, const TArray<FLauncherTile>& Tiles)
    {
        TSharedRef<SWrapBox> Wrap = SNew(SWrapBox).UseAllottedSize(true);
        for (const FLauncherTile& Tile : Tiles)
        {
            Wrap->AddSlot().Padding(5.0f)[BuildTile(Tile)];
        }

        return SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 7)
            [
                SNew(STextBlock)
                .Text(Title)
                .Font(LauncherFont(TEXT("Bold"), 13))
                .ColorAndOpacity(FLinearColor(0.88f, 0.92f, 0.98f, 1.0f))
            ]
            + SVerticalBox::Slot().AutoHeight()
            [
                Wrap
            ];
    }

    class STMToolLauncherWidget : public SCompoundWidget
    {
    public:
        SLATE_BEGIN_ARGS(STMToolLauncherWidget) {}
        SLATE_END_ARGS()

        void Construct(const FArguments&)
        {
            ChildSlot
            [
                SAssignNew(ContentHost, SBox)
                [
                    BuildContent()
                ]
            ];
        }

    private:
        void SetLanguage(const bool bUseKorean)
        {
            TMLoc::SetUseKorean(bUseKorean);
            if (ContentHost.IsValid())
            {
                ContentHost->SetContent(BuildContent());
            }
        }

        TSharedRef<SWidget> BuildLanguageButton(const bool bForKorean)
        {
            return SNew(SCheckBox)
                .Type(ESlateCheckBoxType::ToggleButton)
                .Padding(0.0f)
                .ToolTipText(FText::FromString(bForKorean ? TEXT("Use Korean in TraceMotive") : TEXT("Use English in TraceMotive")))
                .IsChecked_Lambda([bForKorean]()
                {
                    return TMLoc::UseKorean() == bForKorean ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
                })
                .OnCheckStateChanged_Lambda([this, bForKorean](ECheckBoxState State)
                {
                    if (State == ECheckBoxState::Checked)
                    {
                        SetLanguage(bForKorean);
                    }
                })
                [
                    SNew(SBorder)
                    .BorderImage(FAppStyle::GetBrush(TEXT("WhiteBrush")))
                    .BorderBackgroundColor_Lambda([bForKorean]()
                    {
                        return TMLoc::UseKorean() == bForKorean
                            ? FLinearColor(0.18f, 0.50f, 0.82f, 0.82f)
                            : FLinearColor(0.11f, 0.12f, 0.14f, 0.90f);
                    })
                    .Padding(FMargin(10.0f, 4.0f))
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(bForKorean ? TEXT("한국어") : TEXT("EN")))
                        .Font(LauncherFont(TEXT("Bold"), 10))
                        .ColorAndOpacity_Lambda([bForKorean]()
                        {
                            return TMLoc::UseKorean() == bForKorean
                                ? FLinearColor::White
                                : FLinearColor(0.58f, 0.62f, 0.68f, 1.0f);
                        })
                    ]
                ]
                ;
        }

        TSharedRef<SWidget> BuildLanguageToggle()
        {
            return SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 6, 0)
                [
                    SNew(STextBlock)
                    .Text(LauncherText(TEXT("Language"), TEXT("언어")))
                    .Font(LauncherFont(TEXT("Regular"), 9))
                    .ColorAndOpacity(FSlateColor::UseSubduedForeground())
                ]
                + SHorizontalBox::Slot().AutoWidth()[BuildLanguageButton(false)]
                + SHorizontalBox::Slot().AutoWidth().Padding(3, 0, 0, 0)[BuildLanguageButton(true)];
        }

        TSharedRef<SWidget> BuildContent()
        {
            // A restrained category palette keeps the launcher readable as more tools are added.
            const FLinearColor SearchColor = TMStyle::GetSearchColor();
            const FLinearColor RuntimeColor = TMStyle::GetRuntimeColor();
            const FLinearColor WorkflowColor = TMStyle::GetWorkflowColor();
            const TArray<FLauncherTile> SearchTiles = {
                { TEXT("TraceMotive.VisualReferenceSearch"), LauncherText(TEXT("Visual Reference Search"), TEXT("시각 참조 검색")), LauncherText(TEXT("Find variable/function references from Blueprint context."), TEXT("Blueprint 컨텍스트에서 변수와 함수 참조를 찾습니다.")), SearchColor, [](){ OpenVisualReferenceHelp(); } },
                { TEXT("TraceMotive.EnhancedOutlinerSearch"), LauncherText(TEXT("Enhanced Outliner Search"), TEXT("향상된 아웃라이너 검색")), LauncherText(TEXT("Search actors by name, tag, variable, value, and actor arrays."), TEXT("이름, 태그, 변수, 값, 액터 배열로 액터를 검색합니다.")), SearchColor, [](){ TMEnhancedOutlinerSearch::OpenWindow(); } },
                { TEXT("TraceMotive.AssetUsageLocator"), LauncherText(TEXT("Asset Usage Locator"), TEXT("에셋 사용 위치")), LauncherText(TEXT("Exact asset usage is available from Content Browser context."), TEXT("콘텐츠 브라우저 컨텍스트에서 정확한 에셋 사용 위치를 찾습니다.")), SearchColor, [](){ OpenMessageWindow(TEXT("Asset Usage Locator"), TEXT("Select an asset in the Content Browser, right-click it, then choose Find Exact Blueprint Usage.")); } }
            };

            const TArray<FLauncherTile> RuntimeTiles = {
                { TEXT("TraceMotive.CollisionPairAnalyzer"), LauncherText(TEXT("Collision Pair Analyzer"), TEXT("충돌 쌍 분석")), LauncherText(TEXT("Analyze why two selected actors do or do not block."), TEXT("선택한 두 액터의 충돌 여부를 분석합니다.")), RuntimeColor, [](){ TMCollisionPairAnalyzer::OpenWindowAndAnalyzeSelection(); } },
                { TEXT("TraceMotive.ClickEventDiagnostics"), LauncherText(TEXT("Click Event Diagnostics"), TEXT("클릭 이벤트 진단")), LauncherText(TEXT("Trace UI, input, collision, and click delivery."), TEXT("UI, 입력, 충돌, 클릭 전달 과정을 추적합니다.")), RuntimeColor, [](){ TMClickDiagnostics::OpenWindow(); } },
                { TEXT("TraceMotive.AudioPlaybackTrace"), LauncherText(TEXT("Audio Playback Trace"), TEXT("오디오 재생 추적")), LauncherText(TEXT("Detect audio playback and likely source."), TEXT("오디오 재생과 추정 출처를 감지합니다.")), RuntimeColor, [](){ TMAudioPlaybackTrace::OpenWindow(); } },
                { TEXT("TraceMotive.VariableValueTrace"), LauncherText(TEXT("Variable Value Trace"), TEXT("변수 값 추적")), LauncherText(TEXT("Track selected class property changes during PIE."), TEXT("PIE 중 선택한 클래스 프로퍼티 변화를 추적합니다.")), RuntimeColor, [](){ TMVariableValueTrace::OpenWindow(); } },
                { TEXT("TraceMotive.WidgetLifecycleTrace"), LauncherText(TEXT("Widget Lifecycle Trace"), TEXT("위젯 생명주기 추적")), LauncherText(TEXT("Track live widgets and visibility changes."), TEXT("실행 중 위젯과 가시성 변화를 추적합니다.")), RuntimeColor, [](){ TMWidgetLifecycleTrace::OpenWindow(); } },
                { TEXT("TraceMotive.WidgetClickFlowTrace"), LauncherText(TEXT("Widget Click Flow Trace"), TEXT("위젯 클릭 흐름 추적")), LauncherText(TEXT("Capture the Blueprint flow after one PIE UMG click."), TEXT("PIE UMG 클릭 후 Blueprint 흐름을 캡처합니다.")), RuntimeColor, [](){ TMWidgetClickFlowTrace::OpenWindow(); } },
                { TEXT("TraceMotive.InstanceReferenceTracker"), LauncherText(TEXT("Instance Reference Trace"), TEXT("인스턴스 참조 추적")), LauncherText(TEXT("Trace selected actor references and runtime state."), TEXT("선택한 액터 참조와 런타임 상태를 추적합니다.")), RuntimeColor, [](){ OpenInstanceTraceForSelection(); } },
                { TEXT("TraceMotive.RuntimeErrorTrace"), LauncherText(TEXT("Runtime Error Investigation"), TEXT("런타임 오류 분석")), LauncherText(TEXT("Live Blueprint errors and completed PIE log analysis in one tool."), TEXT("실시간 Blueprint 오류와 완료된 PIE 로그를 한 도구에서 분석합니다.")), RuntimeColor, [](){ TMBlueprintRuntimeErrorTrace::OpenWindow(); } }
            };

            const TArray<FLauncherTile> WorkflowTiles = {
                { TEXT("TraceMotive.PackageProgress"), LauncherText(TEXT("Package Progress"), TEXT("패키징 진행")), LauncherText(TEXT("Readable packaging progress and failure hints."), TEXT("읽기 쉬운 패키징 진행 상황과 실패 힌트를 제공합니다.")), WorkflowColor, [](){ TMPackageProgress::OpenWindow(); } },
                { TEXT("TraceMotive.GlobalSpeedControl"), LauncherText(TEXT("Global Speed Control"), TEXT("전역 배속 제어")), LauncherText(TEXT("Time dilation, audio speed, and Skip to Target."), TEXT("시간 배율, 오디오 속도, 타깃까지 건너뛰기를 제어합니다.")), WorkflowColor, [](){ TMGlobalSpeedControl::OpenWindow(); } },
                { TEXT("TraceMotive.ContextShortcutGuide"), LauncherText(TEXT("Context Shortcut Guide"), TEXT("컨텍스트 단축키 안내")), LauncherText(TEXT("Show shortcuts for the clicked editor context."), TEXT("클릭한 에디터 컨텍스트의 단축키를 표시합니다.")), WorkflowColor, [](){ TMContextShortcutHelper::OpenWindow(); } },
                { TEXT("TraceMotive.ClassFavorites"), LauncherText(TEXT("Class Favorites"), TEXT("클래스 즐겨찾기")), LauncherText(TEXT("Favorite classes and quick viewport placement."), TEXT("클래스를 즐겨찾고 뷰포트에 빠르게 배치합니다.")), WorkflowColor, [](){ TMClassFavorites::OpenFavoritesWindow(); } },
                { TEXT("TraceMotive.PluginGuide"), LauncherText(TEXT("Plugin Guide"), TEXT("플러그인 안내")), LauncherText(TEXT("Open the full feature guide."), TEXT("전체 기능 안내를 엽니다.")), WorkflowColor, [](){ TMPluginGuide::OpenWindow(); } }
            };

            return SNew(SBorder)
                .BorderImage(FAppStyle::GetBrush(TEXT("Brushes.Panel")))
                .Padding(12.0f)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
                    [
                        SNew(STextBlock)
                        .Text(LauncherText(TEXT("TraceMotive — Debug Pathfinder for Unreal Remote")))
                        .AutoWrapText(true)
                        .Font(LauncherFont(TEXT("Bold"), 18))
                    ]
                    + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0, 0, 0, 4)
                    [
                        BuildLanguageToggle()
                    ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 10)
                    [
                        SNew(STextBlock)
                        .Text(LauncherText(TEXT("Launch TraceMotive tools by category. Context-only tools use the current selection or show a short instruction window."), TEXT("카테고리별 TraceMotive 도구를 실행합니다. 컨텍스트 전용 도구는 현재 선택을 사용하거나 짧은 안내 창을 표시합니다.")))
                        .AutoWrapText(true)
                        .ColorAndOpacity(FSlateColor::UseSubduedForeground())
                    ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
                    [SNew(SSeparator)]
                    + SVerticalBox::Slot().FillHeight(1.0f)
                    [
                        SNew(SScrollBox)
                        + SScrollBox::Slot().Padding(0, 0, 0, 14)[BuildCategory(LauncherText(TEXT("Search / Reference"), TEXT("검색 / 참조")), SearchTiles)]
                        + SScrollBox::Slot().Padding(0, 0, 0, 14)[BuildCategory(LauncherText(TEXT("Runtime Diagnostics"), TEXT("런타임 진단")), RuntimeTiles)]
                        + SScrollBox::Slot().Padding(0, 0, 0, 14)[BuildCategory(LauncherText(TEXT("Workflow / Utilities"), TEXT("워크플로 / 유틸리티")), WorkflowTiles)]
                    ]
                ];
        }

        TSharedPtr<SBox> ContentHost;
    };

    TSharedRef<SDockTab> SpawnToolLauncherTab(const FSpawnTabArgs&)
    {
        TSharedRef<SDockTab> Tab = SNew(SDockTab)
            .TabRole(ETabRole::NomadTab)
            .Label(LauncherText(TEXT("TraceMotive Remote")))
            .OnTabClosed(SDockTab::FOnTabClosedCallback::CreateLambda([](TSharedRef<SDockTab>) { ExistingToolLauncherTab.Reset(); }))
            [SNew(STMToolLauncherWidget)];
        ExistingToolLauncherTab = Tab;
        return Tab;
    }

    void RegisterToolLauncherTab()
    {
        if (bToolLauncherTabRegistered)
        {
            return;
        }

        FGlobalTabmanager::Get()->RegisterNomadTabSpawner(ToolLauncherTabId, FOnSpawnTab::CreateStatic(&SpawnToolLauncherTab))
            .SetDisplayName(LauncherText(TEXT("TraceMotive Remote")))
            .SetTooltipText(LauncherText(TEXT("Open the TraceMotive — Debug Pathfinder for Unreal tool launcher.")))
            .SetIcon(FSlateIcon(TMStyle::GetStyleSetName(), TEXT("TraceMotive.ToolLauncher")));
        bToolLauncherTabRegistered = true;
    }
}

namespace TMToolLauncher
{
    void RegisterMenus()
    {
        RegisterToolLauncherTab();

        auto AddEntry = [](UToolMenu* Menu, const FName EntryName)
        {
            if (!Menu)
            {
                return;
            }
            FToolMenuSection& Section = Menu->FindOrAddSection(TEXT("TraceMotive"));
            Section.AddMenuEntry(
                EntryName,
                LauncherText(TEXT("TraceMotive Remote")),
                LauncherText(TEXT("Open a categorized launcher for all TraceMotive — Debug Pathfinder for Unreal tools.")),
                FSlateIcon(TMStyle::GetStyleSetName(), TEXT("TraceMotive.ToolLauncher")),
                FToolMenuExecuteAction::CreateLambda([](const FToolMenuContext&) { TMToolLauncher::OpenWindow(); }));
        };

        AddEntry(UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Window")), TEXT("TMOpenToolLauncher"));
        AddEntry(UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Tools")), TEXT("TMToolsOpenToolLauncher"));
    }

    void UnregisterMenus()
    {
        if (bToolLauncherTabRegistered)
        {
            FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ToolLauncherTabId);
            bToolLauncherTabRegistered = false;
        }
        ExistingToolLauncherTab.Reset();
    }

    void OpenWindow()
    {
        RegisterToolLauncherTab();
        ExistingToolLauncherTab = FGlobalTabmanager::Get()->TryInvokeTab(ToolLauncherTabId);
    }
}


