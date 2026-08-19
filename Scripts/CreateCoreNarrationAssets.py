import unreal


ASSET_TOOLS = unreal.AssetToolsHelpers.get_asset_tools()


def create_vr_pawn_blueprint():
    asset_path = "/Game/Core/VR/Pawn/BP_VRPlayerPawn"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.log(f"Asset already exists: {asset_path}")
        return unreal.load_asset(asset_path)

    parent_class = unreal.load_class(None, "/Script/SuwonSiegeContestVR.VRPlayerPawn")
    if not parent_class:
        raise RuntimeError("Could not load AVRPlayerPawn class")

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    asset = ASSET_TOOLS.create_asset(
        "BP_VRPlayerPawn",
        "/Game/Core/VR/Pawn",
        unreal.Blueprint,
        factory,
    )
    if not asset:
        raise RuntimeError("Failed to create BP_VRPlayerPawn")

    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    return asset


def create_narration_data_table():
    asset_path = "/Game/Core/Experience/Definitions/DT_Narration"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.log(f"Asset already exists: {asset_path}")
        return unreal.load_asset(asset_path)

    row_struct = unreal.load_object(None, "/Script/SuwonSiegeContestVR.NarrationSequenceRow")
    if not row_struct:
        raise RuntimeError("Could not load FNarrationSequenceRow")

    factory = unreal.DataTableFactory()
    factory.set_editor_property("struct", row_struct)
    asset = ASSET_TOOLS.create_asset(
        "DT_Narration",
        "/Game/Core/Experience/Definitions",
        unreal.DataTable,
        factory,
    )
    if not asset:
        raise RuntimeError("Failed to create DT_Narration")

    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    return asset


pawn_blueprint = create_vr_pawn_blueprint()
narration_table = create_narration_data_table()

# Make the project Pawn immediately point at the editable narration table.
generated_class = pawn_blueprint.generated_class()
if generated_class:
    pawn_cdo = unreal.get_default_object(generated_class)
    narration_component = pawn_cdo.get_editor_property("narration_sequence")
    if narration_component:
        narration_component.set_editor_property("narration_table", narration_table)
        unreal.EditorAssetLibrary.save_loaded_asset(pawn_blueprint)

unreal.EditorAssetLibrary.save_directory("/Game/Core", only_if_is_dirty=False, recursive=True)
unreal.log("Core VR Pawn and narration Data Table are ready.")
