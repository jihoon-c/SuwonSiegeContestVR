import unreal


errors = []


def check(condition, message):
    if condition:
        unreal.log(f"MAIN_ROUND_TRIP_VERIFY PASS: {message}")
    else:
        errors.append(message)
        unreal.log_error(f"MAIN_ROUND_TRIP_VERIFY FAIL: {message}")


main_scenario = unreal.load_asset("/Game/Data/DA_Scenario_Main")
main_experience = unreal.load_asset(
    "/Game/Core/Experience/Definitions/DA_Experience_Main"
)
singijeon_experience = unreal.load_asset(
    "/Game/Core/Experience/Definitions/DA_Experience_Singijeon"
)
check(main_scenario is not None, "DA_Scenario_Main loads")
check(main_experience is not None, "DA_Experience_Main loads")
check(singijeon_experience is not None, "DA_Experience_Singijeon loads")

if main_scenario:
    stages = list(main_scenario.get_editor_property("stages"))
    ids = [str(item.get_editor_property("interaction_id"))
           for item in stages[0].get_editor_property("interactions")] if stages else []
    check(ids == [
        "MAIN_INTRO", "MAIN_TRAVEL_GONGSIMDON", "MAIN_AFTER_GONGSIMDON",
        "MAIN_TRAVEL_SINGIJEON", "MAIN_AFTER_SINGIJEON",
    ], "DA_Scenario_Main reaches Singijeon after the Gongsimdon return")
    check(str(main_scenario.get_editor_property("scenario_id")) == "SCENARIO_Main",
          "Main Scenario ID is configured")
if main_experience:
    check("L_Main" in str(main_experience.get_editor_property("experience_level")),
          "Main Experience references L_Main")
if singijeon_experience:
    check("L_Main" in str(singijeon_experience.get_editor_property("return_level")),
          "Singijeon Experience returns to L_Main")
    check(singijeon_experience.get_editor_property("return_on_completion"),
          "Singijeon automatic return is enabled")

world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/Main/L_Main")
check(world is not None, "L_Main loads")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
managers = [actor for actor in actors if isinstance(actor, unreal.ScenarioManagerActor)]
triggers = [actor for actor in actors if isinstance(actor, unreal.ExperienceTravelTriggerActor)]
player_starts = [actor for actor in actors if isinstance(actor, unreal.PlayerStart)]
check(len(managers) == 1, "L_Main contains exactly one Scenario Manager")
check(len(triggers) == 2, "L_Main contains the Gongsimdon and Singijeon Travel Triggers")
check(len(player_starts) >= 1, "L_Main contains a PlayerStart")
if managers:
    manager = managers[0]
    check(manager.get_editor_property("scenario_definition") == main_scenario,
          "Main Manager resolves DA_Scenario_Main from Experience")
    check(manager.get_editor_property("experience_definition") == main_experience,
          "Main Manager references DA_Experience_Main")
    check(manager.get_editor_property("auto_start_scenario"),
          "Main Scenario auto-start is enabled")
    check(not manager.get_editor_property("complete_experience_on_scenario_finished"),
          "Main Scenario does not auto-complete its Experience")
    check(manager.get_editor_property("restore_scenario_checkpoint"),
          "Main Scenario checkpoint restore is enabled")
singijeon_triggers = [
    trigger for trigger in triggers
    if trigger.get_actor_label() == "ExperienceTravelTrigger_Singijeon"
]
check(len(singijeon_triggers) == 1, "L_Main contains the named Singijeon Travel Trigger")
if singijeon_triggers:
    trigger = singijeon_triggers[0]
    check(trigger.get_editor_property("destination_experience") == singijeon_experience,
          "Singijeon Travel Trigger targets the Singijeon Experience")
    check(str(trigger.get_editor_property("required_interaction_id")) ==
          "MAIN_TRAVEL_SINGIJEON",
          "Singijeon Travel Trigger is gated after the Gongsimdon return")
    check(str(trigger.get_editor_property("return_interaction_id")) ==
          "MAIN_AFTER_SINGIJEON",
          "Travel Trigger stores the post-Singijeon checkpoint")
    check(trigger.get_editor_property("trigger_on_player_view_location"),
          "Travel Trigger supports collisionless VR Pawn HMD entry")

if errors:
    raise RuntimeError("Main round-trip verification failed: " + "; ".join(errors))
unreal.log("MAIN_SINGIJEON_ROUND_TRIP VERIFY SUCCESS")
