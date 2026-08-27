import unreal


SCENARIO_PATH = "/Game/Data/DA_Scenario_MainEducation"
MAIN_EXPERIENCE_PATH = "/Game/Core/Experience/Definitions/DA_Experience_Main"
MAIN_LEVEL_PATH = "/Game/Maps/Main/L_Main"


scenario = unreal.load_asset(SCENARIO_PATH)
if not isinstance(scenario, unreal.MainEducationScenarioDefinition):
    raise RuntimeError("Main education Scenario Definition is missing or has the wrong class")

stages = scenario.get_editor_property("stages")
content = scenario.get_editor_property("education_content")
routes = scenario.get_editor_property("experience_routes")
if len(stages) != 6:
    raise RuntimeError(f"Expected 6 education stages, found {len(stages)}")
if len(routes) != 0:
    raise RuntimeError(f"Expected no experience routes, found {len(routes)}")
table = scenario.get_editor_property("narration_table")
if table is None or "DT_Narration_Main" not in table.get_path_name():
    raise RuntimeError("Main education scenario must reference DT_Narration_Main")
if [stage.get_editor_property("stage_id") for stage in stages] != [
    "MAIN_GATE", "GEO_NOKRO", "GONGSIMDON", "SINGIJEON", "ONGSEONG", "SUMMARY"
]:
    raise RuntimeError("Unexpected Main gate presentation stage order")

main_experience = unreal.load_asset(MAIN_EXPERIENCE_PATH)
if main_experience.get_editor_property("scenario_definition") != scenario:
    raise RuntimeError("DA_Experience_Main is not linked to DA_Scenario_MainEducation")

unreal.EditorLoadingAndSavingUtils.load_map(MAIN_LEVEL_PATH)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
managers = [
    actor for actor in actors
    if isinstance(actor, unreal.MainEducationScenarioManagerActor)
]
if len(managers) != 1:
    raise RuntimeError(f"Expected one MainEducationScenarioManagerActor, found {len(managers)}")
if managers[0].get_editor_property("experience_definition") != main_experience:
    raise RuntimeError("Placed Main education manager is not linked to DA_Experience_Main")

unreal.log(
    f"MAIN_EDUCATION_FLOW VERIFY SUCCESS: {len(stages)} stages, "
    f"{len(content)} presentation entries, opening narration enabled, {len(routes)} routes"
)
