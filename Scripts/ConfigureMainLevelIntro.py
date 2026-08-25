"""Place/configure the editor-authored Main opening sequence in L_Main.

Run after compiling the SuwonSiegeContestVR Editor target.
"""

import unreal


MAIN_LEVEL_PATH = "/Game/Maps/Main/L_Main"


def first_of_type(actors, actor_type):
    return next((actor for actor in actors if isinstance(actor, actor_type)), None)


unreal.EditorLoadingAndSavingUtils.load_map(MAIN_LEVEL_PATH)
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = actor_subsystem.get_all_level_actors()

manager = first_of_type(actors, unreal.MainEducationScenarioManagerActor)
if not manager:
    raise RuntimeError("L_Main has no MainEducationScenarioManagerActor")

player_start = first_of_type(actors, unreal.PlayerStart)
if not player_start:
    raise RuntimeError("L_Main has no PlayerStart to use as the final gate view")

intros = [actor for actor in actors if isinstance(actor, unreal.MainLevelIntroActor)]
created_intro = not intros
intro = intros[0] if intros else actor_subsystem.spawn_actor_from_class(
    unreal.MainLevelIntroActor, unreal.Vector(), unreal.Rotator()
)
for duplicate in intros[1:]:
    actor_subsystem.destroy_actor(duplicate)

intro.set_actor_label("MainLevelIntro_CastleToGate")
intro.set_editor_property("target_education_manager", manager)
intro.set_editor_property("overview_to_title_duration", 5.0)
intro.set_editor_property("title_display_duration", 3.0)
intro.set_editor_property("title_fade_duration", 1.0)
intro.set_editor_property("title_to_player_duration", 0.0)
intro.set_editor_property("play_on_begin_play", True)
intro.set_editor_property("lock_player_input_during_intro", True)

player_location = player_start.get_actor_location()
player_rotation = player_start.get_actor_rotation()
overview_location = player_location + unreal.Vector(-6000.0, -10000.0, 3500.0)
overview_rotation = unreal.MathLibrary.find_look_at_rotation(overview_location, player_location)

if created_intro:
    overview_anchor = intro.get_editor_property("overview_anchor")
    title_anchor = intro.get_editor_property("title_anchor")
    final_anchor = intro.get_editor_property("player_anchor")

    # Apply these defaults only at creation. Subsequent runs intentionally do
    # not change any placed actor or Anchor transform authored in the level.
    overview_anchor.set_relative_location(overview_location, False, False)
    overview_anchor.set_relative_rotation(overview_rotation, False, False)
    title_anchor.set_relative_location(player_location, False, False)
    title_anchor.set_relative_rotation(player_rotation, False, False)
    final_anchor.set_relative_location(player_location, False, False)
    final_anchor.set_relative_rotation(player_rotation, False, False)

# The manager must wait for this actor; it starts the first instructor narration
# only after title fade-out and arrival at PlayerAnchor.
manager.set_editor_property("wait_for_intro_sequence", True)

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log(
    "MAIN_LEVEL_INTRO CONFIGURE SUCCESS: overview/title/player anchors and deferred Main narration configured"
)
