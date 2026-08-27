import unreal


LEVEL_PATH = "/Game/Maps/LV_Singijeon"
SOURCE_MESH_PATH = (
    "/GF_OngseongCrossbow/Art/Namhansanseong/Fortification/JihwaGate/"
    "SM_Battlement01a6"
)

source_mesh = unreal.load_asset(SOURCE_MESH_PATH)
if not isinstance(source_mesh, unreal.StaticMesh):
    raise RuntimeError(f"Missing source mesh: {SOURCE_MESH_PATH}")

unreal.load_module("StaticMeshEditor")
mesh_editor = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
world = unreal.EditorLoadingAndSavingUtils.load_map(LEVEL_PATH)
if not world:
    raise RuntimeError(f"Could not load level: {LEVEL_PATH}")

references = []
for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    for component in actor.get_components_by_class(unreal.StaticMeshComponent):
        if component.get_editor_property("static_mesh") == source_mesh:
            references.append(
                f"{actor.get_actor_label()}:{component.get_name()}:"
                f"mobility={component.get_editor_property('mobility')}:"
                f"collision={component.get_editor_property('collision_enabled')}:"
                f"shadow={component.get_editor_property('cast_shadow')}"
            )

materials = []
for index in range(source_mesh.get_num_sections(0)):
    material = source_mesh.get_material(index)
    materials.append(material.get_path_name() if material else "None")

unreal.log(
    "BATTLEMENT01A6 INSPECT SUCCESS: "
    f"lods={mesh_editor.get_lod_count(source_mesh)} "
    f"sections_lod0={source_mesh.get_num_sections(0)} "
    f"materials={materials} references={references}"
)
