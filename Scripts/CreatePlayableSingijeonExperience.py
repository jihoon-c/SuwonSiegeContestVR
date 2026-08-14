import unreal


ASSET_TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
SUBOBJECTS = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
SUBOBJECT_BFL = unreal.SubobjectDataBlueprintFunctionLibrary

BP_DIR = "/GF_Singijeon/Gameplay"
LEVEL_PATH = "/GF_Singijeon/Maps/LV_Singijeon"
SCENE_PATH = "/Game/Data/DA_Scene_Singijeon"


def create_or_load_blueprint(name, parent_path):
    path = f"{BP_DIR}/{name}"
    existing = unreal.load_asset(path)
    if existing:
        return existing
    parent = unreal.load_class(None, parent_path)
    if not parent:
        raise RuntimeError(f"Parent class not found: {parent_path}")
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent)
    blueprint = ASSET_TOOLS.create_asset(name, BP_DIR, unreal.Blueprint, factory)
    if not blueprint:
        raise RuntimeError(f"Could not create {path}")
    return blueprint


def add_grab_component(blueprint, component_name):
    grab_bp = unreal.load_asset("/Game/XRFramework/Blueprints/BP_GrabComponent")
    grab_class = grab_bp.generated_class() if grab_bp else None
    if not grab_class:
        raise RuntimeError("BP_GrabComponent class was not found")

    handles = SUBOBJECTS.k2_gather_subobject_data_for_blueprint(blueprint)
    for handle in handles:
        data = SUBOBJECT_BFL.get_data(handle)
        if SUBOBJECT_BFL.get_variable_name(data) == component_name:
            return SUBOBJECT_BFL.get_object(data)

    root_handle = None
    for handle in handles:
        data = SUBOBJECT_BFL.get_data(handle)
        if SUBOBJECT_BFL.is_root_component(data):
            root_handle = handle
            break
    if not root_handle:
        root_handle = handles[0]

    params = unreal.AddNewSubobjectParams(
        parent_handle=root_handle,
        new_class=grab_class,
        blueprint_context=blueprint,
    )
    handle, reason = SUBOBJECTS.add_new_subobject(params)
    if not SUBOBJECT_BFL.is_handle_valid(handle):
        raise RuntimeError(f"Could not add {component_name}: {reason}")
    SUBOBJECTS.rename_subobject(handle, unreal.Text(component_name))
    return SUBOBJECT_BFL.get_object(SUBOBJECT_BFL.get_data(handle))


hwacha_bp = create_or_load_blueprint(
    "BP_SingijeonHwacha", "/Script/GF_Singijeon.SingijeonHwachaActor"
)
arrow_bp = create_or_load_blueprint(
    "BP_SingijeonArrow", "/Script/GF_Singijeon.SingijeonProjectileActor"
)
torch_bp = create_or_load_blueprint(
    "BP_SingijeonTorch", "/Script/GF_Singijeon.IgnitionSourceActor"
)

hwacha_mesh = unreal.load_asset("/GF_Singijeon/Asset/Hwacha/Wooden_Rocket_Cart")
cylinder_mesh = unreal.load_asset("/Engine/BasicShapes/Cylinder")
cube_mesh = unreal.load_asset("/Engine/BasicShapes/Cube")

hwacha_cdo = unreal.get_default_object(hwacha_bp.generated_class())
hwacha_body = hwacha_cdo.get_editor_property("body_mesh")
hwacha_body.set_static_mesh(hwacha_mesh)
hwacha_cdo.get_editor_property("default_ammo_slot").set_editor_property(
    "relative_location", unreal.Vector(25.0, 0.0, 70.0)
)
hwacha_cdo.get_editor_property("default_ammo_slot").set_editor_property(
    "relative_rotation", unreal.Rotator(25.0, 0.0, 0.0)
)
hwacha_cdo.get_editor_property("fuse").set_editor_property(
    "relative_location", unreal.Vector(-35.0, 0.0, 60.0)
)
hwacha_cdo.set_editor_property("minimum_loaded_ammunition", 1)

arrow_cdo = unreal.get_default_object(arrow_bp.generated_class())
arrow_mesh = arrow_cdo.get_editor_property("projectile_mesh")
arrow_mesh.set_static_mesh(cylinder_mesh)
arrow_mesh.set_editor_property("relative_scale3d", unreal.Vector(0.035, 0.035, 0.65))
arrow_mesh.set_editor_property("relative_rotation", unreal.Rotator(90.0, 0.0, 0.0))
add_grab_component(arrow_bp, "GrabPoint")

torch_cdo = unreal.get_default_object(torch_bp.generated_class())
source_mesh = torch_cdo.get_editor_property("source_mesh")
source_mesh.set_static_mesh(cube_mesh)
source_mesh.set_editor_property("relative_scale3d", unreal.Vector(0.06, 0.06, 0.45))
torch_cdo.get_editor_property("ignition_area").set_editor_property(
    "relative_location", unreal.Vector(0.0, 0.0, 45.0)
)
add_grab_component(torch_bp, "GrabPoint")

for blueprint in (hwacha_bp, arrow_bp, torch_bp):
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(blueprint)


def make_interaction(interaction_id, interaction_type, target_id="", next_id=""):
    interaction = unreal.ScenarioInteraction()
    interaction.set_editor_property("interaction_id", interaction_id)
    interaction.set_editor_property("interaction_type", interaction_type)
    interaction.set_editor_property("target_id", target_id)
    interaction.set_editor_property("next_interaction_id", next_id)
    interaction.set_editor_property("required", True)
    return interaction


scene = unreal.load_asset(SCENE_PATH)
if not scene:
    raise RuntimeError(f"Scene not found: {SCENE_PATH}")

interactions = list(scene.get_editor_property("interactions"))
by_id = {
    str(interaction.get_editor_property("interaction_id")): interaction
    for interaction in interactions
}

gameplay_flow = [
    make_interaction(
        "INT_GrabAmmo", unreal.ScenarioInteractionType.GRAB,
        "Singijeon_Ammo", "INT_LoadHwacha"
    ),
    make_interaction(
        "INT_LoadHwacha", unreal.ScenarioInteractionType.CUSTOM,
        "Hwacha_Load", "INT_GrabTorch"
    ),
    make_interaction(
        "INT_GrabTorch", unreal.ScenarioInteractionType.GRAB,
        "Singijeon_Torch", "INT_IgniteHwacha"
    ),
    make_interaction(
        "INT_IgniteHwacha", unreal.ScenarioInteractionType.TRIGGER,
        "Hwacha_Fuse", "INT_FireHwacha"
    ),
    make_interaction(
        "INT_FireHwacha", unreal.ScenarioInteractionType.COMBAT,
        "Hwacha_Fire", ""
    ),
]

for new_interaction in gameplay_flow:
    key = str(new_interaction.get_editor_property("interaction_id"))
    if key in by_id:
        index = interactions.index(by_id[key])
        interactions[index] = new_interaction
    else:
        interactions.append(new_interaction)

if "Singijeon2" in by_id:
    by_id["Singijeon2"].set_editor_property(
        "next_interaction_id", "INT_GrabAmmo"
    )

scene.set_editor_property("interactions", interactions)
unreal.EditorAssetLibrary.save_loaded_asset(scene)


unreal.EditorLoadingAndSavingUtils.load_map(LEVEL_PATH)
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = actor_subsystem.get_all_level_actors()


def find_by_label(label):
    return next((actor for actor in actors if actor.get_actor_label() == label), None)


def spawn_blueprint(blueprint, label, location, rotation=unreal.Rotator()):
    actor = find_by_label(label)
    if not actor:
        actor = actor_subsystem.spawn_actor_from_class(
            blueprint.generated_class(), location, rotation
        )
        actor.set_actor_label(label)
        actors.append(actor)
    else:
        actor.set_actor_location(location, False, False)
        actor.set_actor_rotation(rotation, False)
    return actor


spawn_blueprint(
    hwacha_bp, "BP_SingijeonHwacha_Playable",
    unreal.Vector(300.0, 0.0, 75.0), unreal.Rotator(0.0, 180.0, 0.0)
)
spawn_blueprint(
    arrow_bp, "BP_SingijeonArrow_Playable",
    unreal.Vector(120.0, 40.0, 110.0), unreal.Rotator(0.0, 0.0, 0.0)
)
spawn_blueprint(
    torch_bp, "BP_SingijeonTorch_Playable",
    unreal.Vector(120.0, -60.0, 110.0), unreal.Rotator(0.0, 0.0, 0.0)
)

if not any(isinstance(actor, unreal.PlayerStart) for actor in actors):
    player_start = actor_subsystem.spawn_actor_from_class(
        unreal.PlayerStart, unreal.Vector(0.0, 0.0, 100.0), unreal.Rotator()
    )
    player_start.set_actor_label("PlayerStart_Singijeon")

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log("SINGIJEON_PLAYABLE_SETUP SUCCESS")
