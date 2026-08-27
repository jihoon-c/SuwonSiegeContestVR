import unreal


ASSETS = [
    "/GF_Singijeon/Asset/JosunGoonKneel/Kneeling",
    "/GF_Singijeon/Asset/JosunGoonKneel/Kneeling_Skeleton",
    "/GF_Singijeon/Asset/JosunGoonKneel/Kneeling_Anim",
    "/GF_Singijeon/Asset/JosunGoonPoint/Pointing",
    "/GF_Singijeon/Asset/JosunGoonPoint/Pointing_Skeleton",
    "/GF_Singijeon/Asset/JosunGoonPoint/Pointing_Anim",
]

mesh_editor = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)

for path in ASSETS:
    asset = unreal.load_asset(path)
    if not asset:
        unreal.log_error(f"JOSUN_INSPECT missing={path}")
        continue
    unreal.log(f"JOSUN_INSPECT asset={path} class={asset.get_class().get_name()}")
    if isinstance(asset, unreal.SkeletalMesh):
        skeleton = asset.get_editor_property("skeleton")
        physics = asset.get_editor_property("physics_asset")
        unreal.log(
            f"JOSUN_INSPECT mesh={path} skeleton={skeleton.get_path_name() if skeleton else None} "
            f"physics={physics.get_path_name() if physics else None} lods={mesh_editor.get_lod_count(asset)} "
            f"materials={len(asset.get_editor_property('materials'))} bounds={asset.get_bounds()}"
        )
    elif isinstance(asset, unreal.AnimSequence):
        skeleton = asset.get_editor_property("skeleton")
        unreal.log(
            f"JOSUN_INSPECT anim={path} skeleton={skeleton.get_path_name() if skeleton else None} "
            f"length={asset.get_play_length()} rate={asset.get_editor_property('rate_scale')}"
        )
for skeleton_path in (
    "/GF_Singijeon/Asset/JosunGoonKneel/Kneeling_Skeleton",
    "/GF_Singijeon/Asset/JosunGoonPoint/Pointing_Skeleton",
):
    skeleton = unreal.load_asset(skeleton_path)
    methods = [name for name in dir(skeleton) if "bone" in name.lower() or "reference" in name.lower()]
    unreal.log(f"JOSUN_INSPECT skeleton_methods={skeleton_path}:{methods}")
