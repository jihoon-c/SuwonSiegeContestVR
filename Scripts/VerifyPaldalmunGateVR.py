"""Read-only verification of the VR Paldalmun mesh and L_Main references."""

import unreal


SOURCE_MESH_PATH = (
    "/Game/Art/Model/KCISA-수원화성_팔달문_PaldalmunGate_/StaticMeshes/"
    "KCISA-수원화성_팔달문_PaldalmunGate_"
)
VR_MESH_PATH = "/Game/Art/Model/VR_Optimized/SM_PaldalmunGate_VR"
MAIN_LEVEL_PATH = "/Game/Maps/Main/L_Main"

source_mesh = unreal.load_asset(SOURCE_MESH_PATH)
vr_mesh = unreal.load_asset(VR_MESH_PATH)
if not isinstance(source_mesh, unreal.StaticMesh) or not isinstance(vr_mesh, unreal.StaticMesh):
    raise RuntimeError("Source or VR Paldalmun mesh is missing")

unreal.load_module("StaticMeshEditor")
mesh_editor = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
lod_count = mesh_editor.get_lod_count(vr_mesh)
if lod_count < 3:
    raise RuntimeError(f"VR mesh must have at least three LODs; got {lod_count}")

world = unreal.EditorLoadingAndSavingUtils.load_map(MAIN_LEVEL_PATH)
source_references = []
vr_references = []
for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    for component in actor.get_components_by_class(unreal.StaticMeshComponent):
        mesh = component.get_editor_property("static_mesh")
        if mesh == source_mesh:
            source_references.append(f"{actor.get_name()}:{component.get_name()}")
        elif mesh == vr_mesh:
            vr_references.append(f"{actor.get_name()}:{component.get_name()}")

if source_references:
    raise RuntimeError(f"Original Paldalmun mesh still used in L_Main: {source_references}")
if not vr_references:
    raise RuntimeError("No L_Main component uses the VR Paldalmun mesh")

unreal.log(
    f"PALDALMUN_VR VERIFY SUCCESS: lod_count={lod_count} "
    f"vr_references={vr_references}"
)
