"""Sets every placed Ongseong wave manager's archer engagement range to 2000 cm.

Pass ``-OngseongDryRun`` for a read-only preview. The script is idempotent and touches only the
native ``archer_range`` property on wave managers in the two Ongseong maps.
"""

import unreal


LEVELS = (
    "/GF_OngseongCrossbow/Maps/LV_Ongseong",
    "/GF_OngseongCrossbow/Maps/LV_Ongseong_CombatTest",
)
MANAGER_CLASS = "/Script/GF_OngseongCrossbow.OngseongEnemyWaveManager"
TARGET_RANGE = 2000.0
DRY_RUN = "-ongseongdryrun" in unreal.SystemLibrary.get_command_line().lower()

level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
manager_class = unreal.load_class(None, MANAGER_CLASS)
if not manager_class:
    raise RuntimeError("OngseongEnemyWaveManager class is unavailable; build the Editor target first")

changed = 0
for level_path in LEVELS:
    if not level_subsystem.load_level(level_path):
        raise RuntimeError("Could not load " + level_path)
    managers = unreal.EditorFilterLibrary.by_class(actor_subsystem.get_all_level_actors(), manager_class)
    if not managers:
        raise RuntimeError("No wave manager in " + level_path)
    level_changed = 0
    for manager in managers:
        old_value = float(manager.get_editor_property("archer_range"))
        unreal.log("[ArcherRange] {0} {1}: {2:.0f} -> {3:.0f}{4}".format(
            level_path,
            manager.get_actor_label(),
            old_value,
            TARGET_RANGE,
            " (dry run)" if DRY_RUN else "",
        ))
        if not DRY_RUN and not unreal.MathLibrary.nearly_equal_float_float(old_value, TARGET_RANGE):
            manager.set_editor_property("archer_range", TARGET_RANGE)
            changed += 1
            level_changed += 1
    if not DRY_RUN and level_changed:
        # LV_Ongseong loads actors from more than one map package. The actor itself is not a
        # standalone asset, so save the current level and every map package dirtied in this
        # isolated editor process before switching to the next level.
        actor_results = [unreal.EditorAssetLibrary.save_loaded_asset(manager) for manager in managers]
        level_result = level_subsystem.save_current_level()
        dirty_result = unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, False)
        unreal.log("[ArcherRange] save actor={0}, level={1}, dirty_maps={2}".format(
            actor_results, level_result, dirty_result))
        if not level_result or not dirty_result:
            raise RuntimeError("Failed to persist ArcherRange in " + level_path)

unreal.log("[ArcherRange] {0}; changed={1}".format("DRY_RUN" if DRY_RUN else "APPLIED", changed))
if "-unattended" in unreal.SystemLibrary.get_command_line().lower():
    unreal.SystemLibrary.quit_editor()
