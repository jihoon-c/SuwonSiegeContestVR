import unreal


bp = unreal.load_asset("/GF_Singijeon/Gameplay/BP_SingijeonHwacha")
if not bp:
    raise RuntimeError("BP_SingijeonHwacha is missing")

unreal.BlueprintEditorLibrary.compile_blueprint(bp)
cdo = unreal.get_default_object(bp.generated_class())
subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
bfl = unreal.SubobjectDataBlueprintFunctionLibrary
for handle in subsystem.k2_gather_subobject_data_for_blueprint(bp):
    data = bfl.get_data(handle)
    obj = bfl.get_object(data)
    unreal.log(
        f"HWACHA_SUBOBJECT name={bfl.get_variable_name(data)} "
        f"object={obj} class={obj.get_class().get_name() if obj else None}"
    )
    if obj and "GrabComponent" in obj.get_class().get_name():
        for prop in ("grab_type", "b_is_held"):
            try:
                unreal.log(f"HWACHA_GRAB_PROP {obj.get_name()} {prop}={obj.get_editor_property(prop)}")
            except Exception as error:
                unreal.log_warning(f"HWACHA_GRAB_PROP {obj.get_name()} {prop}: {error}")

unreal.log(f"HWACHA_SLOT_COMPONENTS={len(cdo.get_components_by_class(unreal.SingijeonAmmoSlotComponent))}")
unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/LV_Singijeon")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
arrows = [actor for actor in actors if isinstance(actor, unreal.SingijeonProjectileActor)]
hwachas = [actor for actor in actors if isinstance(actor, unreal.SingijeonHwachaActor)]
unreal.log(f"HWACHA_LEVEL_ARROW_ACTORS={len(arrows)}")
unreal.log(f"HWACHA_LEVEL_HWACHA_ACTORS={len(hwachas)}")
unreal.log("SINGIJEON_HWACHA_GRAB_LOAD INSPECT SUCCESS")
