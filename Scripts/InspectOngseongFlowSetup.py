"""Read-only report on the Ongseong start-of-experience flow: level blueprint graphs,
sound assets and the runtime flags of the placed managers."""

import unreal

LEVELS = [
    "/GF_OngseongCrossbow/Maps/LV_Ongseong",
    "/GF_OngseongCrossbow/Maps/LV_Ongseong_CombatTest",
]
SOUNDS = [
    "/GF_OngseongCrossbow/Asset/Sound/BGM/SC_BGM",
    "/GF_OngseongCrossbow/Asset/Sound/BGM/battlebgm",
    "/GF_OngseongCrossbow/Asset/Sound/BGM/warcry__cut_9sec_",
    "/GF_OngseongCrossbow/Asset/Sound/WarHorn",
    "/GF_OngseongCrossbow/Asset/Sound/SC_OngseongCombat",
]
FLAGS = ["auto_start", "maintain_population", "max_concurrent_enemies", "swordsman_slots",
         "archer_slots", "use_defense_time_limit", "return_to_main_on_success",
         "auto_retry_on_failure", "lock_player_to_battlement", "play_introduction",
         "required_shots_to_complete", "player_operable"]


def log(msg):
    unreal.log("[FLOW] " + str(msg))


for path in SOUNDS:
    asset = unreal.load_asset(path)
    log("sound {0} -> {1}".format(path, asset.get_class().get_name() if asset else "MISSING"))

for level_path in LEVELS:
    log("=" * 20 + " " + level_path)
    unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(level_path)
    world = unreal.EditorLevelLibrary.get_editor_world()

    blueprint = unreal.load_asset(level_path + "." + level_path.split("/")[-1] + ":PersistentLevel")
    level_bp = world.get_editor_property("persistent_level")
    log("level blueprint search")
    lbp = unreal.load_object(world, "PersistentLevel.LV_Blueprint") if False else None
    # Level Script Blueprint lives at <map>.<map>:PersistentLevel.<map>_C  -- enumerate instead
    for obj in unreal.find_objects_with_outer(world) if hasattr(unreal, "find_objects_with_outer") else []:
        log("  outer obj " + obj.get_name())

    for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
        name = actor.get_class().get_name()
        if name in ("BP_OngseongDefenseScenarioManager_C", "OngseongEnemyWaveManager",
                    "BP_PlayableChongtong_C", "BP_AllyChongtong_C", "ActorPool"):
            values = {}
            for flag in FLAGS:
                try:
                    values[flag] = actor.get_editor_property(flag)
                except Exception:
                    pass
            log("  {0} [{1}] {2}".format(actor.get_actor_label(), name, values))
            if name == "BP_OngseongDefenseScenarioManager_C":
                narration = None
                for comp in actor.get_components_by_class(unreal.ActorComponent):
                    if "Narration" in comp.get_class().get_name():
                        narration = comp
                if narration:
                    log("    narration table: {0}".format(narration.get_editor_property("narration_table")))
                    log("    play_introduction: {0}".format(narration.get_editor_property("play_introduction")))
                    for binding in narration.get_editor_property("event_bindings"):
                        log("      bind {0} -> {1} once={2}".format(
                            binding.get_editor_property("event_name"),
                            binding.get_editor_property("narration_row"),
                            binding.get_editor_property("play_once")))
log("ONGSEONG_FLOW_REPORT DONE")
