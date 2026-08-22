import unreal


FOLDER = "/GF_OngseongCrossbow/Blueprints"
PARENT_PATH = "/Script/GF_OngseongCrossbow.ChongtongLoadingItemActor"
GRAB_PATH = "/Game/XRFramework/Blueprints/BP_GrabComponent"
ITEMS = (
    ("BP_ChongtongPowder", "POWDER"),
    ("BP_ChongtongRammer", "RAMMER"),
    ("BP_ChongtongCannonball", "CANNONBALL"),
)


def create_or_load_blueprint(name, parent_class):
    path = f"{FOLDER}/{name}"
    existing = unreal.EditorAssetLibrary.load_asset(path)
    if existing:
        return existing

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    return unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, FOLDER, unreal.Blueprint, factory)


def add_grab_component(blueprint):
    grab_blueprint = unreal.EditorAssetLibrary.load_asset(GRAB_PATH)
    grab_class = grab_blueprint.generated_class() if grab_blueprint else None
    if not grab_class:
        raise RuntimeError("Core BP_GrabComponent is missing")

    subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    library = unreal.SubobjectDataBlueprintFunctionLibrary
    handles = subsystem.k2_gather_subobject_data_for_blueprint(blueprint)
    for handle in handles:
        data = library.get_data(handle)
        if str(library.get_variable_name(data)) == "GrabPoint":
            return

    root_handle = next(
        (handle for handle in handles if library.is_root_component(library.get_data(handle))),
        handles[0],
    )
    params = unreal.AddNewSubobjectParams(
        parent_handle=root_handle,
        new_class=grab_class,
        blueprint_context=blueprint,
    )
    handle, reason = subsystem.add_new_subobject(params)
    if not library.is_handle_valid(handle):
        raise RuntimeError(f"Could not add Core GrabPoint: {reason}")
    subsystem.rename_subobject(handle, unreal.Text("GrabPoint"))


def main():
    parent_class = unreal.load_class(None, PARENT_PATH)
    if not parent_class:
        raise RuntimeError("ChongtongLoadingItemActor is not loaded. Build and restart the editor first.")

    enum_type = getattr(unreal, "ChongtongLoadingItemType")
    created = {}
    for name, enum_name in ITEMS:
        blueprint = create_or_load_blueprint(name, parent_class)
        if not blueprint:
            raise RuntimeError(f"Failed to create {name}")
        add_grab_component(blueprint)
        unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
        default_object = unreal.get_default_object(blueprint.generated_class())
        default_object.set_editor_property("item_type", getattr(enum_type, enum_name))
        unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
        unreal.EditorAssetLibrary.save_loaded_asset(blueprint)
        created[name] = blueprint.generated_class()

    cannon = unreal.EditorAssetLibrary.load_asset(f"{FOLDER}/BP_ChongtongCannon")
    if cannon:
        cannon_default = unreal.get_default_object(cannon.generated_class())
        cannon_default.set_editor_property("powder_item_class", created["BP_ChongtongPowder"])
        cannon_default.set_editor_property("rammer_item_class", created["BP_ChongtongRammer"])
        cannon_default.set_editor_property("cannonball_item_class", created["BP_ChongtongCannonball"])
        unreal.BlueprintEditorLibrary.compile_blueprint(cannon)
        unreal.EditorAssetLibrary.save_loaded_asset(cannon)

    unreal.log("Created and linked Chongtong loading-item Blueprints.")


if __name__ == "__main__":
    main()
