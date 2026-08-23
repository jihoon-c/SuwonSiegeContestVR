import unreal

paths = [
    "/GF_Singijeon/Asset/EnemyMan1/Low_Poly_Samurai",
    "/GF_Singijeon/Asset/EnemyMan1/Low_Poly_Samurai_Anim",
    "/Game/NiagaraExamples/Gallery/SkeletalMesh/Mannequins/Anims/Rifle/Jog/MF_Rifle_Jog_Fwd",
    "/GF_Singijeon/Gameplay/Enemy/DA_SingijeonEnemyRun_GPU",
]

for path in paths:
    asset = unreal.load_asset(path)
    if not asset:
        unreal.log_error(f"SAMURAI_INSPECT missing {path}")
        continue
    unreal.log(f"SAMURAI_INSPECT asset={asset.get_path_name()} class={asset.get_class().get_name()}")
    for prop in ("skeleton", "preview_skeletal_mesh", "retarget_source_asset", "number_of_sampled_keys"):
        try:
            value = asset.get_editor_property(prop)
            unreal.log(f"SAMURAI_INSPECT {path} {prop}={value}")
        except Exception:
            pass
    if isinstance(asset, unreal.SkeletalMesh):
        unreal.log(f"SAMURAI_INSPECT {path} bounds={asset.get_bounds()}")
        skeleton = asset.get_editor_property("skeleton")
        if skeleton:
            unreal.log(f"SAMURAI_INSPECT {path} skeleton={skeleton.get_path_name()}")
