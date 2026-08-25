"""Dumps the compiled archer Behavior Tree, including Move To acceptance radius (read-only)."""

import unreal


TREE_PATH = "/GF_OngseongCrossbow/AI/BT_EnemyArcher"
tree = unreal.EditorAssetLibrary.load_asset(TREE_PATH)
if not tree:
    raise RuntimeError("Missing " + TREE_PATH)

unreal.log("[ArcherBTDump] BEGIN")
unreal.SystemLibrary.execute_console_command(tree, "obj dump BT_EnemyArcher recurse=true")
unreal.SystemLibrary.execute_console_command(tree, "obj dump BTComposite_Sequence_0 recurse=true")
unreal.SystemLibrary.execute_console_command(tree, "obj dump BTTask_MoveTo_0 recurse=true")
unreal.log("[ArcherBTDump] END")

if "-unattended" in unreal.SystemLibrary.get_command_line().lower():
    unreal.SystemLibrary.quit_editor()
