"""Read-only verification of the Main title font, text, and editor-preview state."""

import unreal


MAIN_LEVEL_PATH = "/Game/Maps/Main/L_Main"
EXPECTED_FONT_PATH = "/Game/UI/Fonts/GmarketSansBold_Font"


unreal.EditorLoadingAndSavingUtils.load_map(MAIN_LEVEL_PATH)
expected_font = unreal.load_asset(EXPECTED_FONT_PATH)
if not isinstance(expected_font, unreal.Font):
    raise RuntimeError(f"Missing Runtime Font: {EXPECTED_FONT_PATH}")

actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
intros = [actor for actor in actors if isinstance(actor, unreal.MainLevelIntroActor)]
if len(intros) != 1:
    raise RuntimeError(f"Expected one MainLevelIntroActor, got {len(intros)}")

intro = intros[0]
if not intro.get_editor_property("show_editor_title_preview"):
    raise RuntimeError("Editor title preview is disabled")
if intro.get_editor_property("title_font") != expected_font:
    raise RuntimeError("Title Font is not Gmarket Sans Bold")
if not intro.get_editor_property("title_text"):
    raise RuntimeError("Title Text is empty")

title_component = intro.get_editor_property("title_widget_component")
if not title_component.get_editor_property("visible"):
    raise RuntimeError("Title UMG WidgetComponent is not visible in the editor")

legacy_component = intro.get_editor_property("title_text_render")
if legacy_component.get_editor_property("visible"):
    raise RuntimeError("Legacy TextRender is still visible and may use DefaultTextMaterialOpaque")

widget_class = title_component.get_editor_property("widget_class")
if widget_class != unreal.MainLevelTitleWidget.static_class():
    raise RuntimeError(f"Unexpected title WidgetClass: {widget_class}")

unreal.log(
    "MAIN_TITLE_PRESENTATION VERIFY SUCCESS: "
    f"font={expected_font.get_path_name()} title={intro.get_editor_property('title_text')} "
    f"renderer=UMG widget_class={widget_class.get_name()} "
    f"preview_location={title_component.get_world_location()}"
)
