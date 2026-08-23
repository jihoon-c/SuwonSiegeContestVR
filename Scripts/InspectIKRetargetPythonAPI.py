import unreal

for name in ("IKRigController", "IKRetargeterController", "IKRetargetBatchOperation",
             "IKRigDefinition", "IKRetargeter", "RetargetChainSettings"):
    cls = getattr(unreal, name, None)
    unreal.log(f"IK_API {name}={cls}")
    if cls:
        unreal.log(f"IK_API {name} methods={','.join(x for x in dir(cls) if not x.startswith('_'))}")

for label, symbol in (
    ("apply_auto", unreal.IKRigController.apply_auto_generated_retarget_definition),
    ("set_ik_rig", unreal.IKRetargeterController.set_ik_rig),
    ("add_default_ops", unreal.IKRetargeterController.add_default_ops),
    ("duplicate_and_retarget", unreal.IKRetargetBatchOperation.duplicate_and_retarget)):
    unreal.log(f"IK_API DOC {label}: {symbol.__doc__}")
unreal.log(f"IK_API ENUM RetargetSourceOrTarget={list(unreal.RetargetSourceOrTarget)}")
unreal.log("IK_API RETARGET SYMBOLS=" + ",".join(
    name for name in dir(unreal) if "Retarget" in name or "IKRigDefinitionFactory" in name))
