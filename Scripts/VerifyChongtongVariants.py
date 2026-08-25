import unreal


FOLDER = "/GF_OngseongCrossbow/Blueprints"
EXPECTED = {
    "BP_AllyChongtong": (True, False, True),
    "BP_PlayableChongtong": (False, True, False),
}


def main():
    automatic_fire_type = getattr(unreal, "ChongtongAutomaticFireComponent", None)
    if automatic_fire_type is None:
        raise RuntimeError("Restart the editor after building so ChongtongAutomaticFireComponent is loaded")

    for name, (automatic, placeholder_props, operator) in EXPECTED.items():
        blueprint = unreal.EditorAssetLibrary.load_asset(f"{FOLDER}/{name}")
        if not blueprint:
            raise RuntimeError(f"Missing {name}")

        default_object = unreal.get_default_object(blueprint.generated_class())
        if bool(default_object.get_editor_property("b_enable_automatic_fire")) != automatic:
            raise RuntimeError(f"{name}: automatic-fire mode mismatch")
        if abs(float(default_object.get_editor_property("fire_interval")) - 5.0) > 0.001:
            raise RuntimeError(f"{name}: fire interval must be 5 seconds")
        if bool(default_object.get_editor_property("b_spawn_placeholder_props")) != placeholder_props:
            raise RuntimeError(f"{name}: loading-prop mode mismatch")
        if bool(default_object.get_editor_property("b_spawn_operator_on_begin_play")) != operator:
            raise RuntimeError(f"{name}: operator mode mismatch")

        component = default_object.get_editor_property("automatic_fire")
        if not isinstance(component, automatic_fire_type):
            raise RuntimeError(f"{name}: automatic-fire component is not composed")

    unreal.log("CHONGTONG_VARIANTS_VERIFIED")


if __name__ == "__main__":
    main()
