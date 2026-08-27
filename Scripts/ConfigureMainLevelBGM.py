"""Place one editor-configurable 2D BGM player in L_Main.

Run after compiling the SuwonSiegeContestVR Editor target.
Existing player settings are preserved except for a missing Music assignment.
"""

import unreal

MAIN_LEVEL_PATH = "/Game/Maps/Main/L_Main"
DEFAULT_MUSIC_PATH = "/Game/Audio/Effect/Iron_parade_1.Iron_parade_1"

unreal.EditorLoadingAndSavingUtils.load_map(MAIN_LEVEL_PATH)
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = actor_subsystem.get_all_level_actors()
players = [actor for actor in actors if isinstance(actor, unreal.MainLevelBGMPlayerActor)]

player = players[0] if players else actor_subsystem.spawn_actor_from_class(
    unreal.MainLevelBGMPlayerActor, unreal.Vector(), unreal.Rotator()
)
for duplicate in players[1:]:
    actor_subsystem.destroy_actor(duplicate)

player.set_actor_label("MainLevel_BGM_Player")
if not player.get_editor_property("music"):
    music = unreal.load_asset(DEFAULT_MUSIC_PATH)
    if not music:
        raise RuntimeError("Default BGM asset could not be loaded: {}".format(DEFAULT_MUSIC_PATH))
    player.set_editor_property("music", music)

# This is deliberately applied even when a designer has already selected a
# different Music asset: the selected song must start with the level.
player.set_editor_property("play_on_begin_play", True)

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log("MAIN_LEVEL_BGM CONFIGURE SUCCESS: {} starts '{}' on BeginPlay".format(
    player.get_name(), player.get_editor_property("music").get_path_name()))
