import unreal


OLD_PATH = "/Game/Data/DA_Scenario_SuwonSiege"
NEW_PATH = "/Game/Data/DA_Scenario_Singijeon"

scenario = unreal.load_asset(OLD_PATH)
if not scenario:
    scenario = unreal.load_asset(NEW_PATH)
if not scenario:
    raise RuntimeError("Could not load the Singijeon Scenario asset")

if scenario.get_path_name().startswith(OLD_PATH + "."):
    if not unreal.EditorAssetLibrary.rename_asset(OLD_PATH, NEW_PATH):
        raise RuntimeError(f"Could not rename {OLD_PATH} to {NEW_PATH}")

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(
    save_map_packages=True, save_content_packages=True
)
unreal.log("SINGIJEON_SCENARIO_ASSET_NAME NORMALIZATION SUCCESS")
