import unreal


# This removes only the prefab created by the companion creation script. It is
# used to recover from interrupted creation runs before the asset is handed off.
PREFAB_PATH = "/GF_Singijeon/Gameplay/Props/BP_SingijeonArrowBox_Instanced"

if unreal.EditorAssetLibrary.does_asset_exist(PREFAB_PATH):
    if not unreal.EditorAssetLibrary.delete_asset(PREFAB_PATH):
        raise RuntimeError("Could not remove the incomplete arrow box prefab")
unreal.log("SINGIJEON_ARROW_BOX_RECREATE CLEANUP SUCCESS")
