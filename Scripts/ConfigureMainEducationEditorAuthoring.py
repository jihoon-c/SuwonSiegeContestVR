import os

import unreal


SOURCE_DIRECTORY = os.path.join(
    unreal.Paths.project_content_dir(), "Art", "MainEducation", "Examples"
)
DESTINATION_DIRECTORY = "/Game/Art/MainEducation/Examples"
SCENARIO_PATH = "/Game/Data/DA_Scenario_MainEducation"

TEXTURES = (
    "T_MainEdu_Overview_Example",
    "T_MainEdu_WallDefense_Example",
    "T_MainEdu_GongsimdonCutaway_Example",
    "T_MainEdu_OngseongPlan_Example",
    "T_MainEdu_PulleyComparison_Example",
)

CONTENT_IMAGE_MAP = {
    "INTRO_OVERVIEW": "T_MainEdu_Overview_Example",
    "SUMMARY_IMAGE": "T_MainEdu_Overview_Example",
    "DEFENSE_IMAGE": "T_MainEdu_WallDefense_Example",
    "GONG_IMAGE_01": "T_MainEdu_GongsimdonCutaway_Example",
    "GONG_IMAGE_02": "T_MainEdu_GongsimdonCutaway_Example",
    "ONG_IMAGE_01": "T_MainEdu_OngseongPlan_Example",
    "ONG_IMAGE_02": "T_MainEdu_OngseongPlan_Example",
    "NOKRO_IMAGE_01": "T_MainEdu_PulleyComparison_Example",
    "NOKRO_IMAGE_02": "T_MainEdu_PulleyComparison_Example",
    "GEO_IMAGE_01": "T_MainEdu_PulleyComparison_Example",
    "GEO_IMAGE_02": "T_MainEdu_PulleyComparison_Example",
    "PULLEY_COMPARE": "T_MainEdu_PulleyComparison_Example",
}


def import_example_texture(asset_name):
    asset_path = f"{DESTINATION_DIRECTORY}/{asset_name}"
    texture = unreal.load_asset(asset_path)
    if not texture:
        filename = os.path.join(SOURCE_DIRECTORY, asset_name + ".png")
        if not os.path.isfile(filename):
            raise RuntimeError(f"Missing example PNG: {filename}")
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", filename)
        task.set_editor_property("destination_path", DESTINATION_DIRECTORY)
        task.set_editor_property("destination_name", asset_name)
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", False)
        task.set_editor_property("save", True)
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        texture = unreal.load_asset(asset_path)
    if not isinstance(texture, unreal.Texture2D):
        raise RuntimeError(f"Example Texture2D import failed: {asset_path}")
    texture.set_editor_property("srgb", True)
    unreal.EditorAssetLibrary.save_loaded_asset(texture, only_if_is_dirty=False)
    return texture


textures = {name: import_example_texture(name) for name in TEXTURES}
scenario = unreal.load_asset(SCENARIO_PATH)
if not isinstance(scenario, unreal.MainEducationScenarioDefinition):
    raise RuntimeError("DA_Scenario_MainEducation is missing or has the wrong class")

editor_flow = list(scenario.get_editor_property("editor_flow"))
if not editor_flow:
    scenario_cdo = unreal.get_default_object(scenario.get_class())
    editor_flow = list(scenario_cdo.get_editor_property("editor_flow"))

assigned_content_ids = set()
for stage in editor_flow:
    steps = list(stage.get_editor_property("steps"))
    for step in steps:
        if step.get_editor_property("step_type") == unreal.MainEducationAuthoringStepType.TRAVEL:
            continue
        content = step.get_editor_property("content")
        content_id = str(content.get_editor_property("content_id"))
        texture_name = CONTENT_IMAGE_MAP.get(content_id)
        if texture_name:
            # Do not replace a designer's final art if this script is run again.
            if not content.get_editor_property("image"):
                content.set_editor_property("image", textures[texture_name])
            if not content.get_editor_property("editor_notes"):
                content.set_editor_property(
                    "editor_notes",
                    "AI 생성 예시 이미지입니다. 최종 고증 이미지로 교체 가능합니다.",
                )
            step.set_editor_property("content", content)
            assigned_content_ids.add(content_id)
    stage.set_editor_property("steps", steps)

missing_assignments = sorted(set(CONTENT_IMAGE_MAP) - assigned_content_ids)
if missing_assignments:
    raise RuntimeError(
        "Editor Flow is missing image content IDs: " + ", ".join(missing_assignments)
    )

scenario.set_editor_property("editor_flow", editor_flow)
scenario.rebuild_scenario_from_editor_flow()
if not unreal.EditorAssetLibrary.save_loaded_asset(scenario, only_if_is_dirty=False):
    raise RuntimeError("Could not save DA_Scenario_MainEducation")

unreal.log(
    "MAIN_EDITOR_AUTHORING CONFIGURE SUCCESS: "
    f"{len(editor_flow)} stages, {len(assigned_content_ids)} image slots, {len(textures)} example textures"
)

