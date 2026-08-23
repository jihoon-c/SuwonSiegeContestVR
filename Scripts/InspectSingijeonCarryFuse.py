import unreal


def describe_component(prefix, component):
    if not component:
        unreal.log_warning(f"{prefix}=None")
        return
    values = [
        f"name={component.get_name()}",
        f"class={component.get_class().get_name()}",
        f"location={component.get_editor_property('relative_location')}",
    ]
    for prop in ("sphere_radius", "collision_profile_name", "generate_overlap_events", "auto_activate", "asset"):
        try:
            values.append(f"{prop}={component.get_editor_property(prop)}")
        except Exception:
            pass
    unreal.log(prefix + " " + " ".join(values))


hwacha_bp = unreal.load_asset("/GF_Singijeon/Gameplay/BP_SingijeonHwacha")
torch_bp = unreal.load_asset("/GF_Singijeon/Gameplay/BP_SingijeonTorch")
if not hwacha_bp or not torch_bp:
    raise RuntimeError("Singijeon Hwacha or Torch Blueprint is missing")

for blueprint in (hwacha_bp, torch_bp):
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)

hwacha = unreal.get_default_object(hwacha_bp.generated_class())
torch = unreal.get_default_object(torch_bp.generated_class())
body = hwacha.get_editor_property("body_mesh")
unreal.log(
    f"CARRY_FUSE_HWACHA_BODY mobility={body.get_editor_property('mobility')} "
    f"collision={body.get_collision_profile_name()}"
)
for property_name in (
        "left_handle_grab_point", "right_handle_grab_point",
        "left_handle_highlight", "right_handle_highlight"):
    component = hwacha.get_editor_property(property_name)
    unreal.log(
        f"CARRY_FUSE_HANDLE property={property_name} "
        f"location={component.get_editor_property('relative_location')} "
        f"tags={component.get_editor_property('component_tags')}"
    )
describe_component("CARRY_FUSE_HWACHA_FUSE", hwacha.get_editor_property("fuse"))
describe_component("CARRY_FUSE_HWACHA_FUSE_GUIDE", hwacha.get_editor_property("fuse_ignition_effect"))
describe_component("CARRY_FUSE_TORCH_AREA", torch.get_editor_property("ignition_area"))
unreal.log(f"CARRY_FUSE_TORCH_ACTIVE={torch.get_editor_property('ignition_active')}")

carry = hwacha.get_editor_property("two_hand_carry")
for prop in ("carry_enabled", "allow_single_hand_carry", "follow_hand_without_lag", "sweep_movement"):
    try:
        unreal.log(f"CARRY_FUSE_CARRY {prop}={carry.get_editor_property(prop)}")
    except Exception as error:
        unreal.log_warning(f"CARRY_FUSE_CARRY {prop}: {error}")

for effect in hwacha.get_components_by_class(unreal.NiagaraComponent):
    describe_component("CARRY_FUSE_HWACHA_EFFECT", effect)
for effect in torch.get_components_by_class(unreal.NiagaraComponent):
    describe_component("CARRY_FUSE_TORCH_EFFECT", effect)

unreal.log("SINGIJEON_CARRY_FUSE INSPECT SUCCESS")
