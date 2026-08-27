"""Create the placeable ongseong spawn-point Blueprint after the editor target is built.

Run inside Unreal Editor or headlessly:

    UnrealEditor-Cmd.exe <uproject> -ExecCmds="py <this file>;Quit" -unattended -nosplash
"""

import unreal


ASSET_FOLDER = "/GF_OngseongCrossbow/Gameplay/Waves"
ASSET_NAME = "BP_OngseongSpawnPoint"
ASSET_PATH = f"{ASSET_FOLDER}/{ASSET_NAME}"
PARENT_CLASS_PATH = "/Script/GF_OngseongCrossbow.OngseongSpawnPointActor"


def main():
    # The Game Feature is ExplicitlyLoaded, so commandlets must load its native module before
    # resolving /Script/GF_OngseongCrossbow classes.
    unreal.load_module("GF_OngseongCrossbow")
    asset_library = unreal.EditorAssetLibrary
    blueprint = asset_library.load_asset(ASSET_PATH)
    if not blueprint:
        parent_class = unreal.load_class(None, PARENT_CLASS_PATH)
        if not parent_class:
            raise RuntimeError(
                f"{PARENT_CLASS_PATH} is not loaded. Build the editor target before running this script."
            )

        factory = unreal.BlueprintFactory()
        factory.set_editor_property("parent_class", parent_class)
        blueprint = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            ASSET_NAME,
            ASSET_FOLDER,
            unreal.Blueprint,
            factory,
        )
        if not blueprint:
            raise RuntimeError(f"Failed to create {ASSET_PATH}")

    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    if not asset_library.save_loaded_asset(blueprint):
        raise RuntimeError(f"Failed to save {ASSET_PATH}")

    unreal.log("ONGSEONG_SPAWN_POINT_BLUEPRINT_READY")


if __name__ == "__main__":
    main()
