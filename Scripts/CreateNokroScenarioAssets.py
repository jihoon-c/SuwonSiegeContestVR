import unreal


GAMEPLAY_PATH = "/GF_Geojunggi/Gameplay"
MAP_PATH = "/GF_Geojunggi/Maps/LV_Nokro"


def load_native_class(name):
    native_class = unreal.load_class(None, f"/Script/GF_Geojunggi.{name}")
    if not native_class:
        raise RuntimeError(f"Could not load native class {name}")
    return native_class


def get_or_create_blueprint(asset_name, parent_class):
    asset_path = f"{GAMEPLAY_PATH}/{asset_name}"
    blueprint = unreal.load_asset(asset_path)
    if blueprint:
        return blueprint
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    blueprint = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name, GAMEPLAY_PATH, unreal.Blueprint, factory)
    if not blueprint:
        raise RuntimeError(f"Could not create {asset_path}")
    return blueprint


crane_bp = get_or_create_blueprint("BP_NokroCrane", load_native_class("NokroCraneActor"))
target_bp = get_or_create_blueprint("BP_NokroRepairTarget", load_native_class("NokroRepairTargetActor"))
manager_bp = get_or_create_blueprint("BP_NokroScenarioManager", load_native_class("NokroScenarioManager"))

for blueprint in (crane_bp, target_bp, manager_bp):
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)

manager_class = manager_bp.generated_class()
manager_defaults = unreal.get_default_object(manager_class)
manager_defaults.set_editor_property("crane_class", crane_bp.generated_class())
manager_defaults.set_editor_property("repair_target_class", target_bp.generated_class())
manager_defaults.set_editor_property("spawn_default_layout", True)
manager_defaults.set_editor_property("auto_start", True)
unreal.BlueprintEditorLibrary.compile_blueprint(manager_bp)

if not unreal.EditorLoadingAndSavingUtils.load_map(MAP_PATH):
    raise RuntimeError(f"Could not load {MAP_PATH}")

actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
existing_managers = [
    actor for actor in actor_subsystem.get_all_level_actors()
    if "NokroScenarioManager" in actor.get_class().get_name()
]
if not existing_managers:
    actor = actor_subsystem.spawn_actor_from_class(manager_bp.generated_class(), unreal.Vector(), unreal.Rotator())
    if not actor:
        raise RuntimeError("Could not place BP_NokroScenarioManager in LV_Nokro")
    actor.set_actor_label("NokroScenarioManager")

for blueprint in (crane_bp, target_bp, manager_bp):
    unreal.EditorAssetLibrary.save_loaded_asset(blueprint, only_if_is_dirty=False)
if not unreal.EditorLoadingAndSavingUtils.save_current_level():
    raise RuntimeError("Could not save LV_Nokro")

unreal.log("NOKRO_BLUEPRINT_ASSETS_SUCCESS: art-ready presets created and manager placed")
