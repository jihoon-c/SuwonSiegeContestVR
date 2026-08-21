import unreal


errors = []


def check(condition, message):
    if condition:
        unreal.log(f"SINGIJEON_VERIFY PASS: {message}")
    else:
        errors.append(message)
        unreal.log_error(f"SINGIJEON_VERIFY FAIL: {message}")


hwacha_bp = unreal.load_asset("/GF_Singijeon/Gameplay/BP_SingijeonHwacha")
arrow_bp = unreal.load_asset("/GF_Singijeon/Gameplay/BP_SingijeonArrow")
torch_bp = unreal.load_asset("/GF_Singijeon/Gameplay/BP_SingijeonTorch")
scenario = unreal.load_asset("/Game/Data/DA_Scenario_Singijeon")

for blueprint, name in (
    (hwacha_bp, "BP_SingijeonHwacha"),
    (arrow_bp, "BP_SingijeonArrow"),
    (torch_bp, "BP_SingijeonTorch"),
):
    check(blueprint is not None, f"{name} loads")
    if blueprint:
        unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
        check(blueprint.generated_class() is not None, f"{name} has a generated class")

if hwacha_bp:
    cdo = unreal.get_default_object(hwacha_bp.generated_class())
    check(
        cdo.get_editor_property("body_mesh").get_editor_property("static_mesh") is not None,
        "Hwacha Blueprint uses the Hwacha mesh",
    )
    for property_name, target_id in (
        ("load_scenario_interactor", "Hwacha_Load"),
        ("ignite_scenario_interactor", "Hwacha_Fuse"),
        ("fire_scenario_interactor", "Hwacha_Fire"),
    ):
        interactor = cdo.get_editor_property(property_name)
        check(interactor is not None, f"Hwacha contains {property_name}")
        if interactor:
            check(
                str(interactor.get_editor_property("target_id")) == target_id,
                f"{property_name} uses TargetID {target_id}",
            )

for blueprint, name in ((arrow_bp, "Arrow"), (torch_bp, "Torch")):
    if blueprint:
        handles = unreal.get_engine_subsystem(
            unreal.SubobjectDataSubsystem
        ).k2_gather_subobject_data_for_blueprint(blueprint)
        names = [
            str(unreal.SubobjectDataBlueprintFunctionLibrary.get_variable_name(
                unreal.SubobjectDataBlueprintFunctionLibrary.get_data(handle)
            ))
            for handle in handles
        ]
        check("GrabPoint" in names, f"{name} Blueprint contains GrabPoint")

expected_flow = {
    "NAR_01": "NAR_02", "NAR_02": "NAR_03", "NAR_03": "NAR_04",
    "NAR_04": "NAR_05", "NAR_05": "INT_01", "INT_01": "NAR_06",
    "NAR_06": "NAR_07", "NAR_07": "NAR_08", "NAR_08": "INT_02",
    "INT_02": "NAR_09", "NAR_09": "NAR_10", "NAR_10": "INT_03",
    "INT_03": "NAR_11", "NAR_11": "NAR_12", "NAR_12": "INT_04",
    "INT_04": "NAR_13", "NAR_13": "INT_05", "INT_05": "NAR_14",
    "NAR_14": "NAR_15", "NAR_15": "INT_06", "INT_06": "NAR_16",
    "NAR_16": "INT_07", "INT_07": "NAR_17", "NAR_17": "NAR_18",
    "NAR_18": "NAR_19", "NAR_19": "NAR_20", "NAR_20": "NAR_21",
    "NAR_21": "None",
}
if scenario:
    stages = list(scenario.get_editor_property("stages"))
    stage = stages[0] if len(stages) == 1 else None
    check(stage is not None, "Scenario contains one inline Stage")
    interactions = {
        str(item.get_editor_property("interaction_id")): item
        for item in (stage.get_editor_property("interactions") if stage else [])
    }
    for interaction_id, next_id in expected_flow.items():
        check(interaction_id in interactions, f"Stage contains {interaction_id}")
        if interaction_id in interactions:
            actual_next = str(
                interactions[interaction_id].get_editor_property("next_interaction_id")
            )
            check(actual_next == next_id, f"{interaction_id} advances to {next_id}")

unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/LV_Singijeon")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
labels = {actor.get_actor_label() for actor in actors}
for label in (
    "PlayerStart_Singijeon",
    "BP_SingijeonHwacha_Playable",
    "BP_SingijeonArrow_Playable",
    "BP_SingijeonTorch_Playable",
    "BP_SingijeonFirePit_Playable",
):
    check(label in labels, f"LV_Singijeon contains {label}")

if errors:
    raise RuntimeError("Playable Singijeon verification failed: " + "; ".join(errors))
unreal.log("SINGIJEON_VERIFY SUCCESS")
