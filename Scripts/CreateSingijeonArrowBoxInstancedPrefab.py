import unreal


ROOT = "/GF_Singijeon/Gameplay/Props"
MATERIAL_DIR = ROOT + "/Materials"
PREFAB_NAME = "BP_SingijeonArrowBox_Instanced"
MATERIAL_NAME = "M_SingijeonArrowBox_Wood"
ARROW_MESH_PATH = "/GF_Singijeon/Asset/Arrow/arrowb/StaticMeshes/arrowb"
ARROW_MATERIAL_PATH = "/GF_Singijeon/Asset/Arrow/arrowb/Materials/M_SingijeonArrow_Runtime"
CUBE_PATH = "/Engine/BasicShapes/Cube"
ROWS = 6
COLUMNS = 15
ROW_SPACING = 8.0
COLUMN_SPACING = 8.0

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
subobjects = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
subobject_bfl = unreal.SubobjectDataBlueprintFunctionLibrary


def load_required(path):
    asset = unreal.load_asset(path)
    if not asset:
        raise RuntimeError(f"Required asset is missing: {path}")
    return asset


def find_component(blueprint, name):
    for handle in subobjects.k2_gather_subobject_data_for_blueprint(blueprint):
        data = subobject_bfl.get_data(handle)
        component = subobject_bfl.get_object(data)
        if component and component.get_name() == name:
            return component
    return None


def add_component(blueprint, component_class, name):
    handles = subobjects.k2_gather_subobject_data_for_blueprint(blueprint)
    root_handle = next(
        (handle for handle in handles if subobject_bfl.is_root_component(
            subobject_bfl.get_data(handle))), handles[0]
    )
    params = unreal.AddNewSubobjectParams(
        parent_handle=root_handle,
        new_class=component_class,
        blueprint_context=blueprint,
    )
    handle, reason = subobjects.add_new_subobject(params)
    if not subobject_bfl.is_handle_valid(handle):
        raise RuntimeError(f"Could not add {name}: {reason}")
    subobjects.rename_subobject(handle, unreal.Text(name))
    return subobject_bfl.get_object(subobject_bfl.get_data(handle))


def get_or_add_component(blueprint, component_class, name):
    return find_component(blueprint, name) or add_component(blueprint, component_class, name)


def configure_board(component, cube, wood, location, dimensions):
    component.set_static_mesh(cube)
    component.set_material(0, wood)
    component.set_editor_property("mobility", unreal.ComponentMobility.STATIC)
    component.set_editor_property("cast_shadow", True)
    component.set_collision_profile_name("BlockAll")
    component.set_editor_property("relative_location", unreal.Vector(*location))
    component.set_relative_scale3d(unreal.Vector(
        dimensions[0] / 100.0, dimensions[1] / 100.0, dimensions[2] / 100.0))


for folder in (ROOT, MATERIAL_DIR):
    unreal.EditorAssetLibrary.make_directory(folder)

arrow_mesh = load_required(ARROW_MESH_PATH)
arrow_material = load_required(ARROW_MATERIAL_PATH)
cube = load_required(CUBE_PATH)

wood = unreal.load_asset(f"{MATERIAL_DIR}/{MATERIAL_NAME}")
if not wood:
    wood = asset_tools.create_asset(
        MATERIAL_NAME, MATERIAL_DIR, unreal.Material, unreal.MaterialFactoryNew())
if not wood:
    raise RuntimeError("Could not create arrow box wood material")
unreal.MaterialEditingLibrary.delete_all_material_expressions(wood)
color = unreal.MaterialEditingLibrary.create_material_expression(
    wood, unreal.MaterialExpressionConstant3Vector, -200, 0)
color.set_editor_property("constant", unreal.LinearColor(0.16, 0.055, 0.012, 1.0))
roughness = unreal.MaterialEditingLibrary.create_material_expression(
    wood, unreal.MaterialExpressionConstant, -200, 120)
roughness.set_editor_property("r", 0.82)
unreal.MaterialEditingLibrary.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)
unreal.MaterialEditingLibrary.connect_material_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
wood.set_editor_property("fully_rough", True)
unreal.MaterialEditingLibrary.recompile_material(wood)
unreal.EditorAssetLibrary.save_loaded_asset(wood, only_if_is_dirty=False)

prefab = unreal.load_asset(f"{ROOT}/{PREFAB_NAME}")
if not prefab:
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", unreal.Actor)
    prefab = asset_tools.create_asset(PREFAB_NAME, ROOT, unreal.Blueprint, factory)
if not prefab:
    raise RuntimeError("Could not create arrow box prefab")

# The source arrow points along local X. The tray length follows its mesh bounds,
# so imported arrow scale changes do not make the prefab unusable.
bounds = arrow_mesh.get_bounds()
arrow_length = max(100.0, bounds.box_extent.x * 2.0)
tray_length = arrow_length + 20.0
tray_width = (COLUMNS - 1) * COLUMN_SPACING + 24.0
tray_height = (ROWS - 1) * ROW_SPACING + 30.0
board_thickness = 5.0
base_thickness = 8.0

configure_board(get_or_add_component(prefab, unreal.StaticMeshComponent, "ArrowBoxBottom"),
                cube, wood, (0.0, 0.0, 0.0),
                (tray_length, tray_width, base_thickness))
configure_board(get_or_add_component(prefab, unreal.StaticMeshComponent, "ArrowBoxLeftWall"),
                cube, wood, (0.0, -tray_width / 2.0 + board_thickness / 2.0, tray_height / 2.0),
                (tray_length, board_thickness, tray_height))
configure_board(get_or_add_component(prefab, unreal.StaticMeshComponent, "ArrowBoxRightWall"),
                cube, wood, (0.0, tray_width / 2.0 - board_thickness / 2.0, tray_height / 2.0),
                (tray_length, board_thickness, tray_height))
configure_board(get_or_add_component(prefab, unreal.StaticMeshComponent, "ArrowBoxFrontWall"),
                cube, wood, (-tray_length / 2.0 + board_thickness / 2.0, 0.0, tray_height / 2.0),
                (board_thickness, tray_width, tray_height))
configure_board(get_or_add_component(prefab, unreal.StaticMeshComponent, "ArrowBoxBackWall"),
                cube, wood, (tray_length / 2.0 - board_thickness / 2.0, 0.0, tray_height / 2.0),
                (board_thickness, tray_width, tray_height))

instances = get_or_add_component(prefab, unreal.InstancedStaticMeshComponent, "ArrowInstances")
instances.set_static_mesh(arrow_mesh)
instances.set_material(0, arrow_material)
instances.set_editor_property("mobility", unreal.ComponentMobility.STATIC)
instances.set_editor_property("cast_shadow", False)
instances.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
instances.set_editor_property("generate_overlap_events", False)
instances.set_cull_distance(10000.0)
instances.clear_instances()
for row in range(ROWS):
    for column in range(COLUMNS):
        location = unreal.Vector(
            0.0,
            -((COLUMNS - 1) * COLUMN_SPACING) / 2.0 + column * COLUMN_SPACING,
            base_thickness / 2.0 + 4.0 + row * ROW_SPACING,
        )
        instances.add_instance(unreal.Transform(location=location), False)

unreal.BlueprintEditorLibrary.compile_blueprint(prefab)
unreal.EditorAssetLibrary.save_loaded_asset(prefab, only_if_is_dirty=False)
unreal.log(
    "SINGIJEON_ARROW_BOX_PREFAB SUCCESS: "
    f"{prefab.get_path_name()} instances={ROWS * COLUMNS}"
)
