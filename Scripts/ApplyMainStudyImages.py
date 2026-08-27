import unreal


SCENARIO_PATH = "/Game/Data/DA_Scenario_MainEducation"
EXPECTED_IMAGES = {
    "GEOJUNGGI_IMAGE": "/Game/Art/MainEducation/Examples/T_MainEdu_PulleyComparison_Example.T_MainEdu_PulleyComparison_Example",
    "NOKRO_IMAGE": "/Game/Art/MainEducation/study/study_nokro.study_nokro",
    "GONGSIMDON_IMAGE": "/Game/Art/MainEducation/study/study_gongsimdon.study_gongsimdon",
    "SINGIJEON_IMAGE": "/Game/Art/MainEducation/study/study_singijeon.study_singijeon",
    "ONGSEONG_IMAGE": "/Game/Art/MainEducation/study/study_ongsung.study_ongsung",
}


scenario = unreal.load_asset(SCENARIO_PATH)
if not isinstance(scenario, unreal.MainEducationScenarioDefinition):
    raise RuntimeError("DA_Scenario_MainEducation is missing or has the wrong class")

# Rebuild the data asset from the current project-owned defaults, including the
# separate Geojunggi and Nokro presentation steps.
scenario.reset_to_main_gate_presentation_flow()

editor_flow = list(scenario.get_editor_property("editor_flow"))
geo_stage = next(
    (stage for stage in editor_flow if str(stage.get_editor_property("stage_id")) == "GEO_NOKRO"),
    None,
)
if geo_stage is None:
    raise RuntimeError("GEO_NOKRO stage is missing")

geo_step_ids = [str(step.get_editor_property("step_id")) for step in geo_stage.get_editor_property("steps")]
if geo_step_ids != ["GEOJUNGGI_IMAGE", "NOKRO_IMAGE"]:
    raise RuntimeError("Geojunggi/Nokro steps are not separately ordered: " + repr(geo_step_ids))

found_images = {}
for stage in editor_flow:
    for step in stage.get_editor_property("steps"):
        content = step.get_editor_property("content")
        content_id = str(content.get_editor_property("content_id"))
        if content_id in EXPECTED_IMAGES:
            image = content.get_editor_property("image")
            actual_path = image.get_path_name() if image else ""
            found_images[content_id] = actual_path
            if actual_path != EXPECTED_IMAGES[content_id]:
                raise RuntimeError(
                    "Unexpected image for {}: {}".format(content_id, actual_path)
                )

if set(found_images) != set(EXPECTED_IMAGES):
    raise RuntimeError("Missing image content: " + repr(sorted(set(EXPECTED_IMAGES) - set(found_images))))

scenario.rebuild_scenario_from_editor_flow()
if not unreal.EditorAssetLibrary.save_loaded_asset(scenario, only_if_is_dirty=False):
    raise RuntimeError("Could not save DA_Scenario_MainEducation")

unreal.log("MAIN_STUDY_IMAGES_APPLIED: " + repr(found_images))

