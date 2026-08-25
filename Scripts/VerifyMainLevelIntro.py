"""Static editor verification for the Main opening sequence placement and timing."""

import unreal


MAIN_LEVEL_PATH = "/Game/Maps/Main/L_Main"


unreal.EditorLoadingAndSavingUtils.load_map(MAIN_LEVEL_PATH)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
intros = [actor for actor in actors if isinstance(actor, unreal.MainLevelIntroActor)]
managers = [actor for actor in actors if isinstance(actor, unreal.MainEducationScenarioManagerActor)]

if len(intros) != 1:
    raise RuntimeError(f"Expected exactly one MainLevelIntroActor, got {len(intros)}")
if len(managers) != 1:
    raise RuntimeError(f"Expected exactly one MainEducationScenarioManagerActor, got {len(managers)}")

intro = intros[0]
manager = managers[0]
if not manager.get_editor_property("wait_for_intro_sequence"):
    raise RuntimeError("Main manager is not waiting for the intro sequence")
if intro.get_editor_property("target_education_manager") != manager:
    raise RuntimeError("Intro actor does not target the Main education manager")

if abs(intro.get_editor_property("title_display_duration") - 3.0) > 0.001:
    raise RuntimeError("Title display duration is not the required three seconds")
if intro.get_editor_property("title_fade_duration") <= 0.0:
    raise RuntimeError("Title fade duration must be positive")

for property_name in ("overview_anchor", "title_anchor", "player_anchor"):
    if not intro.get_editor_property(property_name):
        raise RuntimeError(f"Missing editable intro anchor: {property_name}")

title_location = intro.get_editor_property("title_anchor").get_editor_property("relative_location")
player_location = intro.get_editor_property("player_anchor").get_editor_property("relative_location")
if unreal.MathLibrary.vector_distance(title_location, player_location) > 0.1:
    raise RuntimeError("Default title and player viewpoints should match for the requested flow")

unreal.log(
    "MAIN_LEVEL_INTRO VERIFY SUCCESS: 3 editable anchors, 3s title, fade-out, and deferred narration"
)
