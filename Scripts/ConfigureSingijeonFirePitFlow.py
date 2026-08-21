import unreal


ASSET_TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
SUBOBJECTS = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
SUBOBJECT_BFL = unreal.SubobjectDataBlueprintFunctionLibrary
BP_DIR = "/GF_Singijeon/Gameplay"
TORCH_BP_PATH = BP_DIR + "/BP_SingijeonTorch"
FIREPIT_BP_PATH = BP_DIR + "/BP_SingijeonFirePit"
LEVEL_PATH = "/Game/Maps/LV_Singijeon"
FIREPIT_MESH_PATH = "/GF_Singijeon/Asset/FirePit/Geometric_Fire_Pit"
FIRE_SYSTEM_PATH = "/Game/NiagaraExamples/FX_Misc/NS_Fire"
IGNITION_HEIGHT = 102.0
# NS_Fire has symmetric -100..100 fixed bounds. At 0.55 scale its sprite lower
# edge extends about 55 cm below the component origin, so place the visual origin
# above the 100 cm Fire Pit rim instead of using the ignition overlap height.
FIRE_EFFECT_HEIGHT = 155.0


def create_or_load_blueprint(name, parent_path):
    path = f"{BP_DIR}/{name}"
    blueprint = unreal.load_asset(path)
    if blueprint:
        return blueprint
    parent = unreal.load_class(None, parent_path)
    if not parent:
        raise RuntimeError(f"Parent class not found: {parent_path}")
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent)
    blueprint = ASSET_TOOLS.create_asset(name, BP_DIR, unreal.Blueprint, factory)
    if not blueprint:
        raise RuntimeError(f"Could not create {path}")
    return blueprint


def find_or_add_niagara_component(blueprint, component_name):
    handles = SUBOBJECTS.k2_gather_subobject_data_for_blueprint(blueprint)
    for handle in handles:
        data = SUBOBJECT_BFL.get_data(handle)
        component = SUBOBJECT_BFL.get_object(data)
        if isinstance(component, unreal.NiagaraComponent):
            return component

    root_handle = next(
        (
            handle
            for handle in handles
            if SUBOBJECT_BFL.is_root_component(SUBOBJECT_BFL.get_data(handle))
        ),
        handles[0],
    )
    params = unreal.AddNewSubobjectParams(
        parent_handle=root_handle,
        new_class=unreal.NiagaraComponent,
        blueprint_context=blueprint,
    )
    handle, reason = SUBOBJECTS.add_new_subobject(params)
    if not SUBOBJECT_BFL.is_handle_valid(handle):
        raise RuntimeError(f"Could not add {component_name}: {reason}")
    SUBOBJECTS.rename_subobject(handle, unreal.Text(component_name))
    return SUBOBJECT_BFL.get_object(SUBOBJECT_BFL.get_data(handle))


torch_bp = unreal.load_asset(TORCH_BP_PATH)
if not torch_bp:
    raise RuntimeError("BP_SingijeonTorch is missing")
firepit_bp = create_or_load_blueprint(
    "BP_SingijeonFirePit", "/Script/GF_Singijeon.FirePitActor"
)
firepit_mesh = unreal.load_asset(FIREPIT_MESH_PATH)
fire_system = unreal.load_asset(FIRE_SYSTEM_PATH)
if not firepit_mesh or not fire_system:
    raise RuntimeError("Fire Pit mesh or NS_Fire is missing")

torch_cdo = unreal.get_default_object(torch_bp.generated_class())
torch_cdo.set_editor_property("ignition_active", False)
torch_effect = find_or_add_niagara_component(torch_bp, "FireEffect")
torch_effect.set_editor_property("asset", fire_system)
torch_effect.set_editor_property("auto_activate", False)
torch_effect_location = torch_effect.get_editor_property("relative_location")
if (abs(torch_effect_location.x) < 0.01 and
        abs(torch_effect_location.y) < 0.01 and
        abs(torch_effect_location.z) < 0.01):
    ignition_location = torch_cdo.get_editor_property("ignition_area").get_editor_property(
        "relative_location"
    )
    torch_effect.set_editor_property("relative_location", ignition_location)
    torch_effect.set_editor_property("relative_scale3d", unreal.Vector(0.25, 0.25, 0.25))

firepit_cdo = unreal.get_default_object(firepit_bp.generated_class())
firepit_cdo.get_editor_property("fire_pit_mesh").set_static_mesh(firepit_mesh)
firepit_cdo.get_editor_property("ignition_area").set_editor_property(
    "relative_location", unreal.Vector(0.0, 0.0, IGNITION_HEIGHT)
)
firepit_effect = find_or_add_niagara_component(firepit_bp, "FireEffect")
firepit_effect.set_editor_property("asset", fire_system)
firepit_effect.set_editor_property("auto_activate", True)
firepit_effect.set_editor_property(
    "relative_location", unreal.Vector(0.0, 0.0, FIRE_EFFECT_HEIGHT)
)
firepit_effect.set_editor_property("relative_scale3d", unreal.Vector(0.55, 0.55, 0.55))

for blueprint in (torch_bp, firepit_bp):
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(blueprint, only_if_is_dirty=False)

actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
for level_path in (LEVEL_PATH, "/GF_Singijeon/Maps/LV_Singijeon"):
    if not unreal.EditorAssetLibrary.does_asset_exist(level_path):
        continue
    unreal.EditorLoadingAndSavingUtils.load_map(level_path)
    actors = actor_subsystem.get_all_level_actors()
    torch_actor = next(
        (actor for actor in actors if actor.get_actor_label() == "BP_SingijeonTorch_Playable"),
        None,
    )
    if torch_actor:
        torch_actor.set_editor_property("ignition_active", False)

    firepit_actor = next(
        (actor for actor in actors if actor.get_actor_label() == "BP_SingijeonFirePit_Playable"),
        None,
    )
    if not firepit_actor:
        firepit_actor = actor_subsystem.spawn_actor_from_class(
            firepit_bp.generated_class(), unreal.Vector(120.0, -180.0, 0.0), unreal.Rotator()
        )
        firepit_actor.set_actor_label("BP_SingijeonFirePit_Playable")
    for component in firepit_actor.get_components_by_class(unreal.NiagaraComponent):
        if component.get_name() == "FireEffect":
            component.set_editor_property(
                "relative_location", unreal.Vector(0.0, 0.0, FIRE_EFFECT_HEIGHT)
            )
            component.reinitialize_system()
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)

unreal.log("SINGIJEON_FIREPIT_FLOW CONFIGURE SUCCESS")
