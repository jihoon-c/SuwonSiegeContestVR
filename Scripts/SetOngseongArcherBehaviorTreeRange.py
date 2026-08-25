"""Keeps the archer BT Move To radius at 90% of the 2000 cm firing range."""

import unreal


TREE_PATH = "/GF_OngseongCrossbow/AI/BT_EnemyArcher"
MOVE_TASK_PATH = TREE_PATH + ".BT_EnemyArcher:BTTask_MoveTo_0"
TARGET_RADIUS = 1800.0
DRY_RUN = "-ongseongdryrun" in unreal.SystemLibrary.get_command_line().lower()

tree = unreal.EditorAssetLibrary.load_asset(TREE_PATH)
move_task = unreal.load_object(None, MOVE_TASK_PATH)
if not tree or not move_task:
    raise RuntimeError("Could not load the Ongseong archer tree or its Move To task")

radius = move_task.get_editor_property("acceptable_radius")
old_value = float(radius.get_editor_property("default_value"))
unreal.log("[ArcherBTRange] {0:.0f} -> {1:.0f}{2}".format(
    old_value, TARGET_RADIUS, " (dry run)" if DRY_RUN else ""))

if not DRY_RUN and not unreal.MathLibrary.nearly_equal_float_float(old_value, TARGET_RADIUS):
    radius.set_editor_property("default_value", TARGET_RADIUS)
    move_task.set_editor_property("acceptable_radius", radius)
    if not unreal.EditorAssetLibrary.save_loaded_asset(tree):
        raise RuntimeError("Could not save " + TREE_PATH)
    unreal.log("[ArcherBTRange] APPLIED")
else:
    unreal.log("[ArcherBTRange] " + ("DRY_RUN" if DRY_RUN else "UNCHANGED"))

if "-unattended" in unreal.SystemLibrary.get_command_line().lower():
    unreal.SystemLibrary.quit_editor()
