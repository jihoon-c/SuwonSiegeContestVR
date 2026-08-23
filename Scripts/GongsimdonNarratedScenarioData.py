import json
import re


NARRATION_TABLE_PATH = '/GF_Gongsimdon/Data/DT_Narration_Gongsimdon'
NARRATION_DIRECTORY = '/GF_Gongsimdon/Asset/Narration'

SUBTITLES = {
    1: '현재 이곳은 수원화성의 공심돈입니다.',
    2: '공심돈은 성 밖의 상황을 감시하고, 적이 접근할 경우 공격할 수 있도록 만든 방어시설입니다.',
    3: '지금부터 공심돈에서 야간 경계를 시작하겠습니다.',
    4: '성 밖을 살피며 주변에 이상이 없는지 확인해 보세요.',
    5: '소리가 난 방향을 확인해 보세요.',
    6: '수풀 사이에 움직임이 보이지만 적의 모습은 확인되지 않습니다.',
    7: '다행히 적이 아니라 야생동물이었던 것 같습니다.',
    8: '경계를 계속 이어가세요.',
    9: '성 밖에서 여러 명의 적군이 접근하고 있습니다.',
    10: '적의 위치와 규모를 확인하세요.',
    11: '동쪽 성벽 방향에서 성벽으로 접근하고 있습니다.',
    12: '적을 발견했을 때는 적의 위치와 규모를 신속하게 보고하는 것이 중요합니다.',
    13: '확인한 내용을 보고하세요.',
    14: '이제 신호 체계가 정상적으로 작동하고 있는지 확인해야 합니다.',
    15: '수원화성의 봉돈은 연기와 불을 이용해 주변의 상황을 전달하던 통신 시설입니다.',
    16: '봉돈 방향을 확인해 보세요.',
    17: '봉돈에 신호가 올라왔습니다.',
    18: '적의 접근 상황이 전달되고 있습니다.',
    19: '이제 적의 움직임을 계속 감시하며 공격에 대비하세요.',
    20: '적군이 퇴각하고 있습니다.',
    21: '공심돈은 적을 감시하는 시설인 동시에, 성 밖의 적을 공격할 수 있도록 만들어진 방어시설입니다.',
    22: '달아나는 적을 조준하세요.',
    23: '적군이 사정거리 밖으로 퇴각했습니다.',
    24: '경계를 유지하며 주변에 남아 있는 적이 없는지 확인하세요.',
    25: '이번 체험에서는 공심돈에서 성 밖을 경계하고 이상 징후를 발견한 뒤 적의 위치와 규모를 보고했습니다.',
    26: '또한 봉돈의 신호를 확인하고 공심돈에서 적을 공격하는 과정을 통해 수원화성의 방어 체계를 살펴보았습니다.',
    27: '수원화성은 공심돈과 봉돈 등 여러 시설을 활용하여 적을 감시하고 상황을 전달하며 성을 방어했습니다.',
}

ACTION_DEFINITIONS = {
    'GONG_ACT_SCAN_PERIMETER': ('OBSERVE', 'OBS_PERIMETER', False),
    'GONG_ACT_SOUND_ANIMAL': ('SEQUENCE', 'CUE_ANIMAL_SOUND', True),
    'GONG_ACT_CHECK_ANIMAL': ('OBSERVE', 'OBS_ANIMAL', False),
    'GONG_ACT_SOUND_METAL': ('SEQUENCE', 'CUE_METAL_SOUND', True),
    'GONG_ACT_CHECK_METAL': ('OBSERVE', 'OBS_METAL', False),
    'GONG_ACT_REVEAL_ENEMY': ('SPAWN', 'CUE_REVEAL_ENEMY', True),
    'GONG_ACT_IDENTIFY_ENEMY': ('OBSERVE', 'OBS_ENEMY_GROUP', False),
    'GONG_ACT_REPORT_ENEMY': ('CUSTOM', 'REPORT_ENEMY', False),
    'GONG_ACT_ENABLE_BEACON': ('SEQUENCE', 'CUE_ENABLE_BEACON', True),
    'GONG_ACT_CHECK_BEACON': ('OBSERVE', 'OBS_BEACON', False),
    'GONG_ACT_RETREAT_ENEMY': ('SEQUENCE', 'CUE_RETREAT_ENEMY', True),
    'GONG_ACT_SHOOT_RETREATING': ('COMBAT', 'COMBAT_RETREATING', False),
    'GONG_ACT_FINAL_SCAN': ('OBSERVE', 'OBS_FINAL_AREA', False),
}

ACTION_GUIDES = {
    'GONG_ACT_SCAN_PERIMETER': ('OBSERVE', '성 밖 경계 구역을 바라보세요.'),
    'GONG_ACT_CHECK_ANIMAL': ('OBSERVE', '수상한 소리가 난 수풀을 바라보세요.'),
    'GONG_ACT_CHECK_METAL': ('OBSERVE', '쇳소리가 난 방향을 바라보세요.'),
    'GONG_ACT_IDENTIFY_ENEMY': ('OBSERVE', '접근 중인 적의 위치와 규모를 확인하세요.'),
    'GONG_ACT_REPORT_ENEMY': ('TRIGGER', '보고 장치를 트리거로 눌러 동쪽 적군 6명 이상을 보고하세요.'),
    'GONG_ACT_CHECK_BEACON': ('OBSERVE', '봉돈의 신호를 바라보세요.'),
    'GONG_ACT_SHOOT_RETREATING': ('COMBAT', '퇴각하는 적을 조준하고 발사하세요.'),
    'GONG_ACT_FINAL_SCAN': ('OBSERVE', '주변에 남은 적이 없는지 확인하세요.'),
}

ORDERED_INTERACTION_IDS = [
    'GONG_NAR_01', 'GONG_NAR_02', 'GONG_NAR_03', 'GONG_NAR_04',
    'GONG_ACT_SCAN_PERIMETER', 'GONG_ACT_SOUND_ANIMAL', 'GONG_NAR_05',
    'GONG_ACT_CHECK_ANIMAL', 'GONG_NAR_06', 'GONG_NAR_07', 'GONG_NAR_08',
    'GONG_ACT_SOUND_METAL', 'GONG_ACT_CHECK_METAL', 'GONG_ACT_REVEAL_ENEMY',
    'GONG_NAR_09', 'GONG_NAR_10', 'GONG_ACT_IDENTIFY_ENEMY',
    'GONG_NAR_11', 'GONG_NAR_12', 'GONG_NAR_13', 'GONG_ACT_REPORT_ENEMY',
    'GONG_NAR_14', 'GONG_NAR_15', 'GONG_NAR_16', 'GONG_ACT_ENABLE_BEACON',
    'GONG_ACT_CHECK_BEACON', 'GONG_NAR_17', 'GONG_NAR_18', 'GONG_NAR_19',
    'GONG_ACT_RETREAT_ENEMY', 'GONG_NAR_20', 'GONG_NAR_21', 'GONG_NAR_22',
    'GONG_ACT_SHOOT_RETREATING', 'GONG_NAR_23', 'GONG_NAR_24',
    'GONG_ACT_FINAL_SCAN', 'GONG_NAR_25', 'GONG_NAR_26', 'GONG_NAR_27',
]


def create_or_update_narration_table(unreal):
    table = unreal.load_asset(NARRATION_TABLE_PATH)
    if not table:
        row_struct = unreal.load_object(None, '/Script/SuwonSiegeContestVR.NarrationSequenceRow')
        if not row_struct:
            raise RuntimeError('FNarrationSequenceRow could not be loaded')
        factory = unreal.DataTableFactory()
        factory.set_editor_property('struct', row_struct)
        table = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            'DT_Narration_Gongsimdon', '/GF_Gongsimdon/Data', unreal.DataTable, factory
        )
    if not table:
        raise RuntimeError('DT_Narration_Gongsimdon could not be created')

    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    asset_registry.scan_paths_synchronous([NARRATION_DIRECTORY], True)
    audio_by_index = {}
    for asset_data in asset_registry.get_assets_by_path(
            NARRATION_DIRECTORY, recursive=False):
        asset_path = str(asset_data.package_name)
        asset_name = str(asset_data.asset_name)
        match = re.match(r'^(\d{2})_', asset_name)
        if not match:
            continue
        index = int(match.group(1))
        asset = unreal.load_asset(asset_path)
        if asset:
            audio_by_index[index] = asset.get_path_name()

    missing = [index for index in range(1, 28) if index not in audio_by_index]
    if missing:
        raise RuntimeError('Missing Gongsimdon narration assets: ' + ', '.join(map(str, missing)))

    rows = []
    for index in range(1, 28):
        rows.append({
            'Name': f'GONG_NA_{index:02d}',
            'SpeakerName': '해설',
            'Subtitle': SUBTITLES[index],
            'NarrationSound': audio_by_index[index],
            'PreviewDuration': 3.0,
            'NextRow': 'None',
            'AdvanceMode': 'Stop',
            'AdvanceDelay': 0.0,
            'CompletionEvents': [],
            'PostNarrationWidgetClass': 'None',
        })
    if not unreal.DataTableFunctionLibrary.fill_data_table_from_json_string(
            table, json.dumps(rows, ensure_ascii=False)):
        raise RuntimeError('Could not populate DT_Narration_Gongsimdon')
    if not unreal.EditorAssetLibrary.save_loaded_asset(table, only_if_is_dirty=False):
        raise RuntimeError('Could not save DT_Narration_Gongsimdon')
    return table


def make_gongsimdon_stage(unreal):
    type_by_name = {
        'OBSERVE': unreal.ScenarioInteractionType.OBSERVE,
        'SEQUENCE': unreal.ScenarioInteractionType.SEQUENCE,
        'SPAWN': unreal.ScenarioInteractionType.SPAWN,
        'CUSTOM': unreal.ScenarioInteractionType.CUSTOM,
        'COMBAT': unreal.ScenarioInteractionType.COMBAT,
    }
    guide_by_name = {
        'OBSERVE': unreal.ScenarioGuideAction.OBSERVE,
        'SPEAK': unreal.ScenarioGuideAction.SPEAK,
        'TRIGGER': unreal.ScenarioGuideAction.TRIGGER,
        'COMBAT': unreal.ScenarioGuideAction.COMBAT,
    }
    interactions = []
    for position, interaction_id in enumerate(ORDERED_INTERACTION_IDS):
        next_id = (ORDERED_INTERACTION_IDS[position + 1]
                   if position + 1 < len(ORDERED_INTERACTION_IDS) else 'None')
        interaction = unreal.ScenarioInteraction()
        interaction.set_editor_property('interaction_id', interaction_id)
        interaction.set_editor_property('next_interaction_id', next_id)
        interaction.set_editor_property('required', True)
        if interaction_id.startswith('GONG_NAR_'):
            index = int(interaction_id[-2:])
            interaction.set_editor_property(
                'interaction_type', unreal.ScenarioInteractionType.NARRATION)
            interaction.set_editor_property('narration_id', f'GONG_NA_{index:02d}')
        else:
            type_name, target_id, complete_on_start = ACTION_DEFINITIONS[interaction_id]
            interaction.set_editor_property('interaction_type', type_by_name[type_name])
            interaction.set_editor_property('target_id', target_id)
            interaction.set_editor_property('complete_on_start', complete_on_start)
            interaction.set_editor_property('fail_interaction_id', interaction_id)
            if interaction_id in ACTION_GUIDES:
                guide_name, guide_text = ACTION_GUIDES[interaction_id]
                interaction.set_editor_property('guide_action', guide_by_name[guide_name])
                interaction.set_editor_property('guide_text', guide_text)
        interactions.append(interaction)

    stage = unreal.ScenarioStageDefinition()
    stage.set_editor_property('stage_id', 'GONGSIMDON_STAGE_01')
    stage.set_editor_property('stage_name', '공심돈 야간 경계')
    stage.set_editor_property('start_interaction_id', ORDERED_INTERACTION_IDS[0])
    stage.set_editor_property('interactions', interactions)
    return stage
