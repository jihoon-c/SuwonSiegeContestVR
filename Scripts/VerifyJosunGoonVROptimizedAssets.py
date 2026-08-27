import unreal


MESH_PATH = "/GF_Singijeon/Gameplay/Characters/JosunGoon/SKM_JosunGoon_VR"
POINT_PATH = "/GF_Singijeon/Gameplay/Characters/JosunGoon/A_JosunGoon_Point_VR"
KNEEL_PATH = "/GF_Singijeon/Gameplay/Characters/JosunGoon/A_JosunGoon_Kneel_VR"
failures = []


def check(condition, message):
    if condition:
        unreal.log(f"[PASS] {message}")
    else:
        unreal.log_error(f"[FAIL] {message}")
        failures.append(message)


mesh = unreal.load_asset(MESH_PATH)
point = unreal.load_asset(POINT_PATH)
kneel = unreal.load_asset(KNEEL_PATH)
check(isinstance(mesh, unreal.SkeletalMesh), "SKM_JosunGoon_VR loads")
check(isinstance(point, unreal.AnimSequence), "Point VR animation loads")
check(isinstance(kneel, unreal.AnimSequence), "Kneel VR animation loads")

if isinstance(mesh, unreal.SkeletalMesh):
    mesh_editor = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
    common_skeleton = mesh.get_editor_property("skeleton")
    check(mesh_editor.get_lod_count(mesh) == 3, "VR mesh has three LODs")
    check(len(mesh.get_editor_property("materials")) > 0, "VR mesh keeps its material")
    for lod_index in range(mesh_editor.get_lod_count(mesh)):
        settings = mesh_editor.get_lod_build_settings(mesh, lod_index)
        check(
            settings.get_editor_property("optimize_for_instancing"),
            f"LOD{lod_index} is optimized for instancing",
        )
    if isinstance(point, unreal.AnimSequence) and isinstance(kneel, unreal.AnimSequence):
        check(point.get_editor_property("skeleton") == common_skeleton,
              "Point animation uses the common mesh Skeleton")
        check(kneel.get_editor_property("skeleton") == common_skeleton,
              "Kneel animation uses the common mesh Skeleton")
        check(point.get_play_length() > 0.0, "Point animation contains motion")
        check(kneel.get_play_length() > 0.0, "Kneel animation contains motion")

if failures:
    raise RuntimeError("; ".join(failures))
unreal.log("JOSUN_GOON_VR_VERIFY SUCCESS")

