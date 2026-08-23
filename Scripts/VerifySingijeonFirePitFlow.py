import unreal


errors = []


def check(condition, message):
    if condition:
        unreal.log(f"SINGIJEON_FIREPIT_VERIFY PASS: {message}")
    else:
        errors.append(message)
        unreal.log_error(f"SINGIJEON_FIREPIT_VERIFY FAIL: {message}")


torch_bp = unreal.load_asset("/GF_Singijeon/Gameplay/BP_SingijeonTorch")
firepit_bp = unreal.load_asset("/GF_Singijeon/Gameplay/BP_SingijeonFirePit")
firepit_mesh = unreal.load_asset("/GF_Singijeon/Asset/FirePit/Geometric_Fire_Pit")
fire_system = unreal.load_asset("/Game/NiagaraExamples/FX_Misc/NS_Fire")
scenario = unreal.load_asset("/Game/Data/DA_Scenario_Singijeon")
check(torch_bp is not None, "BP_SingijeonTorch loads")
check(firepit_bp is not None, "BP_SingijeonFirePit loads")
check(firepit_mesh is not None, "Fire Pit mesh loads")
check(fire_system is not None, "NS_Fire loads")

for blueprint, label in ((torch_bp, "Torch"), (firepit_bp, "Fire Pit")):
    if not blueprint:
        continue
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    handles = unreal.get_engine_subsystem(
        unreal.SubobjectDataSubsystem
    ).k2_gather_subobject_data_for_blueprint(blueprint)
    components = [
        unreal.SubobjectDataBlueprintFunctionLibrary.get_object(
            unreal.SubobjectDataBlueprintFunctionLibrary.get_data(handle)
        )
        for handle in handles
    ]
    effects = [item for item in components if isinstance(item, unreal.NiagaraComponent)]
    check(len(effects) > 0, f"{label} contains a Niagara fire effect")
    if effects:
        check(effects[0].get_editor_property("asset") == fire_system,
              f"{label} Niagara uses NS_Fire")

if torch_bp:
    torch_cdo = unreal.get_default_object(torch_bp.generated_class())
    check(not torch_cdo.get_editor_property("ignition_active"),
          "Torch starts unlit")

if firepit_bp:
    firepit_cdo = unreal.get_default_object(firepit_bp.generated_class())
    check(
        firepit_cdo.get_editor_property("fire_pit_mesh").get_editor_property(
            "static_mesh"
        ) == firepit_mesh,
        "Fire Pit Blueprint uses Geometric_Fire_Pit mesh",
    )
    check(firepit_mesh.get_editor_property("allow_cpu_access"),
          "Fire Pit mesh allows CPU access for Niagara location sampling")
    interactor = firepit_cdo.get_editor_property("ignite_torch_scenario_interactor")
    check(str(interactor.get_editor_property("target_id")) == "Torch_Ignite",
          "Fire Pit reports Torch_Ignite")

if scenario:
    stages = list(scenario.get_editor_property("stages"))
    interactions = {
        str(item.get_editor_property("interaction_id")): item
        for item in (stages[0].get_editor_property("interactions") if stages else [])
    }
    check(str(interactions["INT_04"].get_editor_property("next_interaction_id")) == "NAR_13",
          "Torch Grab advances toward Torch ignition")
    check(str(interactions["INT_05"].get_editor_property("target_id")) == "Torch_Ignite",
          "INT_05 waits for Fire Pit torch ignition")
    check(str(interactions["INT_06"].get_editor_property("target_id")) == "Hwacha_Fuse",
          "INT_06 waits for lit torch at Hwacha Fuse")

unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/LV_Singijeon")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
firepit_actor = next(
    (actor for actor in actors if actor.get_actor_label() == "BP_SingijeonFirePit_Playable"),
    None,
)
check(firepit_actor is not None, "LV_Singijeon contains playable Fire Pit")
if firepit_actor:
    fire_effects = firepit_actor.get_components_by_class(unreal.NiagaraComponent)
    fire_effect = next((item for item in fire_effects if item.get_name() == "FireEffect"), None)
    check(fire_effect is not None, "Playable Fire Pit has its FireEffect component")
    if fire_effect:
        check(not fire_effect.get_editor_property("absolute_location"),
              "Playable FireEffect follows the Fire Pit transform")
        check(fire_effect.get_world_location().z >= 200.0,
              "Playable FireEffect is positioned above the Fire Pit rim")

if errors:
    raise RuntimeError("Singijeon Fire Pit verification failed: " + "; ".join(errors))
unreal.log("SINGIJEON_FIREPIT_FLOW VERIFY SUCCESS")
