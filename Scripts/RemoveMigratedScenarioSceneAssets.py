import unreal


LEGACY_ASSETS = [
    "/Game/Data/DA_Main",
    "/Game/Data/DA_Scene_Singijeon",
]

for asset_path in LEGACY_ASSETS:
    if not unreal.EditorAssetLibrary.delete_asset(asset_path):
        if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
            raise RuntimeError(f"Could not remove migrated asset {asset_path}")
    else:
        unreal.log(f"Removed migrated Scenario Scene asset: {asset_path}")

unreal.log("MIGRATED_SCENARIO_SCENE_ASSET REMOVAL SUCCESS")
