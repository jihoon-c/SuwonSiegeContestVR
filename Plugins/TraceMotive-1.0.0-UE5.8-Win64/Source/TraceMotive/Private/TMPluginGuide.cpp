#include "TMPluginGuide.h"
#include "TMStyle.h"

#include "TMLocalization.h"
#include "Rendering/DrawElements.h"
#include "Framework/Docking/TabManager.h"
#include "Styling/AppStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
const FName PluginGuideTabId(TEXT("TraceMotive.PluginGuide"));
TWeakPtr<SDockTab> ExistingGuideTab;
bool bGuideTabRegistered = false;

FText GuideText(const TCHAR* Source)
{
    return TMLoc::Text(Source, Source);
}

FSlateFontInfo GuideFont(const FName Style, const int32 Size)
{
    return FCoreStyle::GetDefaultFontStyle(Style, Size);
}

const FSlateBrush* GetGuideIcon(const FName IconName)
{
    if (const ISlateStyle* Style = FSlateStyleRegistry::FindSlateStyle(TMStyle::GetStyleSetName()))
    {
        return Style->GetBrush(IconName);
    }
    return FAppStyle::GetBrush(TEXT("Icons.Help"));
}

struct FGuideFeature
{
    const TCHAR* Icon;
    const TCHAR* Name;
    const TCHAR* Purpose;
    const TCHAR* Scenario;
    const TCHAR* Steps;
    const TCHAR* Result;
    const TCHAR* Limitation;
    FLinearColor Color;
};

TArray<FGuideFeature> BuildGuideFeatures()
{
    return {
        { TEXT("TraceMotive.VisualReferenceSearch"), TEXT("Visual Reference Search / Call Chain"),
          TEXT("Find Blueprint and C++ references and follow the call path around a selected variable, function, event, or dispatcher."),
          TEXT("Example: an event changes a value at runtime, but you do not know which Blueprint or C++ caller reaches it."),
          TEXT("Open the graph context menu, choose reference or call-chain analysis, then use normal search or enable Search Boost for a faster high-budget scan."),
          TEXT("Read the chain from caller to target. Open a result to verify the actual graph or source location before changing code."),
          TEXT("Dynamic dispatch, reflection, interfaces, delegates, and runtime-created references can make a static chain incomplete."), FLinearColor(0.24f,0.58f,1.0f) },
        { TEXT("TraceMotive.EnhancedOutlinerSearch"), TEXT("Enhanced Outliner Search"),
          TEXT("Search level instances by label, class, tag, variable name, value, name/value pair, and actor references inside arrays."),
          TEXT("Example: find every placed actor whose variable 'DoorType' contains 'Security', even when the actor label is unrelated."),
          TEXT("Choose a search mode, enter the variable and value when using Pair Search, then enable Search Boost only when a broader scan is worth the editor cost."),
          TEXT("Each result shows the Outliner label and the matched field/value evidence. Select keeps the chosen result highlighted."),
          TEXT("Only reflected and safely readable instance data can be inspected; custom containers or generated runtime state may not be searchable."), FLinearColor(0.20f,0.72f,0.94f) },
        { TEXT("TraceMotive.AssetUsageLocator"), TEXT("Asset Usage Locator"),
          TEXT("Locate where an asset is used from the Content Browser context."),
          TEXT("Example: before replacing a material or texture, determine which assets or graphs depend on it."),
          TEXT("Select the asset in the Content Browser and run the usage/reference command from its context menu."),
          TEXT("Use the returned assets and locations as a review list, then open important matches for confirmation."),
          TEXT("Soft paths assembled at runtime and data loaded outside the asset registry may not appear."), FLinearColor(0.32f,0.64f,0.96f) },
        { TEXT("TraceMotive.CollisionPairAnalyzer"), TEXT("Collision Pair Analyzer"),
          TEXT("Compare two actors and explain configuration, shape-query, movement, Sweep, Teleport, and UpdatedComponent risks."),
          TEXT("Example: the pawn visually overlaps a volume, but gameplay either passes through it or stops despite the analyzer showing no current contact."),
          TEXT("Select A and B, run analysis, inspect the important component pair, then reproduce with Sweep enabled when the report asks for a runtime check."),
          TEXT("'Block capable' means the settings allow blocking; it does not prove a current Chaos contact. Follow the suggested component and movement-node checks."),
          TEXT("PhysicsOnly contact, custom movement, complex BodySetup, Teleport, and runtime Chaos state cannot always be concluded from static data."), FLinearColor(0.94f,0.34f,0.25f) },
        { TEXT("TraceMotive.ClickEventDiagnostics"), TEXT("Click Event Diagnostics"),
          TEXT("Trace a PIE click or touch through Slate, UMG, PlayerController, Enhanced/Legacy Input, visibility, and world collision."),
          TEXT("Example: a button or actor stopped receiving clicks after a new full-screen widget was added."),
          TEXT("Arm capture, click once in PIE, then inspect the pipeline and expand the full diagnostic steps. Copy the log when sharing the case."),
          TEXT("The first blocked stage is the strongest lead: UI consumption, input binding, controller setup, visibility, or hit-test collision."),
          TEXT("Platform input routing and custom Slate preprocessors can require manual confirmation beyond captured evidence."), FLinearColor(0.96f,0.68f,0.18f) },
        { TEXT("TraceMotive.AudioPlaybackTrace"), TEXT("Audio Playback Trace"),
          TEXT("Observe AudioComponent and fire-and-forget playback and estimate the closest owner or Blueprint source."),
          TEXT("Example: an unidentified sound repeatedly plays in PIE and no placed AudioComponent is obvious."),
          TEXT("Open the trace before reproducing, play the scene, filter the event list, and open Details on the suspicious sound."),
          TEXT("Use sound asset, owner, location, world, and confidence together; low-confidence source attribution is a lead rather than proof."),
          TEXT("Engine-native, middleware, pooled, or immediately destroyed fire-and-forget audio may not retain a definitive caller."), FLinearColor(0.22f,0.58f,0.95f) },
        { TEXT("TraceMotive.WidgetLifecycleTrace"), TEXT("Widget Lifecycle Trace"),
          TEXT("Track live UUserWidgets, creation/removal, child discovery, and meaningful Visibility transitions."),
          TEXT("Example: a health bar becomes Collapsed unexpectedly, or duplicate menu widgets remain alive after closing."),
          TEXT("Open the trace before reproduction, filter by widget/class, select an event, and use Details only when owner path and controller evidence are needed."),
          TEXT("Created, Opened, Visibility, Closed, and Destroyed annotations show the lifecycle without Tick/frame noise."),
          TEXT("A controller is inferred from captured runtime/script context; native or indirect changes can have lower confidence."), FLinearColor(0.34f,0.74f,1.0f) },
        { TEXT("TraceMotive.InstanceReferenceTracker"), TEXT("Instance Reference Trace"),
          TEXT("Watch a selected actor/component for meaningful reference, transform, visibility, and collision changes during PIE."),
          TEXT("Example: another actor changes a child collision component to NoCollision, but the responsible instance is unknown."),
          TEXT("Select the runtime instance, open the trace, choose the relevant category, then reproduce the change. Save a snapshot if comparison is needed."),
          TEXT("A collision event should show the changed property and available caller/controller evidence without Actor Tick spam."),
          TEXT("Very short changes between samples or native writes without script context may identify the change but not the exact writer."), FLinearColor(0.25f,0.78f,0.45f) },
        { TEXT("TraceMotive.RuntimeErrorTrace"), TEXT("Runtime Error Investigation"),
          TEXT("One investigation workspace for live Blueprint error capture and completed PIE log analysis."),
          TEXT("Example: use Live Trace while reproducing an Accessed None error, or inspect Completed PIE Logs when the trace was not open."),
          TEXT("Open the tool. Use Live Trace before reproduction; after PIE ends, use Completed PIE Logs to recover class, function, node, and instance clues from the log."),
          TEXT("Live Trace can retain live-object evidence; completed logs clearly label instance/function recovery as inferred when the log lacks an exact object path."),
          TEXT("Destroyed objects and omitted log context cannot be reconstructed. Confirm inferred matches in the Blueprint before changing code."), FLinearColor(0.96f,0.27f,0.22f) },
        { TEXT("TraceMotive.PackageProgress"), TEXT("Package Progress"),
          TEXT("Present UAT packaging as readable stages with current-stage and monotonic overall progress, elapsed time, and failure hints."),
          TEXT("Example: packaging appears stuck during Cooking, or fails with a class/compiler message hidden in a long log."),
          TEXT("Start packaging normally. Watch the stage bar and overall bar; on failure, inspect the summarized cause and use the available source navigation action."),
          TEXT("Stage progress may reset for a new phase, while overall progress must never move backward during one packaging run."),
          TEXT("UAT does not expose one universal percentage; overall progress is an estimate based on detected phases and work counts."), FLinearColor(0.20f,0.66f,0.93f) },
        { TEXT("TraceMotive.GlobalSpeedControl"), TEXT("Global Speed Control"),
          TEXT("Apply game time dilation and audio pitch speed together, with Skip to Target support."),
          TEXT("Example: fast-forward a long sequence, then return to 1x and pause when a log phrase or selected class event occurs."),
          TEXT("Choose 0.1x-20x on the slider. Optionally arm Skip to Target with a log substring or class, then reproduce in PIE."),
          TEXT("The integer ruler shows each speed unit; a matched target forces 1x, restores tracked audio pitch, and pauses the game."),
          TEXT("High speed increases CPU/Tick/physics/audio load. Media, networking, custom clocks, and some native timers may not follow exactly."), FLinearColor(0.36f,0.84f,0.36f) },
        { TEXT("TraceMotive.ContextShortcutGuide"), TEXT("Context Shortcut Guide"),
          TEXT("Show shortcuts for the editor area activated by a mouse click, including Material and generic graph editors."),
          TEXT("Example: click the Material graph canvas to see node creation, wire editing, navigation, bookmark, and edit shortcuts."),
          TEXT("Open the guide and click once inside the editor area whose commands you want. Expand only the shortcut categories you need."),
          TEXT("The active context remains based on the last click rather than changing from mouse hover."),
          TEXT("Plugin-defined or project-specific command bindings may differ from the built-in reference."), FLinearColor(0.70f,0.52f,1.0f) },
        { TEXT("TraceMotive.ClassFavorites"), TEXT("Class Favorites"),
          TEXT("Keep frequently used Blueprint classes and levels in a compact dockable palette."),
          TEXT("Example: repeatedly open or place the same debug actors while iterating on a level."),
          TEXT("Add assets from the Content Browser, then open, place, reorder, search, or drag them into the viewport."),
          TEXT("Compact mode preserves recognizable icons even when the panel is extremely narrow."),
          TEXT("Removing an entry only removes the favorite; it does not delete the asset."), FLinearColor(1.0f,0.76f,0.18f) },
        { TEXT("TraceMotive.SkipToTarget"), TEXT("Viewport Move Helpers"),
          TEXT("Move a selected actor or component to the active viewport camera location."),
          TEXT("Example: place a debug volume or effect component exactly where the level designer is currently looking."),
          TEXT("Select the target, open its context command, then choose whole-actor or selected-component movement."),
          TEXT("The selection cache helps preserve the intended target when context-menu focus changes."),
          TEXT("Review attachment and relative-transform consequences before moving child components."), FLinearColor(0.18f,0.72f,0.38f) }
    };
}

struct FGuidePreview
{
    const TCHAR* Context;
    const TCHAR* Header;
    const TCHAR* Status;
    const TCHAR* FirstResult;
    const TCHAR* SecondResult;
    const TCHAR* Action;
    const TCHAR* ContextNote;
    const TCHAR* StatusNote;
    const TCHAR* ResultNote;
    const TCHAR* ActionNote;
};

FGuidePreview PreviewForFeature(const TCHAR* Name)
{
    const FString Key(Name);
    if (Key == TEXT("Visual Reference Search / Call Chain")) return { TEXT("Target: BP_Door.OpenDoor"), TEXT("Function Call Chain"), TEXT("Search complete | 18 assets | 7 call paths"), TEXT("[Blueprint] BP_Keypad.OnAccepted -> OpenDoor"), TEXT("[C++] ADoorTrigger::NotifyActorBeginOverlap -> OpenDoor"), TEXT("Open selected node"), TEXT("The analyzed symbol and scan mode are shown here."), TEXT("Asset count, call-path count, and incomplete-scan warnings prevent silent assumptions."), TEXT("Blueprint and C++ results display the caller and target path, not only an asset name."), TEXT("Open the exact graph node or source location to verify the inferred chain.") };
    if (Key == TEXT("Enhanced Outliner Search")) return { TEXT("Pair: DoorType = Security"), TEXT("Enhanced Outliner Search"), TEXT("12 / 846 actors matched | Search Boost: Off"), TEXT("SecurityDoor_Lobby | DoorType = Security"), TEXT("SecurityDoor_B2 | DoorType = Security | Tags: Locked"), TEXT("Select actor"), TEXT("The active name/tag/variable/value/pair query remains visible."), TEXT("Scanned and matched counts show whether the search is still partial."), TEXT("Every result includes the exact matched field and value evidence."), TEXT("Select focuses the Outliner label and keeps this result highlighted.") };
    if (Key == TEXT("Asset Usage Locator")) return { TEXT("Asset: M_Master_Wall"), TEXT("Asset Usage Locator"), TEXT("31 references in 14 assets"), TEXT("SM_Wall_A.Materials[0] -> M_Master_Wall"), TEXT("BP_Building.DefaultWallMaterial -> M_Master_Wall"), TEXT("Browse to reference"), TEXT("The Content Browser asset selected for analysis is fixed here."), TEXT("Reference and asset totals indicate the scope of the result."), TEXT("The owning asset and property/graph location explain how it is used."), TEXT("Browse opens the concrete reference for manual confirmation.") };
    if (Key == TEXT("Collision Pair Analyzer")) return { TEXT("A: BP_MainPawn | B: BlockingVolume9"), TEXT("Collision Pair Analyzer"), TEXT("Block capable: 1 | Current shape contact: 0 | Confidence: Medium"), TEXT("CapsuleComponent <-> BrushComponent0 | BLOCK CAPABLE"), TEXT("Movement check: Sweep/Teleport/UpdatedComponent requires runtime verification"), TEXT("Show details / Find movement node"), TEXT("The exact A/B actors can be replaced without reopening the window."), TEXT("Configuration capability and actual contact are deliberately reported separately."), TEXT("Important component pairs and the likely movement-side cause stay visible; auxiliary pairs are hidden."), TEXT("Details reveals channels, BodySetup, Chaos limitations, and suspected node locations.") };
    if (Key == TEXT("Click Event Diagnostics")) return { TEXT("Armed | Click anywhere in PIE"), TEXT("Click Event Diagnostics"), TEXT("Last click (612, 348) | Consumed by UI"), TEXT("Slate hit test -> WBP_PauseMenu (Visible, blocking)"), TEXT("World actor -> Not reached | Bound event -> Not reached"), TEXT("Copy log / View full diagnostic steps"), TEXT("Capture state confirms whether the next click/touch will be analyzed."), TEXT("The final delivery outcome is summarized without requiring raw logs."), TEXT("The pipeline shows the exact stage that consumed or lost the input."), TEXT("Copy shares evidence; full steps exposes UMG, input, controller, visibility, and collision checks.") };
    if (Key == TEXT("Audio Playback Trace")) return { TEXT("PIE World: Lv_Menu | Filter: All playback"), TEXT("Audio Playback Trace"), TEXT("24 events | 3 active sounds"), TEXT("S_UI_Confirm | WBP_MainMenu.Button_Play | Confidence: High"), TEXT("S_Ambient_Wind | Fire-and-forget near BP_WindZone | Confidence: Medium"), TEXT("Details / Copy report"), TEXT("World and filters prevent unrelated editor-preview audio from mixing into the trace."), TEXT("Active and historical event counts reveal repeated or overlapping playback."), TEXT("Sound, owner/source candidate, playback type, and confidence appear together."), TEXT("Details exposes location and evidence; the report preserves recent playback context.") };
    if (Key == TEXT("Widget Lifecycle Trace")) return { TEXT("Filter: WBP_ | Source: All"), TEXT("Widget Lifecycle Trace"), TEXT("4 live UserWidgets | Real-time"), TEXT("Visibility | Image_HealthBar | Visible -> Collapsed"), TEXT("Destroyed | WBP_DamageNumber | GC teardown detected"), TEXT("Details / Copy report"), TEXT("Widget/class and source filters keep large PIE sessions readable."), TEXT("The live-widget count is separate from lifecycle-event history."), TEXT("Meaningful lifecycle/visibility transitions appear without Tick or frame noise."), TEXT("Details shows owner path, controller evidence, timestamp, and confidence only when requested.") };
    if (Key == TEXT("Instance Reference Trace")) return { TEXT("Target: IA_Click_Middle.InteractingCollision"), TEXT("Instance Reference Trace"), TEXT("Collision events: 1 | Ignored Tick events: 438"), TEXT("CollisionEnabled: QueryOnly -> NoCollision"), TEXT("Controller candidate: BP_DoorManager_C_2 | Confidence: ActiveScriptContext"), TEXT("Save snapshot / Select controller"), TEXT("The exact live actor/component target remains pinned while inspecting results."), TEXT("Ignored Tick/frame updates are excluded from both the list and meaningful-change count."), TEXT("The changed collision property and best available writer evidence are paired."), TEXT("Snapshot supports before/after comparison; Select jumps to the candidate instance.") };
    if (Key == TEXT("Runtime Error Investigation")) return { TEXT("Live Trace + Completed PIE Logs"), TEXT("Runtime Error Investigation"), TEXT("3 live errors | 8 completed-log findings | 1 exact instance match"), TEXT("Accessed None: CallFunc_GetPlayerCharacter | BP_Door.Open"), TEXT("Live: BP_Door_Lobby (exact) | Log: WBP_Inventory_C_3 (inferred)"), TEXT("Select instance / Browse node / Copy evidence"), TEXT("Live Trace listens only while this window is open; Completed PIE Logs fills the gap after a missed session."), TEXT("The source of each result states whether it is live evidence or completed-log inference."), TEXT("Class, function/node, instance evidence, and confidence remain together across both sources."), TEXT("Use Select/Browse for live objects; verify log-derived candidates in the Blueprint.") };
    if (Key == TEXT("Package Progress")) return { TEXT("Windows Development"), TEXT("Package Progress"), TEXT("Cooking | Stage 63% | Overall 48% | Elapsed 08:42"), TEXT("Preparing [Done]  Building Code [Done]  Cooking [Now]"), TEXT("Estimated remaining 09:17 | 684 / 1081 packages"), TEXT("Failure details / Open class"), TEXT("Platform/configuration identify the packaging job being measured."), TEXT("Current-stage progress can reset; monotonic overall progress never moves backward."), TEXT("Completed checks, current stage, work counts, elapsed time, and estimate explain apparent stalls."), TEXT("After failure, summarized causes and source navigation replace manual log hunting.") };
    if (Key == TEXT("Global Speed Control")) return { TEXT("PIE World | Audio tracking enabled"), TEXT("Global Speed Control"), TEXT("Speed 5.0x | Worlds 1 | Audio tracked 7"), TEXT("0.1  1  2  3  4  [5]  6 ... 20"), TEXT("Skip target armed: Log contains 'BossPhase2'"), TEXT("Reset to 1x / Pause on target"), TEXT("The affected world and audio-tracking state are visible before acceleration."), TEXT("Applied gameplay/audio speed and tracked-object count confirm current state."), TEXT("The responsive ruler displays every integer unit and the active speed; skip conditions remain visible."), TEXT("Reset restores time and audio pitch; a matched target restores 1x and pauses automatically.") };
    if (Key == TEXT("Context Shortcut Guide")) return { TEXT("Clicked context: Material Graph"), TEXT("Context Shortcut Guide"), TEXT("Material Graph shortcuts | 5 categories"), TEXT("Node creation: 1/2/3/4, A, B, D, F, I, L, M, N, O, P, R, S, T, U, V + Click"), TEXT("Wire editing: Alt+Pin Click | Wire Double Click | Graph: F, Home"), TEXT("Expand category / Search shortcut"), TEXT("The last clicked editor area, not mouse hover, chooses the shortcut set."), TEXT("The recognized context and category count show whether a specialized guide was selected."), TEXT("Material-specific and generic graph shortcuts are grouped instead of mixed into one list."), TEXT("Expand only relevant groups or search by key/function when the full list is long.") };
    if (Key == TEXT("Class Favorites")) return { TEXT("Favorites: 9 | Compact layout"), TEXT("Class Favorites"), TEXT("Search: Skeletal | 4 matches"), TEXT("BP_MultipleSkeleton | Blueprint Class"), TEXT("SK_Mannequin | Skeletal Mesh"), TEXT("Open / Place / Remove favorite"), TEXT("Favorite count and compact mode explain the current palette."), TEXT("The active search and match count remain readable even in a narrow dock."), TEXT("Icon plus asset label/type preserve identity; compact mode prioritizes the icon."), TEXT("Actions open/place the asset; Remove affects only the favorite entry.") };
    if (Key == TEXT("Viewport Move Helpers")) return { TEXT("Selected: BP_DebugVolume.BoxComponent"), TEXT("Viewport Move Helpers"), TEXT("Active viewport camera: Perspective"), TEXT("Move actor to camera | BP_DebugVolume"), TEXT("Move selected component to camera | BoxComponent"), TEXT("Apply movement"), TEXT("The cached actor/component selection prevents context-menu focus from changing the target."), TEXT("The active viewport camera used as the destination is stated explicitly."), TEXT("Actor movement and component-relative movement are separate choices."), TEXT("Apply performs the chosen operation; review attachments and relative transforms afterward.") };
    return { TEXT("Selected feature"), TEXT("Feature Preview"), TEXT("Ready"), TEXT("Primary result"), TEXT("Secondary evidence"), TEXT("Open details"), TEXT("Shows the active target or filter."), TEXT("Summarizes current diagnostic state."), TEXT("Shows the evidence produced by the feature."), TEXT("Opens the next verification action.") };
}

TSharedRef<SWidget> PreviewLine(const FText& Text, const bool bStrong = false)
{
    return SNew(SBorder)
        .BorderImage(FAppStyle::GetBrush(TEXT("Brushes.Recessed")))
        .Padding(8.0f)
        [
            SNew(STextBlock).Text(Text).AutoWrapText(true).Font(GuideFont(bStrong ? TEXT("Bold") : TEXT("Regular"), 9))
        ];
}

struct FGuideCallout
{
    FString Number;
    FText Title;
    FText Note;
    FVector2D LabelPosition;
    FVector2D Anchor;
    bool bLabelOnLeft;
};

class SGuideCalloutOverlay : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SGuideCalloutOverlay) {}
        SLATE_DEFAULT_SLOT(FArguments, Content)
        SLATE_ARGUMENT(FGuidePreview, Preview)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs)
    {
        Preview = InArgs._Preview;
        ChildSlot
        [
            SNew(SBox)
            .MinDesiredHeight(390.0f)
            .Padding(FMargin(182.0f, 20.0f, 182.0f, 20.0f))
            [
                InArgs._Content.Widget
            ]
        ];
    }

    void SetAnnotationsVisible(const bool bInVisible)
    {
        bAnnotationsVisible = bInVisible;
        Invalidate(EInvalidateWidgetReason::Paint);
    }

    virtual int32 OnPaint(
        const FPaintArgs& Args,
        const FGeometry& AllottedGeometry,
        const FSlateRect& MyCullingRect,
        FSlateWindowElementList& OutDrawElements,
        int32 LayerId,
        const FWidgetStyle& InWidgetStyle,
        bool bParentEnabled) const override
    {
        const int32 ChildLayer = SCompoundWidget::OnPaint(
            Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
        if (!bAnnotationsVisible)
        {
            return ChildLayer;
        }

        const FVector2D Size = AllottedGeometry.GetLocalSize();
        if (Size.X < 610.0f || Size.Y < 300.0f)
        {
            return ChildLayer;
        }

        const float RightX = Size.X - 171.0f;
        const TArray<FGuideCallout> Callouts = {
            { TEXT("1"), GuideText(TEXT("Target / context")), GuideText(Preview.ContextNote), FVector2D(8.0f, 44.0f), FVector2D(0.30f, 0.27f), true },
            { TEXT("2"), GuideText(TEXT("Diagnostic summary")), GuideText(Preview.StatusNote), FVector2D(8.0f, 222.0f), FVector2D(0.30f, 0.43f), true },
            { TEXT("3"), GuideText(TEXT("Evidence")), GuideText(Preview.ResultNote), FVector2D(RightX, 78.0f), FVector2D(0.70f, 0.62f), false },
            { TEXT("4"), GuideText(TEXT("Next action")), GuideText(Preview.ActionNote), FVector2D(RightX, 255.0f), FVector2D(0.70f, 0.79f), false }
        };

        int32 PaintLayer = ChildLayer + 2;
        for (const FGuideCallout& Callout : Callouts)
        {
            DrawCallout(AllottedGeometry, OutDrawElements, PaintLayer, Callout);
            PaintLayer += 3;
        }
        return PaintLayer;
    }

private:
    void DrawCallout(
        const FGeometry& Geometry,
        FSlateWindowElementList& OutDrawElements,
        const int32 Layer,
        const FGuideCallout& Callout) const
    {
        const FVector2D Size = Geometry.GetLocalSize();
        const FVector2D Anchor(
            182.0f + (Size.X - 364.0f) * Callout.Anchor.X,
            20.0f + (Size.Y - 40.0f) * Callout.Anchor.Y);
        const FVector2D CircleCenter = Callout.LabelPosition + FVector2D(9.0f, 9.0f);
        const FVector2D TextPosition = Callout.LabelPosition + FVector2D(22.0f, -1.0f);
        const FVector2D LineStart = Callout.bLabelOnLeft
            ? FVector2D(171.0f, Callout.LabelPosition.Y + 13.0f)
            : FVector2D(Size.X - 180.0f, Callout.LabelPosition.Y + 13.0f);
        const FVector2D Bend(
            Callout.bLabelOnLeft ? Anchor.X - 34.0f : Anchor.X + 34.0f,
            Anchor.Y);

        TArray<FVector2D> ArrowPoints = { LineStart, Bend, Anchor };
        FSlateDrawElement::MakeLines(
            OutDrawElements, Layer, Geometry.ToPaintGeometry(), ArrowPoints,
            ESlateDrawEffect::None, AnnotationColor, true, 2.0f);

        const FVector2D Direction = (Bend - Anchor).GetSafeNormal();
        const FVector2D Normal(-Direction.Y, Direction.X);
        TArray<FVector2D> ArrowHead = {
            Anchor + Direction * 11.0f + Normal * 5.0f,
            Anchor,
            Anchor + Direction * 11.0f - Normal * 5.0f
        };
        FSlateDrawElement::MakeLines(
            OutDrawElements, Layer + 1, Geometry.ToPaintGeometry(), ArrowHead,
            ESlateDrawEffect::None, AnnotationColor, true, 2.0f);

        TArray<FVector2D> CirclePoints;
        constexpr int32 CircleSegments = 18;
        for (int32 Index = 0; Index <= CircleSegments; ++Index)
        {
            const float Angle = 2.0f * PI * static_cast<float>(Index) / static_cast<float>(CircleSegments);
            CirclePoints.Add(CircleCenter + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * 9.0f);
        }
        FSlateDrawElement::MakeLines(
            OutDrawElements, Layer + 1, Geometry.ToPaintGeometry(), CirclePoints,
            ESlateDrawEffect::None, AnnotationColor, true, 1.8f);

        FSlateDrawElement::MakeText(
            OutDrawElements, Layer + 2,
            Geometry.ToPaintGeometry(FVector2D(18.0f, 18.0f), FSlateLayoutTransform(Callout.LabelPosition + FVector2D(5.0f, -1.0f))),
            Callout.Number, GuideFont(TEXT("Bold"), 9), ESlateDrawEffect::None, AnnotationColor);
        FSlateDrawElement::MakeText(
            OutDrawElements, Layer + 2,
            Geometry.ToPaintGeometry(FVector2D(145.0f, 18.0f), FSlateLayoutTransform(TextPosition)),
            Callout.Title, GuideFont(TEXT("Bold"), 9), ESlateDrawEffect::None, AnnotationColor);

        const FString NoteString = Callout.Note.ToString();
        const FString ShortNote = NoteString.Len() > 42 ? NoteString.Left(39) + TEXT("...") : NoteString;
        FSlateDrawElement::MakeText(
            OutDrawElements, Layer + 2,
            Geometry.ToPaintGeometry(FVector2D(149.0f, 18.0f), FSlateLayoutTransform(TextPosition + FVector2D(0.0f, 18.0f))),
            ShortNote, GuideFont(TEXT("Regular"), 7), ESlateDrawEffect::None,
            FLinearColor(AnnotationColor.R, AnnotationColor.G, AnnotationColor.B, 0.88f));
    }

    FGuidePreview Preview;
    bool bAnnotationsVisible = true;
    const FLinearColor AnnotationColor = FLinearColor(1.0f, 0.72f, 0.05f, 1.0f);
};

TSharedRef<SWidget> BuildAnnotatedPreview(const FGuideFeature& Feature)
{
    const FGuidePreview Preview = PreviewForFeature(Feature.Name);
    TSharedRef<SWidget> ExampleWidget =
        SNew(SBorder).BorderImage(FAppStyle::GetBrush(TEXT("Brushes.Panel"))).Padding(10.0f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 8, 0)
                [SNew(SImage).Image(GetGuideIcon(Feature.Icon)).ColorAndOpacity(Feature.Color)]
                + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
                [SNew(STextBlock).Text(GuideText(Preview.Header)).Font(GuideFont(TEXT("Bold"), 12))]
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 6)[PreviewLine(GuideText(Preview.Context))]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 6)[PreviewLine(GuideText(Preview.Status), true)]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)[PreviewLine(GuideText(Preview.FirstResult))]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 6)[PreviewLine(GuideText(Preview.SecondResult))]
            + SVerticalBox::Slot().AutoHeight()[PreviewLine(GuideText(Preview.Action), true)]
        ];

    TSharedRef<SGuideCalloutOverlay> OverlayWidget =
        SNew(SGuideCalloutOverlay)
        .Preview(Preview)
        [ExampleWidget];
    const TWeakPtr<SGuideCalloutOverlay> WeakOverlay = OverlayWidget;

    return SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 6)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()
                [SNew(STextBlock).Text(GuideText(TEXT("Annotated live Slate example"))).Font(GuideFont(TEXT("Bold"), 11))]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 2, 0, 0)
                [SNew(STextBlock)
                    .Text(GuideText(TEXT("The sample actors and values are illustrative. The yellow callouts are painted in a separate non-interactive overlay.")))
                    .AutoWrapText(true).Font(GuideFont(TEXT("Regular"), 8)).ColorAndOpacity(FSlateColor::UseSubduedForeground())]
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(12, 0, 0, 0)
            [
                SNew(SCheckBox)
                .IsChecked(ECheckBoxState::Checked)
                .OnCheckStateChanged_Lambda([WeakOverlay](ECheckBoxState NewState)
                {
                    if (TSharedPtr<SGuideCalloutOverlay> Pinned = WeakOverlay.Pin())
                    {
                        Pinned->SetAnnotationsVisible(NewState == ECheckBoxState::Checked);
                    }
                })
                [SNew(STextBlock).Text(GuideText(TEXT("Show callouts")))]
            ]
        ]
        + SVerticalBox::Slot().AutoHeight()[OverlayWidget];
}

enum class EGuideDetailSection : uint8
{
    Example,
    Usage,
    Result,
    Limit
};

FText DetailedAnnotationText(const FGuideFeature& Feature, const EGuideDetailSection Section)
{
    FString Text;
    switch (Section)
    {
    case EGuideDetailSection::Example:
        Text = FString::Printf(
            TEXT("Situation\n%s\n\nWhat you are trying to learn\n%s\n\nBefore reproducing\nIdentify the exact asset, actor, component, widget, session, or graph involved. Keep the relevant TM tab open before the issue occurs when the feature depends on runtime capture. Reproduce one controlled case first so unrelated events do not dilute the evidence."),
            Feature.Scenario, Feature.Purpose);
        break;

    case EGuideDetailSection::Usage:
        Text = FString::Printf(
            TEXT("1. Set the scope\nChoose the target, A/B pair, asset, search field, PIE session, or capture filter shown in the annotated preview.\n\n2. Run or arm the feature\n%s\n\n3. Reproduce once\nTrigger the smallest action that demonstrates the problem. Avoid repeatedly clicking or restarting while capture is active unless repetition itself is the issue.\n\n4. Narrow the evidence\nStart from the visible summary and strongest result. Enable Search Boost, full details, raw logs, or broader scanning only when the initial result is insufficient.\n\n5. Verify at the source\nUse Select, Browse, Open Node, or Details to inspect the actual instance, Blueprint graph, C++ source, property, or log location before applying a fix."),
            Feature.Steps);
        break;

    case EGuideDetailSection::Result:
        Text = FString::Printf(
            TEXT("Primary interpretation\n%s\n\nEvidence priority\nPrefer exact runtime object paths, captured property transitions, direct graph/source locations, and confirmed engine responses. Treat class-only matches, nearest-owner matches, static call paths, and heuristic controller attribution as candidates.\n\nConfidence and coverage\nCheck confidence, scanned/total counts, filters, ignored-event counts, and limitation badges. A clean result from a partial scan does not prove that no cause exists.\n\nRecommended confirmation\nOpen the strongest result and reproduce again while watching the referenced property, node, component, input stage, package phase, or runtime instance. The report should guide confirmation rather than replace it."),
            Feature.Result);
        break;

    case EGuideDetailSection::Limit:
        Text = FString::Printf(
            TEXT("Known limitation\n%s\n\nDo not over-interpret\n'Not detected' does not always mean 'did not happen'. Static analysis cannot fully reconstruct runtime dispatch, and sampled runtime observation can miss state that changes and returns between samples. Estimated progress and inferred sources are not engine guarantees.\n\nManual fallback\nReproduce with a smaller target set, turn on the relevant engine log or debugger breakpoint, inspect the actual Blueprint/C++ location, and compare the before/after runtime state. For collision use Sweep/FHitResult or Chaos inspection; for input inspect the captured Slate/UMG/controller pipeline; for packaging confirm the cited UAT lines.\n\nPerformance caution\nUse normal/background-friendly scanning first on large projects. Enable Search Boost or expanded detail only when needed, and stop live traces after collecting the required evidence."),
            Feature.Limitation);
        break;
    }
    return FText::FromString(Text);
}

TSharedRef<SWidget> AnnotationCard(const TCHAR* Label, const TCHAR* Icon, const FText& Body, const FLinearColor& Color)
{
    return SNew(SBorder)
        .BorderImage(FAppStyle::GetBrush(TEXT("Brushes.Panel")))
        .BorderBackgroundColor(FLinearColor(Color.R, Color.G, Color.B, 0.11f))
        .Padding(10.0f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top).Padding(0, 1, 9, 0)
            [
                SNew(SImage).Image(FAppStyle::GetBrush(Icon)).ColorAndOpacity(Color)
            ]
            + SHorizontalBox::Slot().FillWidth(1.0f)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()
                [
                    SNew(STextBlock).Text(GuideText(Label)).Font(GuideFont(TEXT("Bold"), 10)).ColorAndOpacity(Color)
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 0)
                [
                    SNew(STextBlock).Text(Body).AutoWrapText(true).Font(GuideFont(TEXT("Regular"), 10))
                ]
            ]
        ];
}

class STMPluginGuideWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(STMPluginGuideWidget) {}
    SLATE_END_ARGS()

    void Construct(const FArguments&)
    {
        Features = BuildGuideFeatures();
        TSharedRef<SVerticalBox> Navigation = SNew(SVerticalBox);
        SAssignNew(ContentSwitcher, SWidgetSwitcher);

        for (int32 Index = 0; Index < Features.Num(); ++Index)
        {
            Navigation->AddSlot().AutoHeight().Padding(0, 0, 0, 3)
            [
                BuildNavigationTab(Index)
            ];
            ContentSwitcher->AddSlot()[BuildFeaturePage(Features[Index])];
        }

        ChildSlot
        [
            SNew(SBorder).BorderImage(FAppStyle::GetBrush(TEXT("Brushes.Panel"))).Padding(12.0f)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()
                [
                    SNew(STextBlock).Text(GuideText(TEXT("TraceMotive — Debug Pathfinder for Unreal Guide"))).Font(GuideFont(TEXT("Bold"), 17))
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 10)
                [
                    SNew(STextBlock)
                    .Text(GuideText(TEXT("Choose a feature tab, then follow its annotated example, workflow, interpretation, and limitation notes.")))
                    .AutoWrapText(true).ColorAndOpacity(FSlateColor::UseSubduedForeground())
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 10)[SNew(SSeparator)]
                + SVerticalBox::Slot().FillHeight(1.0f)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 12, 0)
                    [
                        SNew(SBox).WidthOverride(230.0f)
                        [
                            SNew(SScrollBox) + SScrollBox::Slot()[Navigation]
                        ]
                    ]
                    + SHorizontalBox::Slot().FillWidth(1.0f)
                    [
                        ContentSwitcher.ToSharedRef()
                    ]
                ]
            ]
        ];
    }

private:
    TSharedRef<SWidget> BuildNavigationTab(const int32 Index)
    {
        const FGuideFeature& Feature = Features[Index];
        return SNew(SBorder)
            .BorderImage(FAppStyle::GetBrush(TEXT("WhiteBrush")))
            .BorderBackgroundColor_Lambda([this, Index, Color = Feature.Color]()
            {
                return SelectedIndex == Index ? FLinearColor(Color.R, Color.G, Color.B, 0.24f) : FLinearColor(0.04f, 0.04f, 0.04f, 0.12f);
            })
            .Padding(1.0f)
            [
                SNew(SButton)
                .ButtonStyle(FAppStyle::Get(), TEXT("SimpleButton"))
                .ContentPadding(FMargin(8, 7))
                .OnClicked_Lambda([this, Index]()
                {
                    SelectedIndex = Index;
                    ContentSwitcher->SetActiveWidgetIndex(Index);
                    return FReply::Handled();
                })
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 8, 0)
                    [SNew(SImage).Image(GetGuideIcon(Feature.Icon)).ColorAndOpacity(Feature.Color)]
                    + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
                    [SNew(STextBlock).Text(GuideText(Feature.Name)).AutoWrapText(true).Font(GuideFont(TEXT("Bold"), 9))]
                ]
            ];
    }

    TSharedRef<SWidget> BuildFeaturePage(const FGuideFeature& Feature) const
    {
        return SNew(SScrollBox)
            + SScrollBox::Slot()
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 6)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 10, 0)
                    [SNew(SImage).Image(GetGuideIcon(Feature.Icon)).ColorAndOpacity(Feature.Color)]
                    + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
                    [SNew(STextBlock).Text(GuideText(Feature.Name)).Font(GuideFont(TEXT("Bold"), 16))]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
                [SNew(STextBlock).Text(GuideText(Feature.Purpose)).AutoWrapText(true).Font(GuideFont(TEXT("Regular"), 11)).ColorAndOpacity(FSlateColor::UseSubduedForeground())]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 14)
                [BuildAnnotatedPreview(Feature)]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
                [AnnotationCard(TEXT("Example situation"), TEXT("Icons.Help"), DetailedAnnotationText(Feature, EGuideDetailSection::Example), Feature.Color)]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
                [AnnotationCard(TEXT("How to use"), TEXT("Icons.Play"), DetailedAnnotationText(Feature, EGuideDetailSection::Usage), FLinearColor(0.30f, 0.72f, 1.0f))]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
                [AnnotationCard(TEXT("How to read the result"), TEXT("Icons.Info"), DetailedAnnotationText(Feature, EGuideDetailSection::Result), FLinearColor(0.34f, 0.82f, 0.48f))]
                + SVerticalBox::Slot().AutoHeight()
                [AnnotationCard(TEXT("Limit / verify"), TEXT("Icons.Warning"), DetailedAnnotationText(Feature, EGuideDetailSection::Limit), FLinearColor(1.0f, 0.62f, 0.20f))]
            ];
    }

    TArray<FGuideFeature> Features;
    TSharedPtr<SWidgetSwitcher> ContentSwitcher;
    int32 SelectedIndex = 0;
};

TSharedRef<SDockTab> SpawnGuideTab(const FSpawnTabArgs&)
{
    TSharedRef<SDockTab> Tab = SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        .Label(GuideText(TEXT("TraceMotive — Debug Pathfinder for Unreal Guide")))
        .OnTabClosed(SDockTab::FOnTabClosedCallback::CreateLambda([](TSharedRef<SDockTab>){ ExistingGuideTab.Reset(); }))
        [SNew(STMPluginGuideWidget)];
    ExistingGuideTab = Tab;
    return Tab;
}

void RegisterGuideTabSpawner()
{
    if (bGuideTabRegistered)
    {
        return;
    }
    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(PluginGuideTabId, FOnSpawnTab::CreateStatic(&SpawnGuideTab))
        .SetDisplayName(GuideText(TEXT("TraceMotive — Debug Pathfinder for Unreal Guide")))
        .SetTooltipText(GuideText(TEXT("Open the annotated guide for TraceMotive — Debug Pathfinder for Unreal tools.")))
        .SetIcon(FSlateIcon(TMStyle::GetStyleSetName(), TEXT("TraceMotive.PluginGuide")));
    bGuideTabRegistered = true;
}
}

namespace TMPluginGuide
{
void RegisterMenus()
{
    RegisterGuideTabSpawner();
    auto AddEntry = [](UToolMenu* Menu, const FName EntryName)
    {
        if (!Menu)
        {
            return;
        }
        Menu->FindOrAddSection(TEXT("TraceMotive")).AddMenuEntry(
            EntryName,
            GuideText(TEXT("TraceMotive — Debug Pathfinder for Unreal Guide")),
            GuideText(TEXT("Open the feature-by-feature annotated guide.")),
            FSlateIcon(TMStyle::GetStyleSetName(), TEXT("TraceMotive.PluginGuide")),
            FToolMenuExecuteAction::CreateLambda([](const FToolMenuContext&){ TMPluginGuide::OpenWindow(); }));
    };
    AddEntry(UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Window")), TEXT("TMOpenPluginGuide"));
    AddEntry(UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Tools")), TEXT("TMToolsOpenPluginGuide"));
}

void UnregisterMenus()
{
    if (bGuideTabRegistered)
    {
        FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(PluginGuideTabId);
        bGuideTabRegistered = false;
    }
    ExistingGuideTab.Reset();
}

void OpenWindow()
{
    RegisterGuideTabSpawner();
    if (ExistingGuideTab.IsValid())
    {
        FGlobalTabmanager::Get()->TryInvokeTab(PluginGuideTabId);
        return;
    }
    ExistingGuideTab = FGlobalTabmanager::Get()->TryInvokeTab(PluginGuideTabId);
}
}
