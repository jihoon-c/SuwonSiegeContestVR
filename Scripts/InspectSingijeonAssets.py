import unreal


for path in (
    "/GF_Singijeon/Asset/Wooden_Rocket_Cart",
    "/GF_Singijeon/Asset/Hwacha/Wooden_Rocket_Cart",
    "/Game/XRFramework/Blueprints/BP_GrabComponent",
):
    asset = unreal.load_asset(path)
    unreal.log(f"SINGIJEON_ASSET {path}: {asset} class={asset.get_class() if asset else None}")
    if isinstance(asset, unreal.StaticMesh):
        unreal.log(f"SINGIJEON_BOUNDS {path}: {asset.get_bounds()}")

subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
unreal.log(f"SINGIJEON_SUBOBJECT_METHODS {dir(subsystem)}")
unreal.log(f"SINGIJEON_BFL_METHODS {dir(unreal.BlueprintEditorLibrary)}")
unreal.log(f"SINGIJEON_ADD_DOC {unreal.SubobjectDataSubsystem.add_new_subobject.__doc__}")
unreal.log(f"SINGIJEON_CREATE_BP_COMP_DOC {unreal.SubobjectDataSubsystem.create_new_bp_component.__doc__}")
unreal.log(f"SINGIJEON_ADD_PARAMS_DOC {unreal.AddNewSubobjectParams.__doc__}")
params = unreal.AddNewSubobjectParams()
unreal.log(f"SINGIJEON_ADD_PARAMS_DIR {dir(params)}")
unreal.log(f"SINGIJEON_SUBOBJECT_BFL_METHODS {dir(unreal.SubobjectDataBlueprintFunctionLibrary)}")
grab_bp = unreal.load_asset("/Game/XRFramework/Blueprints/BP_GrabComponent")
if grab_bp and grab_bp.generated_class():
    grab_cdo = unreal.get_default_object(grab_bp.generated_class())
    unreal.log(f"SINGIJEON_GRAB_CDO {grab_cdo}")
    for prop in ("grab_type", "b_is_held", "attach_parent_to_motion_controller"):
        try:
            unreal.log(f"SINGIJEON_GRAB_PROP {prop}={grab_cdo.get_editor_property(prop)}")
        except Exception as error:
            unreal.log_warning(f"SINGIJEON_GRAB_PROP {prop}: {error}")
