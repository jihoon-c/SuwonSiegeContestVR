"""Verifies the Main education flow: greet, then explain -> spoken quiz -> travel per experience.

Run with Unreal Editor Python after ConfigureMainSingijeonOngseongOnly.py.
"""

import unreal


MAIN_LEVEL_PATH = "/Game/Maps/Main/L_Main"
ONGSEONG_LEVEL_PATH = "/GF_OngseongCrossbow/Maps/LV_Ongseong"
MAIN_SCENARIO_PATH = "/Game/Data/DA_Scenario_MainEducation"
MAIN_NARRATION_TABLE_PATH = "/Game/Audio/Narration/DT_Narration_Main"
REMOVED_TOKENS = ("gongsimdon", "nokro", "geojunggi", "singijeon", "ongseong")

EXPECTED_STAGES = ["MAIN_GATE", "SINGIJEON", "ONGSEONG", "SUMMARY"]

# stage -> (narration start row, quiz step, quiz content, initial consonants, answer, route, return step)
EXPECTED_EXPERIENCES = {
    "SINGIJEON": ("MAIN_NA_13", "SINGIJEON_QUIZ", "QUIZ_HWACHA", "ㅎ ㅊ", "화차",
                  "Travel_Singijeon", "AFTER_SINGIJEON"),
    "ONGSEONG": ("MAIN_NA_18", "ONGSEONG_QUIZ", "QUIZ_ONGSEONG", "ㅇ ㅅ", "옹성",
                 "Travel_Ongseong", "AFTER_ONGSEONG"),
}


def check(condition, message):
    if not condition:
        raise RuntimeError(message)


scenario = unreal.load_asset(MAIN_SCENARIO_PATH)
check(scenario is not None, f"Missing Main scenario: {MAIN_SCENARIO_PATH}")
table = scenario.get_editor_property("narration_table")
check(table is not None and table.get_path_name().startswith(MAIN_NARRATION_TABLE_PATH),
      "Main education scenario does not reference DT_Narration_Main")
narration_rows = {str(name) for name in unreal.DataTableFunctionLibrary.get_data_table_row_names(table)}

flow = scenario.get_editor_property("editor_flow")
check(str(scenario.get_editor_property("start_stage_id")) == "MAIN_GATE",
      "Main flow has an invalid start stage")
check([str(stage.get_editor_property("stage_id")) for stage in flow] == EXPECTED_STAGES,
      "Unexpected Main Editor Flow stage order")

routes = {str(route.get_editor_property("route_id")): route
          for route in scenario.get_editor_property("experience_routes")}
check(sorted(routes) == ["Travel_Ongseong", "Travel_Singijeon"],
      f"Unexpected Main experience routes: {sorted(routes)}")
for route_id, route in routes.items():
    experience = route.get_editor_property("experience")
    check(experience is not None, f"Route {route_id} has no Experience asset")
    loaded = unreal.load_asset(experience.get_path_name().split(".")[0])
    check(loaded is not None, f"Route {route_id} points at a missing Experience asset")
    return_level = str(loaded.get_editor_property("return_level"))
    check(MAIN_LEVEL_PATH in return_level,
          f"{route_id} does not return to L_Main (ReturnLevel={return_level})")

stages = {str(stage.get_editor_property("stage_id")): stage.get_editor_property("steps")
          for stage in flow}

# The opening greeting is the only narration outside an experience stage.
gate_steps = stages["MAIN_GATE"]
check(len(gate_steps) == 1 and str(gate_steps[0].get_editor_property("step_id")) == "GATE_GREETING",
      "The gate stage must be the single greeting narration")
check(str(gate_steps[0].get_editor_property("narration_start_row")) == "MAIN_NA_01",
      "The greeting must start at MAIN_NA_01")

for stage_id, expected in EXPECTED_EXPERIENCES.items():
    (narration_row, quiz_step_id, quiz_content_id,
     consonants, answer, route_id, return_step_id) = expected
    steps = stages[stage_id]
    step_ids = [str(step.get_editor_property("step_id")) for step in steps]

    narration_index = next(
        index for index, step in enumerate(steps)
        if step.get_editor_property("step_type") == unreal.MainEducationAuthoringStepType.NARRATION)
    check(str(steps[narration_index].get_editor_property("narration_start_row")) == narration_row,
          f"{stage_id} explanation must start at {narration_row}")
    check(narration_row in narration_rows, f"DT_Narration_Main has no row {narration_row}")

    # The quiz is asked only after the explanation, and travel only after the quiz.
    quiz_index = narration_index + 1
    check(step_ids[quiz_index] == quiz_step_id,
          f"{stage_id} must ask {quiz_step_id} right after its narration")
    quiz_step = steps[quiz_index]
    check(quiz_step.get_editor_property("step_type") == unreal.MainEducationAuthoringStepType.QUIZ,
          f"{quiz_step_id} is not authored as a Quiz step")
    quiz_content = quiz_step.get_editor_property("content")
    check(str(quiz_content.get_editor_property("content_id")) == quiz_content_id,
          f"{quiz_step_id} does not point at {quiz_content_id}")
    check(str(quiz_content.get_editor_property("initial_consonants")) == consonants,
          f"{quiz_content_id} does not show {consonants}")
    accepted = [str(text) for text in quiz_content.get_editor_property("accepted_answers")]
    check(accepted and accepted[0] == answer,
          f"{quiz_content_id} does not accept {answer}: {accepted}")

    travel_index = quiz_index + 1
    travel_step = steps[travel_index]
    check(travel_step.get_editor_property("step_type") == unreal.MainEducationAuthoringStepType.TRAVEL,
          f"{stage_id} must travel right after the quiz")
    check(str(travel_step.get_editor_property("experience_route_id")) == route_id,
          f"{stage_id} travel does not use {route_id}")
    # BeginExperienceTravel stores the next step as the return checkpoint, so travel is never last.
    check(travel_index + 1 < len(steps) and step_ids[travel_index + 1] == return_step_id,
          f"{stage_id} has no {return_step_id} step for the return checkpoint")

    image_step = steps[narration_index - 1] if narration_index > 0 else None
    check(image_step is not None and
          image_step.get_editor_property("content").get_editor_property("image") is not None,
          f"{stage_id} has no presentation image before its narration")

unreal.EditorLoadingAndSavingUtils.load_map(MAIN_LEVEL_PATH)
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = actor_subsystem.get_all_level_actors()
managers = [actor for actor in actors if isinstance(actor, unreal.MainEducationScenarioManagerActor)]
check(len(managers) == 1, f"Expected one Main education manager, found {len(managers)}")
check(managers[0].get_editor_property("wait_for_intro_sequence"),
      "Main education manager is not waiting for the intro")
check(managers[0].get_editor_property("presentation_widget_component") is not None,
      "Main education manager has no runtime presentation panel")
check(managers[0].get_editor_property("use_voice_quiz"),
      "Main education manager has the spoken quiz turned off")
check(managers[0].get_editor_property("education_quiz") is not None,
      "Main education manager has no Core quiz runtime")
# Travel is driven by the Scenario steps now, so no placed trigger should also fire it.
for actor in actors:
    if not isinstance(actor, unreal.ExperienceTravelTriggerActor):
        continue
    destination = actor.get_editor_property("destination_experience")
    searchable = f"{actor.get_actor_label()} {destination.get_path_name() if destination else ''}".lower()
    check(not any(token in searchable for token in REMOVED_TOKENS),
          f"Removed experience trigger remains: {actor.get_actor_label()}")

# Main asks the 옹성 quiz, so the copy inside the Ongseong level stays off.
unreal.EditorLoadingAndSavingUtils.load_map(ONGSEONG_LEVEL_PATH)
ongseong_managers = [actor for actor in actor_subsystem.get_all_level_actors()
                     if isinstance(actor, unreal.OngseongDefenseScenarioManager)]
check(len(ongseong_managers) == 1,
      f"Expected one Ongseong defense manager, found {len(ongseong_managers)}")
check(not ongseong_managers[0].get_editor_property("run_intro_quiz"),
      "LV_Ongseong still asks its own 옹성 quiz; Main already asks it")

unreal.log("VERIFY_MAIN_SINGIJEON_ONGSEONG_ONLY SUCCESS")
