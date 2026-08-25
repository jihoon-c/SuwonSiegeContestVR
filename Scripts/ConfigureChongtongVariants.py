import unreal


FOLDER = "/GF_OngseongCrossbow/Blueprints"
TEMPLATE = f"{FOLDER}/BP_ChongtongCannon"
VARIANTS = {
    "BP_AllyChongtong": {
        "b_enable_automatic_fire": True,
        "fire_interval": 5.0,
        "b_spawn_placeholder_props": False,
        "b_spawn_operator_on_begin_play": True,
    },
    "BP_PlayableChongtong": {
        "b_enable_automatic_fire": False,
        "fire_interval": 5.0,
        "b_spawn_placeholder_props": True,
        "b_spawn_operator_on_begin_play": False,
    },
}


def main():
    template = unreal.EditorAssetLibrary.load_asset(TEMPLATE)
    if not template:
        raise RuntimeError("BP_ChongtongCannon template is missing")

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    for name, values in VARIANTS.items():
        path = f"{FOLDER}/{name}"
        blueprint = unreal.EditorAssetLibrary.load_asset(path)
        if not blueprint:
            blueprint = asset_tools.duplicate_asset(name, FOLDER, template)
        if not blueprint:
            raise RuntimeError(f"Could not create {name}")

        default_object = unreal.get_default_object(blueprint.generated_class())
        for property_name, value in values.items():
            default_object.set_editor_property(property_name, value)
        unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
        unreal.EditorAssetLibrary.save_loaded_asset(blueprint)

    unreal.log("CHONGTONG_VARIANTS_CONFIGURED: ally auto-fire=5s, playable interaction enabled")


if __name__ == "__main__":
    main()
