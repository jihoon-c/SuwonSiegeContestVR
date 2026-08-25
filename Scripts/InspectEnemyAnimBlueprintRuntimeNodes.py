"""Dumps compiled enemy AnimBP node properties for loop verification (read-only)."""

import unreal


for asset_path in (
    "/GF_OngseongCrossbow/Animation/ABP_EnemyMelee",
    "/GF_OngseongCrossbow/Animation/ABP_EnemyArcher",
):
    blueprint = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not blueprint:
        unreal.log_error("[AnimNodeDump] Missing " + asset_path)
        continue
    generated_class = blueprint.generated_class()
    default_object = unreal.get_default_object(generated_class)
    unreal.log("[AnimNodeDump] BEGIN " + asset_path)
    unreal.SystemLibrary.execute_console_command(
        default_object,
        "obj dump {0} recurse=true".format(default_object.get_name()),
    )
    unreal.log("[AnimNodeDump] END " + asset_path)


if "-unattended" in unreal.SystemLibrary.get_command_line().lower():
    unreal.SystemLibrary.quit_editor()
