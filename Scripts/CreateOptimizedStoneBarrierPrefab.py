import unreal


ROOT = "/GF_Singijeon/Asset/NamhansanseongWall/Optimized"
TEXTURE_DIR = ROOT + "/Textures"
MATERIAL_DIR = ROOT + "/Materials"
PREFAB_DIR = ROOT + "/Prefabs"
SOURCE_MESH_PATH = "/GF_Singijeon/Asset/NamhansanseongWall/Meshes/SM_Stone_Barrier"
SOURCE_TEXTURE_PATH = "/GF_Singijeon/Asset/NamhansanseongWall/Textures/T_Stone_Barrier01a_BC"
LOW_TEXTURE_NAME = "T_Stone_Barrier01a_BC_Low"
LOW_MATERIAL_NAME = "M_Stone_Barrier_Low"
PREFAB_NAME = "BP_Stone_Barrier_Optimized"

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
subobjects = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
subobject_bfl = unreal.SubobjectDataBlueprintFunctionLibrary


def load_required(path):
    asset = unreal.load_asset(path)
    if not asset:
        raise RuntimeError(f"Required asset is missing: {path}")
    return asset


def find_component(blueprint, component_name):
    for handle in subobjects.k2_gather_subobject_data_for_blueprint(blueprint):
        data = subobject_bfl.get_data(handle)
        component = subobject_bfl.get_object(data)
        if component and (component.get_name() == component_name or
                          isinstance(component, unreal.StaticMeshComponent)):
            return component
    return None


for folder in (ROOT, TEXTURE_DIR, MATERIAL_DIR, PREFAB_DIR):
    unreal.EditorAssetLibrary.make_directory(folder)

source_mesh = load_required(SOURCE_MESH_PATH)
source_texture = load_required(SOURCE_TEXTURE_PATH)
low_texture = unreal.load_asset(f"{TEXTURE_DIR}/{LOW_TEXTURE_NAME}")
if not low_texture:
    low_texture = asset_tools.duplicate_asset(LOW_TEXTURE_NAME, TEXTURE_DIR, source_texture)
if not low_texture:
    raise RuntimeError("Could not duplicate the low-resolution wall Base Color texture")
low_texture.set_editor_property("max_texture_size", 1024)
low_texture.set_editor_property("srgb", True)
unreal.EditorAssetLibrary.save_loaded_asset(low_texture, only_if_is_dirty=False)

low_material = unreal.load_asset(f"{MATERIAL_DIR}/{LOW_MATERIAL_NAME}")
if not low_material:
    low_material = asset_tools.create_asset(
        LOW_MATERIAL_NAME, MATERIAL_DIR, unreal.Material, unreal.MaterialFactoryNew()
    )
if not low_material:
    raise RuntimeError("Could not create the optimized wall material")

unreal.MaterialEditingLibrary.delete_all_material_expressions(low_material)
base_color = unreal.MaterialEditingLibrary.create_material_expression(
    low_material, unreal.MaterialExpressionTextureSample, -300, 0
)
base_color.set_editor_property("texture", low_texture)
unreal.MaterialEditingLibrary.connect_material_property(
    base_color, "RGB", unreal.MaterialProperty.MP_BASE_COLOR
)
low_material.set_editor_property("fully_rough", True)
low_material.set_editor_property("two_sided", False)
unreal.MaterialEditingLibrary.recompile_material(low_material)
unreal.EditorAssetLibrary.save_loaded_asset(low_material, only_if_is_dirty=False)

prefab = unreal.load_asset(f"{PREFAB_DIR}/{PREFAB_NAME}")
if not prefab:
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", unreal.Actor)
    prefab = asset_tools.create_asset(PREFAB_NAME, PREFAB_DIR, unreal.Blueprint, factory)
if not prefab:
    raise RuntimeError("Could not create the optimized wall prefab")

mesh_component = find_component(prefab, "StoneBarrierMesh")
if not mesh_component:
    handles = subobjects.k2_gather_subobject_data_for_blueprint(prefab)
    root_handle = next(
        (handle for handle in handles if subobject_bfl.is_root_component(
            subobject_bfl.get_data(handle))), handles[0]
    )
    params = unreal.AddNewSubobjectParams(
        parent_handle=root_handle,
        new_class=unreal.StaticMeshComponent,
        blueprint_context=prefab,
    )
    handle, reason = subobjects.add_new_subobject(params)
    if not subobject_bfl.is_handle_valid(handle):
        raise RuntimeError(f"Could not add wall mesh component: {reason}")
    subobjects.rename_subobject(handle, unreal.Text("StoneBarrierMesh"))
    mesh_component = subobject_bfl.get_object(subobject_bfl.get_data(handle))

mesh_component.set_static_mesh(source_mesh)
for material_index in range(max(1, len(source_mesh.get_editor_property("static_materials")))):
    mesh_component.set_material(material_index, low_material)
mesh_component.set_editor_property("mobility", unreal.ComponentMobility.STATIC)
mesh_component.set_editor_property("cast_shadow", True)
mesh_component.set_collision_profile_name("BlockAll")
mesh_component.set_cull_distance(3500.0)

unreal.BlueprintEditorLibrary.compile_blueprint(prefab)
unreal.EditorAssetLibrary.save_loaded_asset(prefab, only_if_is_dirty=False)
unreal.log(
    "OPTIMIZED_STONE_BARRIER_PREFAB SUCCESS: "
    f"{prefab.get_path_name()} texture={low_texture.get_path_name()}"
)
