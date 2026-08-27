import unreal


errors = []


def check(condition, message):
    if condition:
        unreal.log(f"EXPERIENCE_VERIFY PASS: {message}")
    else:
        errors.append(message)
        unreal.log_error(f"EXPERIENCE_VERIFY FAIL: {message}")


definition = unreal.load_asset(
    "/Game/Core/Experience/Definitions/DA_Experience_Singijeon"
)
check(definition is not None, "DA_Experience_Singijeon loads")
if definition:
    check(
        str(definition.get_editor_property("experience_id")) == "EXP_Singijeon",
        "ExperienceID is EXP_Singijeon",
    )
    experience_level = definition.get_editor_property("experience_level")
    check(
        "LV_Singijeon" in str(experience_level),
        "ExperienceLevel references LV_Singijeon",
    )
    check(
        "L_Main" in str(definition.get_editor_property("return_level")),
        "ReturnLevel references L_Main",
    )
    check(
        definition.get_editor_property("return_on_completion"),
        "Automatic return on completion is enabled",
    )

unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/LV_Singijeon")
managers = [
    actor
    for actor in unreal.EditorLevelLibrary.get_all_level_actors()
    if isinstance(actor, unreal.ScenarioManagerActor)
]
check(len(managers) == 1, "LV_Singijeon contains exactly one Scenario Manager")
if managers and definition:
    manager = managers[0]
    check(
        manager.get_editor_property("experience_definition") == definition,
        "Placed Manager references DA_Experience_Singijeon",
    )
    check(
        manager.get_editor_property("activate_experience_when_opened_directly"),
        "Direct PIE activation is enabled",
    )
    check(
        manager.get_editor_property("complete_experience_on_scenario_finished"),
        "Scenario completion bridge is enabled",
    )

if errors:
    raise RuntimeError("Experience verification failed: " + "; ".join(errors))

unreal.log("EXPERIENCE_VERIFY SUCCESS")
