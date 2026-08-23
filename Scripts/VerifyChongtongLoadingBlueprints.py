import unreal


FOLDER = "/GF_OngseongCrossbow/Blueprints"
PARENT_PATH = "/Script/GF_OngseongCrossbow.ChongtongLoadingItemActor"
GRAB_PATH = "/Game/XRFramework/Blueprints/BP_GrabComponent"
ITEMS = (
    ("BP_ChongtongPowder", "POWDER", "powder_item_class"),
    ("BP_ChongtongRammer", "RAMMER", "rammer_item_class"),
    ("BP_ChongtongCannonball", "CANNONBALL", "cannonball_item_class"),
)


def main():
    parent_class = unreal.load_class(None, PARENT_PATH)
    grab_blueprint = unreal.EditorAssetLibrary.load_asset(GRAB_PATH)
    cannon_blueprint = unreal.EditorAssetLibrary.load_asset(f"{FOLDER}/BP_ChongtongCannon")
    if not parent_class or not grab_blueprint or not cannon_blueprint:
        raise RuntimeError("Required Chongtong/Core assets or native parent class are missing")

    grab_class = grab_blueprint.generated_class()
    cannon_default = unreal.get_default_object(cannon_blueprint.generated_class())
    enum_type = unreal.ChongtongLoadingItemType

    for name, enum_name, cannon_property in ITEMS:
        blueprint = unreal.EditorAssetLibrary.load_asset(f"{FOLDER}/{name}")
        if not blueprint:
            raise RuntimeError(f"Missing loading Blueprint: {name}")

        generated_class = blueprint.generated_class()
        default_object = unreal.get_default_object(generated_class)
        # Accessing this native-only property also proves the generated class inherits
        # ChongtongLoadingItemActor; Unreal Python UObject wrappers do not expose IsA.
        if default_object.get_editor_property("item_type") != getattr(enum_type, enum_name):
            raise RuntimeError(f"{name} has the wrong item_type")

        subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
        library = unreal.SubobjectDataBlueprintFunctionLibrary
        grab_components = []
        for handle in subsystem.k2_gather_subobject_data_for_blueprint(blueprint):
            data = library.get_data(handle)
            if str(library.get_variable_name(data)) == "GrabPoint":
                grab_components.append(library.get_object(data))
        if len(grab_components) != 1:
            raise RuntimeError(f"{name} must contain exactly one Core BP_GrabComponent")
        if grab_components[0].get_class() != grab_class:
            raise RuntimeError(f"{name}.GrabPoint is not the Core BP_GrabComponent class")

        configured_class = cannon_default.get_editor_property(cannon_property)
        if configured_class != generated_class:
            raise RuntimeError(f"BP_ChongtongCannon.{cannon_property} does not reference {name}")

        unreal.log(f"VERIFIED {name}: parent, ItemType, Core GrabPoint, cannon class slot")

    unreal.log("CHONGTONG_LOADING_BLUEPRINTS_VERIFIED")


if __name__ == "__main__":
    main()
