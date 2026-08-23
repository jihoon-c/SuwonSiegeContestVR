import unreal

subsystem = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
unreal.log("SKEL_LOD_API methods=" + ",".join(x for x in dir(subsystem) if "lod" in x.lower()))
for name in dir(subsystem):
    if "lod" in name.lower():
        method = getattr(subsystem, name)
        unreal.log(f"SKEL_LOD_API DOC {name}: {getattr(method, '__doc__', '')}")
mesh = unreal.load_asset("/GF_Singijeon/Asset/EnemyMan1/Low_Poly_Samurai")
unreal.log(f"SKEL_LOD_API count={subsystem.get_lod_count(mesh)}")
