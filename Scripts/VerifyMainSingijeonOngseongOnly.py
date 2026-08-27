import unreal


MAIN_LEVEL_PATH = "/Game/Maps/Main/L_Main"
MAIN_SCENARIO_PATH = "/Game/Data/DA_Scenario_MainEducation"
MAIN_NARRATION_TABLE_PATH = "/Game/Audio/Narration/DT_Narration_Main"
REMOVED_TOKENS = ("gongsimdon", "nokro", "geojunggi", "singijeon", "ongseong")


def check(condition, message):
    if not condition:
        raise RuntimeError(message)


scenario = unreal.load_asset(MAIN_SCENARIO_PATH)
check(scenario is not None, f"Missing Main scenario: {MAIN_SCENARIO_PATH}")
table = scenario.get_editor_property("narration_table")
check(table is not None and table.get_path_name().startswith(MAIN_NARRATION_TABLE_PATH),
      "Main education scenario does not reference DT_Narration_Main")

flow = scenario.get_editor_property("editor_flow")
check(str(scenario.get_editor_property("start_stage_id")) == "MAIN_GATE",
      "Main gate presentation flow has an invalid start stage")
check([stage.get_editor_property("stage_id") for stage in flow] == [
    "MAIN_GATE", "GEO_NOKRO", "GONGSIMDON", "SINGIJEON", "ONGSEONG", "SUMMARY"
], "Unexpected Main Editor Flow stage order")

routes = scenario.get_editor_property("experience_routes")
route_ids = [route.get_editor_property("route_id") for route in routes]
check(route_ids == [],
      f"Unexpected Main experience routes: {route_ids}")

all_steps = []
for stage in flow:
    for step in stage.get_editor_property("steps"):
        all_steps.append(step)

narration_steps = [step for step in all_steps
                   if step.get_editor_property("step_type") == unreal.MainEducationAuthoringStepType.NARRATION]
check(len(narration_steps) == 1, f"Expected one opening narration, found {len(narration_steps)}")
check(str(narration_steps[0].get_editor_property("step_id")) == "GATE_GREETING",
      "Opening narration step is not GATE_GREETING")
check(str(narration_steps[0].get_editor_property("narration_start_row")) == "MAIN_NA_01",
      "Opening narration must start at MAIN_NA_01")
check("MAIN_NA_01" in [str(name) for name in unreal.DataTableFunctionLibrary.get_data_table_row_names(table)],
      "DT_Narration_Main does not contain MAIN_NA_01")

image_step_ids = {
    "GEO_NOKRO_IMAGE", "GONGSIMDON_IMAGE", "SINGIJEON_IMAGE",
    "ONGSEONG_IMAGE", "SUMMARY_01"
}
for step in all_steps:
    if str(step.get_editor_property("step_id")) in image_step_ids:
        content = step.get_editor_property("content")
        check(content.get_editor_property("image") is not None,
              f"Presentation image is missing: {step.get_editor_property('step_id')}")

unreal.EditorLoadingAndSavingUtils.load_map(MAIN_LEVEL_PATH)
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = actor_subsystem.get_all_level_actors()
managers = [actor for actor in actors if isinstance(actor, unreal.MainEducationScenarioManagerActor)]
check(len(managers) == 1, f"Expected one Main education manager, found {len(managers)}")
check(managers[0].get_editor_property("wait_for_intro_sequence"),
      "Main education manager is not waiting for the intro")
check(managers[0].get_editor_property("presentation_widget_component") is not None,
      "Main education manager has no runtime presentation panel")
for actor in actors:
    if not isinstance(actor, unreal.ExperienceTravelTriggerActor):
        continue
    destination = actor.get_editor_property("destination_experience")
    searchable = f"{actor.get_actor_label()} {destination.get_path_name() if destination else ''}".lower()
    check(not any(token in searchable for token in REMOVED_TOKENS),
          f"Removed experience trigger remains: {actor.get_actor_label()}")

unreal.log("VERIFY_MAIN_SINGIJEON_ONGSEONG_ONLY SUCCESS")
