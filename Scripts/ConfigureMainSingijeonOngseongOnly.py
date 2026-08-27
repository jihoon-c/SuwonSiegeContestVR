import unreal


MAIN_LEVEL_PATH = "/Game/Maps/Main/L_Main"
MAIN_SCENARIO_PATH = "/Game/Data/DA_Scenario_MainEducation"
MAIN_NARRATION_TABLE_PATH = "/Game/Audio/Narration/DT_Narration_Main"
REMOVED_TOKENS = ("gongsimdon", "nokro", "geojunggi", "singijeon", "ongseong")


scenario = unreal.load_asset(MAIN_SCENARIO_PATH)
if not scenario:
    raise RuntimeError(f"Missing Main scenario: {MAIN_SCENARIO_PATH}")

# This writes the designer-facing Editor Flow, its generated runtime graph, and
# the two remaining experience routes in one operation.
scenario.reset_to_main_gate_presentation_flow()
narration_table = unreal.load_asset(MAIN_NARRATION_TABLE_PATH)
if narration_table is None:
    raise RuntimeError(f"Missing Main narration table: {MAIN_NARRATION_TABLE_PATH}")
scenario.set_editor_property("narration_table", narration_table)
unreal.EditorAssetLibrary.save_loaded_asset(scenario, only_if_is_dirty=False)

unreal.EditorLoadingAndSavingUtils.load_map(MAIN_LEVEL_PATH)
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
removed_labels = []
for actor in actor_subsystem.get_all_level_actors():
    if not isinstance(actor, unreal.ExperienceTravelTriggerActor):
        continue
    label = actor.get_actor_label()
    destination = actor.get_editor_property("destination_experience")
    destination_path = destination.get_path_name() if destination else ""
    searchable = f"{label} {destination_path}".lower()
    if any(token in searchable for token in REMOVED_TOKENS):
        removed_labels.append(label)
        actor_subsystem.destroy_actor(actor)

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log(
    "MAIN_SINGIJEON_ONGSEONG_ONLY SUCCESS "
    f"removed_triggers={','.join(removed_labels) if removed_labels else 'none'}"
)
