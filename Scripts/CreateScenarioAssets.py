import unreal


ASSET_TOOLS = unreal.AssetToolsHelpers.get_asset_tools()


def create_scenario_manager_blueprint():
    asset_path = "/Game/Core/Scenario/Managers/BP_ScenarioManager"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.log(f"Asset already exists: {asset_path}")
        return unreal.load_asset(asset_path)

    parent_class = unreal.load_class(
        None, "/Script/SuwonSiegeContestVR.ScenarioManagerActor"
    )
    if not parent_class:
        raise RuntimeError("Could not load AScenarioManagerActor class")

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    asset = ASSET_TOOLS.create_asset(
        "BP_ScenarioManager",
        "/Game/Core/Scenario/Managers",
        unreal.Blueprint,
        factory,
    )
    if not asset:
        raise RuntimeError("Failed to create BP_ScenarioManager")

    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    return asset


create_scenario_manager_blueprint()
unreal.EditorAssetLibrary.save_directory(
    "/Game/Core/Scenario", only_if_is_dirty=False, recursive=True
)
unreal.log("Core scenario manager Blueprint is ready.")
