import sys
from pathlib import Path

import unreal


sys.path.insert(0, str(Path(__file__).resolve().parent))
from GongsimdonNarratedScenarioData import (  # noqa: E402
    create_or_update_narration_table,
    make_gongsimdon_stage,
)

LEVEL_PATH = '/GF_Gongsimdon/Maps/LV_Gongsimdon'
SCENARIO_PATH = '/GF_Gongsimdon/Data/DA_Scenario_Gongsimdon'
EXPERIENCE_PATH = '/Game/Core/Experience/Definitions/DA_Experience_Gongsimdon'

narration_table = create_or_update_narration_table(unreal)
scenario = unreal.load_asset(SCENARIO_PATH)
experience = unreal.load_asset(EXPERIENCE_PATH)
if not scenario or not experience:
    raise RuntimeError('Gongsimdon Scenario or Experience is missing')

scenario.set_editor_property('scenario_id', 'SCENARIO_Gongsimdon')
scenario.set_editor_property('scenario_name', '공심돈 야간 경계')
scenario.set_editor_property('stages', [make_gongsimdon_stage(unreal)])
scenario.set_editor_property('start_stage_id', 'GONGSIMDON_STAGE_01')
scenario.set_editor_property('narration_table', narration_table)
unreal.EditorAssetLibrary.save_loaded_asset(scenario, only_if_is_dirty=False)

experience.set_editor_property('scenario_definition', scenario)
experience.set_editor_property('auto_start_scenario', True)
experience.set_editor_property('complete_on_scenario_finished', True)
unreal.EditorAssetLibrary.save_loaded_asset(experience, only_if_is_dirty=False)

world = unreal.EditorLoadingAndSavingUtils.load_map(LEVEL_PATH)
if not world:
    raise RuntimeError('LV_Gongsimdon could not be loaded')

actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = list(actor_subsystem.get_all_level_actors())


def find_by_label(label):
    return next((actor for actor in actors if actor.get_actor_label() == label), None)


def find_or_spawn(label, actor_class, location):
    actor = find_by_label(label)
    if actor and not isinstance(actor, actor_class):
        raise RuntimeError(f'{label} exists with the wrong class')
    if not actor:
        actor = actor_subsystem.spawn_actor_from_class(
            actor_class, location, unreal.Rotator())
        actor.set_actor_label(label)
        actors.append(actor)
    actor.set_actor_location(location, False, False)
    return actor


managers = [actor for actor in actors if isinstance(actor, unreal.ScenarioManagerActor)]
if len(managers) != 1:
    raise RuntimeError(f'LV_Gongsimdon must contain one Scenario Manager, found {len(managers)}')
manager = managers[0]
manager.set_editor_property('experience_definition', experience)
manager.set_editor_property('standalone_scenario_definition', None)
manager.set_editor_property('level_narration_table', None)
manager.set_editor_property('activate_experience_when_opened_directly', True)
manager.set_editor_property('restore_scenario_checkpoint', True)
manager.refresh_resolved_configuration()

player_start = next((actor for actor in actors if isinstance(actor, unreal.PlayerStart)), None)
origin = player_start.get_actor_location() if player_start else unreal.Vector(0.0, 0.0, 120.0)


def offset(x, y, z):
    return unreal.Vector(origin.x + x, origin.y + y, origin.z + z)


find_or_spawn('Gongsimdon_ScenarioDirector', unreal.GongsimdonScenarioDirectorActor, origin)

observation_specs = [
    ('Gongsimdon_OBS_Perimeter', 'OBS_PERIMETER', (900.0, 0.0, 80.0), 2.0, 24.0),
    ('Gongsimdon_OBS_Animal', 'OBS_ANIMAL', (1300.0, 550.0, 20.0), 1.5, 18.0),
    ('Gongsimdon_OBS_Metal', 'OBS_METAL', (1250.0, -650.0, 30.0), 1.5, 18.0),
    ('Gongsimdon_OBS_Beacon', 'OBS_BEACON', (-1600.0, 650.0, 500.0), 2.0, 18.0),
    ('Gongsimdon_OBS_FinalArea', 'OBS_FINAL_AREA', (1100.0, 0.0, 100.0), 2.0, 28.0),
]
for label, target_id, position, view_time, view_angle in observation_specs:
    actor = find_or_spawn(label, unreal.GongsimdonObservationTargetActor, offset(*position))
    actor.set_editor_property('target_id', target_id)
    actor.set_editor_property('required_view_time', view_time)
    actor.set_editor_property('required_view_angle', view_angle)
    actor.set_editor_property('max_distance', 5000.0)
    actor.set_editor_property('require_line_of_sight', False)

report_actor = find_or_spawn(
    'Gongsimdon_Report', unreal.GongsimdonReportActor, offset(100.0, 0.0, 0.0))
report_actor.set_editor_property('target_id', 'REPORT_ENEMY')
report_actor.set_editor_property('expected_direction', unreal.GongsimdonReportDirection.EAST)
report_actor.set_editor_property('minimum_enemy_count', 6)
report_actor.set_editor_property('maximum_enemy_count', 8)

defense_weapon = find_or_spawn(
    'Gongsimdon_DefenseWeapon', unreal.GongsimdonDefenseWeaponActor,
    offset(80.0, -60.0, 0.0))
defense_weapon.set_editor_property('shot_range', 5000.0)
defense_weapon.set_editor_property('aim_assist_radius', 80.0)
defense_weapon.set_editor_property('shot_damage', 10.0)

for obsolete_label in ('Gongsimdon_OBS_EnemyGroup', 'Gongsimdon_Combat_Retreating'):
    obsolete_actor = find_by_label(obsolete_label)
    if obsolete_actor:
        actor_subsystem.destroy_actor(obsolete_actor)
        actors.remove(obsolete_actor)

enemy_group = find_or_spawn(
    'Gongsimdon_EnemyGroup', unreal.GongsimdonEnemyGroupActor,
    offset(400.0, -850.0, -90.0))
enemy_group.set_editor_property('enemy_count', 7)
enemy_group.set_editor_property('formation_columns', 3)
enemy_group.set_editor_property('lateral_spacing', 140.0)
enemy_group.set_editor_property('row_spacing', 180.0)
enemy_group.set_editor_property('approach_speed', 140.0)
enemy_group.set_editor_property('retreat_speed', 260.0)
enemy_group.set_editor_property('observation_target_id', 'OBS_ENEMY_GROUP')
enemy_group.set_editor_property('combat_target_id', 'COMBAT_RETREATING')
enemy_group.set_editor_property('required_combat_hits', 1)

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log('GONGSIMDON_NARRATED_ACTION_INTERACTIONS CONFIGURE SUCCESS')
