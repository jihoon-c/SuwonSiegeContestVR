import unreal

FEATURE_FOLDER = "/GF_OngseongCrossbow/Blueprints"
LEVEL_PATH = "/GF_OngseongCrossbow/Maps/LV_Ongseong"
MODULE = "/Script/GF_OngseongCrossbow."


def get_or_create_blueprint(name, parent_name):
    path = f"{FEATURE_FOLDER}/{name}"
    existing = unreal.EditorAssetLibrary.load_asset(path)
    if existing:
        return existing
    parent = unreal.load_class(None, MODULE + parent_name)
    if not parent:
        raise RuntimeError(f"{parent_name} is not loaded. Build and restart the editor first.")
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent)
    blueprint = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, FEATURE_FOLDER, unreal.Blueprint, factory)
    if not blueprint:
        raise RuntimeError(f"Failed to create {name}")
    return blueprint


gate_bp = get_or_create_blueprint("BP_OngseongGate", "OngseongGateActor")
ram_bp = get_or_create_blueprint("BP_OngseongRam", "OngseongRamActor")
scenario_bp = get_or_create_blueprint("BP_OngseongDefenseScenarioManager", "OngseongDefenseScenarioManager")


def get_or_duplicate_enemy_blueprint(name):
    path = f"{FEATURE_FOLDER}/{name}"
    existing = unreal.EditorAssetLibrary.load_asset(path)
    if existing:
        return existing
    source = f"{FEATURE_FOLDER}/BP_OngseongEnemySoldier"
    if not unreal.EditorAssetLibrary.duplicate_asset(source, path):
        raise RuntimeError(f"Failed to create {name} from BP_OngseongEnemySoldier")
    return unreal.EditorAssetLibrary.load_asset(path)


swordsman_bp = get_or_duplicate_enemy_blueprint("BP_OngseongSwordsman")
archer_bp = get_or_duplicate_enemy_blueprint("BP_OngseongArcher")

unreal.EditorLoadingAndSavingUtils.load_map(LEVEL_PATH)
actors = unreal.EditorLevelLibrary.get_all_level_actors()
by_label = {actor.get_actor_label(): actor for actor in actors}
gate_visual = by_label.get("JihwaGate_Main")
wave = by_label.get("Ongseong_WaveManager")
gate_actor = by_label.get("Ongseong_GateObjective")
gate_anchor = gate_actor or gate_visual
if not gate_anchor or not wave:
    raise RuntimeError("LV_Ongseong is missing its gate objective/visual or Ongseong_WaveManager")

gate_cdo = unreal.get_default_object(gate_bp.generated_class())
gate_mesh = gate_cdo.get_editor_property("gate_mesh")
source_mesh_component = gate_visual.get_component_by_class(unreal.StaticMeshComponent) if gate_visual else None
if source_mesh_component and source_mesh_component.static_mesh:
    gate_mesh.set_editor_property("static_mesh", source_mesh_component.static_mesh)
gate_health = gate_cdo.get_editor_property("health_component")
gate_health.set_editor_property("max_health", 1000.0)
gate_health.set_editor_property("current_health", 1000.0)

ram_cdo = unreal.get_default_object(ram_bp.generated_class())
ram_mesh = ram_cdo.get_editor_property("ram_mesh")
ram_mesh.set_editor_property("static_mesh", unreal.load_asset("/Engine/BasicShapes/Cube.Cube"))
ram_mesh.set_editor_property("relative_scale3d", unreal.Vector(4.0, 1.2, 1.0))

if not gate_actor:
    gate_actor = unreal.EditorLevelLibrary.spawn_actor_from_object(gate_bp, gate_visual.get_actor_location(), gate_visual.get_actor_rotation())
    gate_actor.set_actor_label("Ongseong_GateObjective")
if gate_visual:
    gate_visual.set_is_temporarily_hidden_in_editor(True)
    gate_visual.set_actor_hidden_in_game(True)

ram_spawn = by_label.get("Ongseong_RamSpawnPoint")
if not ram_spawn:
    ram_spawn = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.TargetPoint, wave.get_actor_location())
ram_spawn.set_actor_label("Ongseong_RamSpawnPoint")

retreat = by_label.get("Ongseong_RetreatPoint")
if not retreat:
    retreat = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.TargetPoint, gate_anchor.get_actor_location() + unreal.Vector(0, -4000, 0))
    retreat.set_actor_label("Ongseong_RetreatPoint")

scenario = by_label.get("Ongseong_DefenseScenario")
if not scenario:
    scenario = unreal.EditorLevelLibrary.spawn_actor_from_object(scenario_bp, gate_anchor.get_actor_location())
    scenario.set_actor_label("Ongseong_DefenseScenario")

wave.set_editor_property("b_auto_start", False)
wave.set_editor_property("objective_target", gate_actor)
enemy_pool = by_label.get("Enemy_ActorPool")
if enemy_pool:
    enemy_pool.set_editor_property("pooled_actor_class", swordsman_bp.generated_class())
    enemy_pool.set_editor_property("initial_pool_size", 6)

archer_pool = by_label.get("Archer_ActorPool")
if not archer_pool:
    pool_class = unreal.load_class(None, "/Script/SuwonSiegeContestVR.ActorPool")
    archer_pool = unreal.EditorLevelLibrary.spawn_actor_from_class(pool_class, wave.get_actor_location())
    archer_pool.set_actor_label("Archer_ActorPool")
archer_pool.set_editor_property("pooled_actor_class", archer_bp.generated_class())
archer_pool.set_editor_property("initial_pool_size", 4)
archer_pool.set_editor_property("b_allow_pool_expansion", False)
archer_pool.tags = [unreal.Name("Ongseong.ArcherPool")]
wave.set_editor_property("archer_enemy_pool", archer_pool)
scenario.set_editor_property("gate_actor", gate_actor)
scenario.set_editor_property("wave_manager", wave)
scenario.set_editor_property("ram_class", ram_bp.generated_class())
scenario.set_editor_property("ram_spawn_point", ram_spawn)
scenario.set_editor_property("retreat_point", retreat)

for blueprint in (gate_bp, ram_bp, scenario_bp, swordsman_bp, archer_bp):
    unreal.KismetEditorUtilities.compile_blueprint(blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(blueprint)
unreal.EditorLevelLibrary.save_current_level()
unreal.log("Created and linked Ongseong gate, ram, and defense scenario assets.")
