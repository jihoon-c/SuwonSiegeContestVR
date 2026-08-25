import unreal


SCENARIO_PATH = "/Game/Data/DA_Scenario_MainEducation"
TEXTURE_ROOT = "/Game/Art/MainEducation/Examples"
TEXTURES = (
    "T_MainEdu_Overview_Example",
    "T_MainEdu_WallDefense_Example",
    "T_MainEdu_GongsimdonCutaway_Example",
    "T_MainEdu_OngseongPlan_Example",
    "T_MainEdu_PulleyComparison_Example",
)
IMAGE_CONTENT_IDS = {
    "INTRO_OVERVIEW", "SUMMARY_IMAGE", "DEFENSE_IMAGE",
    "GONG_IMAGE_01", "GONG_IMAGE_02", "ONG_IMAGE_01", "ONG_IMAGE_02",
    "NOKRO_IMAGE_01", "NOKRO_IMAGE_02", "GEO_IMAGE_01", "GEO_IMAGE_02",
    "PULLEY_COMPARE",
}

for texture_name in TEXTURES:
    if not isinstance(unreal.load_asset(f"{TEXTURE_ROOT}/{texture_name}"), unreal.Texture2D):
        raise RuntimeError(f"Missing example texture: {texture_name}")

scenario = unreal.load_asset(SCENARIO_PATH)
if not isinstance(scenario, unreal.MainEducationScenarioDefinition):
    raise RuntimeError("Main education Scenario is missing")

editor_flow = list(scenario.get_editor_property("editor_flow"))
runtime_stages = list(scenario.get_editor_property("stages"))
if len(editor_flow) != 6 or len(editor_flow) != len(runtime_stages):
    raise RuntimeError(
        f"Expected six matching Editor/Runtime stages, got {len(editor_flow)}/{len(runtime_stages)}"
    )

image_content_found = set()
step_count = 0
for stage_index, editor_stage in enumerate(editor_flow):
    editor_steps = list(editor_stage.get_editor_property("steps"))
    runtime_interactions = list(runtime_stages[stage_index].get_editor_property("interactions"))
    if len(editor_steps) != len(runtime_interactions):
        raise RuntimeError(f"Step count mismatch in stage index {stage_index}")
    for step_index, step in enumerate(editor_steps):
        step_count += 1
        step_id = str(step.get_editor_property("step_id"))
        if step_id != str(runtime_interactions[step_index].get_editor_property("interaction_id")):
            raise RuntimeError(f"Step ID mismatch at {stage_index}/{step_index}")
        expected_next = (
            str(editor_steps[step_index + 1].get_editor_property("step_id"))
            if step_index + 1 < len(editor_steps) else "None"
        )
        actual_next = str(runtime_interactions[step_index].get_editor_property("next_interaction_id"))
        if actual_next != expected_next:
            raise RuntimeError(f"Next ID mismatch for {step_id}: {actual_next} != {expected_next}")
        if not step.get_editor_property("player_action") or not step.get_editor_property("completion_condition"):
            raise RuntimeError(f"Designer action/completion description is empty: {step_id}")
        if step.get_editor_property("step_type") != unreal.MainEducationAuthoringStepType.TRAVEL:
            content = step.get_editor_property("content")
            if not content.get_editor_property("interaction_guide_text"):
                raise RuntimeError(f"Player guide is empty: {step_id}")
            content_id = str(content.get_editor_property("content_id"))
            if content_id in IMAGE_CONTENT_IDS:
                if not content.get_editor_property("image"):
                    raise RuntimeError(f"Example image is empty: {content_id}")
                image_content_found.add(content_id)

if image_content_found != IMAGE_CONTENT_IDS:
    raise RuntimeError(f"Image content set mismatch: {sorted(image_content_found)}")

unreal.log(
    "MAIN_EDITOR_AUTHORING VERIFY SUCCESS: "
    f"{len(editor_flow)} stages, {step_count} steps, {len(image_content_found)} image slots, {len(TEXTURES)} textures"
)

