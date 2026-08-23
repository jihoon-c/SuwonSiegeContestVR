import sys
from pathlib import Path

import unreal

sys.path.insert(0, str(Path(__file__).resolve().parent))
from GongsimdonNarratedScenarioData import (  # noqa: E402
    NARRATION_TABLE_PATH,
    ORDERED_INTERACTION_IDS,
)


errors = []


def check(condition, message):
    if condition:
        unreal.log(f"MAIN_MULTI_TRAVEL_VERIFY PASS: {message}")
    else:
        errors.append(message)
        unreal.log_error(f"MAIN_MULTI_TRAVEL_VERIFY FAIL: {message}")


main_scenario = unreal.load_asset("/Game/Data/DA_Scenario_Main")
gongsimdon_scenario = unreal.load_asset("/GF_Gongsimdon/Data/DA_Scenario_Gongsimdon")
main_experience = unreal.load_asset("/Game/Core/Experience/Definitions/DA_Experience_Main")
gongsimdon_experience = unreal.load_asset(
    "/Game/Core/Experience/Definitions/DA_Experience_Gongsimdon"
)
singijeon_experience = unreal.load_asset(
    "/Game/Core/Experience/Definitions/DA_Experience_Singijeon"
)
for asset, name in (
    (main_scenario, "DA_Scenario_Main"),
    (gongsimdon_scenario, "DA_Scenario_Gongsimdon"),
    (main_experience, "DA_Experience_Main"),
    (gongsimdon_experience, "DA_Experience_Gongsimdon"),
    (singijeon_experience, "DA_Experience_Singijeon"),
):
    check(asset is not None, f"{name} loads")

if main_scenario:
    stages = list(main_scenario.get_editor_property("stages"))
    ids = [
        str(item.get_editor_property("interaction_id"))
        for item in (stages[0].get_editor_property("interactions") if stages else [])
    ]
    check(ids == [
        "MAIN_INTRO", "MAIN_TRAVEL_GONGSIMDON", "MAIN_AFTER_GONGSIMDON",
        "MAIN_TRAVEL_SINGIJEON", "MAIN_AFTER_SINGIJEON",
    ], "Main flow is Gongsimdon return then Singijeon return")

if gongsimdon_scenario:
    stages = list(gongsimdon_scenario.get_editor_property("stages"))
    interactions = list(stages[0].get_editor_property("interactions")) if stages else []
    ids = [str(item.get_editor_property("interaction_id")) for item in interactions]
    check(ids == ORDERED_INTERACTION_IDS,
          "Gongsimdon narrated 27 + action 13 flow is configured")
    check(gongsimdon_scenario.get_editor_property("narration_table") ==
          unreal.load_asset(NARRATION_TABLE_PATH),
          "Gongsimdon Scenario owns its narration table")
    check(str(gongsimdon_scenario.get_editor_property("scenario_id")) ==
          "SCENARIO_Gongsimdon", "Gongsimdon Scenario ID is configured")

if gongsimdon_experience:
    check(gongsimdon_experience.get_editor_property("scenario_definition") ==
          gongsimdon_scenario, "Gongsimdon Experience owns its Scenario")
    check("LV_Gongsimdon" in str(gongsimdon_experience.get_editor_property("experience_level")),
          "Gongsimdon Experience references its plugin Level")
    check("L_Main" in str(gongsimdon_experience.get_editor_property("return_level")),
          "Gongsimdon Experience returns to Main")
    check(gongsimdon_experience.get_editor_property("return_on_completion"),
          "Gongsimdon automatic return is enabled")
if singijeon_experience:
    check("L_Main" in str(singijeon_experience.get_editor_property("return_level")),
          "Singijeon Experience still returns to Main")

unreal.EditorLoadingAndSavingUtils.load_map("/GF_Gongsimdon/Maps/LV_Gongsimdon")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
managers = [actor for actor in actors if isinstance(actor, unreal.ScenarioManagerActor)]
check(len(managers) == 1, "Gongsimdon Level contains one Scenario Manager")
check(any(isinstance(actor, unreal.PlayerStart) for actor in actors),
      "Gongsimdon Level contains a PlayerStart")
if managers:
    check(managers[0].get_editor_property("experience_definition") == gongsimdon_experience,
          "Gongsimdon Manager references its Experience")
    check(managers[0].get_editor_property("scenario_definition") == gongsimdon_scenario,
          "Gongsimdon Manager resolves its Scenario")
    check(managers[0].get_editor_property("narration_table") ==
          unreal.load_asset(NARRATION_TABLE_PATH),
          "Gongsimdon Manager resolves the Scenario-owned narration table")

unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/Main/L_Main")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
triggers = {
    actor.get_actor_label(): actor
    for actor in actors
    if isinstance(actor, unreal.ExperienceTravelTriggerActor)
}
check("ExperienceTravelTrigger_Gongsimdon" in triggers,
      "Main contains the Gongsimdon Travel Trigger")
check("ExperienceTravelTrigger_Singijeon" in triggers,
      "Main contains the Singijeon Travel Trigger")
if "ExperienceTravelTrigger_Gongsimdon" in triggers:
    trigger = triggers["ExperienceTravelTrigger_Gongsimdon"]
    check(trigger.get_editor_property("destination_experience") == gongsimdon_experience,
          "Gongsimdon Trigger targets its Experience")
    check(str(trigger.get_editor_property("required_interaction_id")) ==
          "MAIN_TRAVEL_GONGSIMDON", "Gongsimdon Trigger is order-gated")
    check(str(trigger.get_editor_property("return_interaction_id")) ==
          "MAIN_AFTER_GONGSIMDON", "Gongsimdon return resumes after its step")
if "ExperienceTravelTrigger_Singijeon" in triggers:
    trigger = triggers["ExperienceTravelTrigger_Singijeon"]
    check(trigger.get_editor_property("destination_experience") == singijeon_experience,
          "Singijeon Trigger targets its Experience")
    check(str(trigger.get_editor_property("required_interaction_id")) ==
          "MAIN_TRAVEL_SINGIJEON", "Singijeon Trigger unlocks after Gongsimdon")
    check(str(trigger.get_editor_property("return_interaction_id")) ==
          "MAIN_AFTER_SINGIJEON", "Singijeon return resumes after its step")

if errors:
    raise RuntimeError("Main multi-travel verification failed: " + "; ".join(errors))
unreal.log("MAIN_GONGSIMDON_SINGIJEON_SEQUENCE VERIFY SUCCESS")
