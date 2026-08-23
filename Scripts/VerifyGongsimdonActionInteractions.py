import json
import sys
from pathlib import Path

import unreal


sys.path.insert(0, str(Path(__file__).resolve().parent))
from GongsimdonNarratedScenarioData import (  # noqa: E402
    ACTION_DEFINITIONS,
    ACTION_GUIDES,
    NARRATION_TABLE_PATH,
    ORDERED_INTERACTION_IDS,
    SUBTITLES,
)

errors = []


def check(condition, message):
    if condition:
        unreal.log(f'GONGSIMDON_NARRATED_VERIFY PASS: {message}')
    else:
        errors.append(message)
        unreal.log_error(f'GONGSIMDON_NARRATED_VERIFY FAIL: {message}')


scenario = unreal.load_asset('/GF_Gongsimdon/Data/DA_Scenario_Gongsimdon')
table = unreal.load_asset(NARRATION_TABLE_PATH)
experience = unreal.load_asset('/Game/Core/Experience/Definitions/DA_Experience_Gongsimdon')
check(scenario is not None, 'DA_Scenario_Gongsimdon loads')
check(table is not None, 'DT_Narration_Gongsimdon loads')
check(experience is not None, 'DA_Experience_Gongsimdon loads')

if table:
    export_result = unreal.DataTableFunctionLibrary.export_data_table_to_json_string(table)
    export_success, payload = export_result if isinstance(export_result, tuple) else (True, export_result)
    rows = json.loads(payload) if export_success and payload else []
    check(len(rows) == 27, 'Narration table contains 27 rows')
    row_by_name = {row['Name']: row for row in rows}
    for index in range(1, 28):
        row_name = f'GONG_NA_{index:02d}'
        row = row_by_name.get(row_name)
        check(row is not None, f'{row_name} exists')
        if row:
            check(row['NextRow'] == 'None' and row['AdvanceMode'] == 'Stop',
                  f'{row_name} returns flow control to Scenario')
            check(bool(row['NarrationSound']) and '/GF_Gongsimdon/Asset/Narration/' in row['NarrationSound'],
                  f'{row_name} references its plugin audio')
            check(SUBTITLES[index] in row['Subtitle'], f'{row_name} contains its subtitle')

if scenario:
    stages = list(scenario.get_editor_property('stages'))
    check(len(stages) == 1, 'Scenario contains one inline Stage')
    stage = stages[0] if stages else None
    interactions = list(stage.get_editor_property('interactions')) if stage else []
    ids = [str(item.get_editor_property('interaction_id')) for item in interactions]
    check(ids == ORDERED_INTERACTION_IDS, 'Narration and Action interactions follow the authored order')
    check(len(interactions) == 40, 'Scenario contains 27 Narration and 13 Action interactions')
    check(stage is not None and str(stage.get_editor_property('start_interaction_id')) == 'GONG_NAR_01',
          'Stage starts with the first narration')
    check(scenario.get_editor_property('narration_table') == table,
          'Scenario owns the Gongsimdon narration table')
    for position, interaction in enumerate(interactions):
        expected_next = (ORDERED_INTERACTION_IDS[position + 1]
                         if position + 1 < len(ORDERED_INTERACTION_IDS) else 'None')
        interaction_id = ids[position]
        check(str(interaction.get_editor_property('next_interaction_id')) == expected_next,
              f'{interaction_id} advances to {expected_next}')
        if interaction_id.startswith('GONG_NAR_'):
            check(interaction.get_editor_property('interaction_type') ==
                  unreal.ScenarioInteractionType.NARRATION,
                  f'{interaction_id} is Narration')
        else:
            check(interaction_id in ACTION_DEFINITIONS, f'{interaction_id} is a known Action')
            if interaction_id in ACTION_GUIDES:
                check(interaction.get_editor_property('guide_action') !=
                      unreal.ScenarioGuideAction.AUTO,
                      f'{interaction_id} has an explicit contextual guide')
                check(bool(str(interaction.get_editor_property('guide_text'))),
                      f'{interaction_id} has guide text')

if experience and scenario:
    check(experience.get_editor_property('scenario_definition') == scenario,
          'Gongsimdon Experience references the narrated Scenario')
    check(experience.get_editor_property('auto_start_scenario'),
          'Gongsimdon Scenario auto-starts')
    check(experience.get_editor_property('complete_on_scenario_finished'),
          'Scenario completion completes the Gongsimdon Experience')

unreal.EditorLoadingAndSavingUtils.load_map('/GF_Gongsimdon/Maps/LV_Gongsimdon')
actors = list(unreal.get_editor_subsystem(
    unreal.EditorActorSubsystem).get_all_level_actors())
managers = [actor for actor in actors if isinstance(actor, unreal.ScenarioManagerActor)]
directors = [actor for actor in actors if isinstance(actor, unreal.GongsimdonScenarioDirectorActor)]
observations = [actor for actor in actors
                if isinstance(actor, unreal.GongsimdonObservationTargetActor)]
reports = [actor for actor in actors if isinstance(actor, unreal.GongsimdonReportActor)]
enemy_groups = [actor for actor in actors if isinstance(actor, unreal.GongsimdonEnemyGroupActor)]
defense_weapons = [actor for actor in actors
                   if isinstance(actor, unreal.GongsimdonDefenseWeaponActor)]

check(len(managers) == 1, 'Level contains one Scenario Manager')
if managers:
    managers[0].refresh_resolved_configuration()
    check(managers[0].get_editor_property('experience_definition') == experience,
          'Level Manager references the Gongsimdon Experience')
    check(managers[0].get_editor_property('scenario_definition') == scenario,
          'Level Manager resolves DA_Scenario_Gongsimdon')
    check(managers[0].get_editor_property('narration_table') == table,
          'Level Manager resolves DT_Narration_Gongsimdon')
    check(managers[0].get_editor_property('level_narration_table') is None,
          'Level does not override the Scenario-owned narration table')

check(len(directors) == 1, 'Level contains one Gongsimdon Scenario Director')
check(len(observations) == 5, 'Level contains five static observation targets')
check(len(reports) == 1, 'Level contains one report validator')
check(len(enemy_groups) == 1, 'Level contains one enemy group')
check(len(defense_weapons) == 1, 'Level contains one playable defense weapon')
observation_ids = {str(actor.get_editor_property('target_id')) for actor in observations}
check(observation_ids == {
    'OBS_PERIMETER', 'OBS_ANIMAL', 'OBS_METAL', 'OBS_BEACON', 'OBS_FINAL_AREA'
}, 'Observation Target IDs match the Scenario')
if reports:
    check(str(reports[0].get_editor_property('target_id')) == 'REPORT_ENEMY',
          'Report Target ID matches the Scenario')
    report_button = reports[0].get_editor_property('report_button')
    check(report_button is not None and report_button.component_has_tag('VRGrab'),
          'Report actor exposes a visible VR trigger button')
if defense_weapons:
    weapon_mesh = defense_weapons[0].get_editor_property('weapon_mesh')
    combat_guide = defense_weapons[0].get_editor_property('combat_guide_interactor')
    check(weapon_mesh is not None and weapon_mesh.component_has_tag('VRGrab'),
          'Defense weapon is usable with either VR trigger')
    check(str(combat_guide.get_editor_property('target_id')) == 'COMBAT_RETREATING',
          'Defense weapon receives the Combat interaction guide')
if enemy_groups:
    check(str(enemy_groups[0].get_editor_property('observation_target_id')) == 'OBS_ENEMY_GROUP',
          'Enemy Group observation ID matches the Scenario')
    check(str(enemy_groups[0].get_editor_property('combat_target_id')) == 'COMBAT_RETREATING',
          'Enemy Group combat ID matches the Scenario')

if errors:
    raise RuntimeError('Gongsimdon narrated verification failed: ' + '; '.join(errors))
unreal.log('GONGSIMDON_NARRATED_SCENARIO VERIFY SUCCESS')
