import json
import re

import unreal


NARRATION_DIRECTORY = "/Game/Audio/Narration2"
# Keep the established table asset path so the already placed Main-level
# scenario manager continues to resolve it without a level-actor replacement.
NARRATION_TABLE_PATH = "/Game/Audio/Narration/DT_Narration_Main"
SCENARIO_PATH = "/Game/Data/DA_Scenario_MainEducation"

SUBTITLES = {
    1: "먼저 수원화성을 건설할 때 사용된 장치부터 살펴보겠습니다.",
    2: "이것은 거중기와 녹로입니다.",
    3: "수원화성을 쌓기 위해서는 무거운 돌과 많은 건축 자재를 옮겨야 했습니다.",
    4: "거중기는 도르래의 원리를 이용해 무거운 물체를 적은 힘으로 들어 올릴 수 있도록 만든 장치입니다.",
    5: "녹로 역시 도르래를 이용하여 무거운 돌이나 자재를 높은 곳으로 들어 올리는 데 활용되었습니다.",
    6: "이런 장치들은 성을 보다 효율적으로 건설하는 데 큰 도움을 주었습니다.",
    7: "이제 성이 완성된 뒤 어떻게 주변을 감시하고 적의 공격에 대비했는지 살펴보겠습니다.",
    8: "지휘관님, 이것은 공심돈입니다.",
    9: "공심돈은 내부가 비어 있는 높은 망루 형태의 방어 시설입니다.",
    10: "군사들은 이곳에서 성 밖을 감시하고 적의 움직임을 살필 수 있었습니다.",
    11: "또한 여러 층에 마련된 총안 등을 이용하여 성벽 가까이 접근하는 적을 공격할 수 있었습니다.",
    12: "높은 곳에서 주변을 넓게 살펴볼 수 있기 때문에 적의 접근을 빠르게 확인하는 데에도 유리했습니다.",
    13: "이번에는 조선의 화약 무기인 신기전을 살펴보겠습니다.",
    14: "신기전은 화살에 화약을 넣은 약통을 달아 화약의 힘으로 날아가도록 만든 무기입니다.",
    15: "도화선에 불을 붙이면 약통 속 화약이 연소하면서 발생하는 힘으로 앞으로 날아갑니다.",
    16: "특히 여러 발의 신기전을 화차에 장착하면 한꺼번에 많은 신기전을 발사할 수 있었습니다.",
    17: "이러한 화약 무기는 많은 적이 접근하는 상황에서 효과적으로 사용할 수 있었습니다.",
    18: "이것은 성문 앞을 둘러싸도록 만든 방어 시설, 옹성입니다.",
    19: "성문은 사람들이 성을 드나드는 중요한 통로지만 적에게는 집중적으로 공격할 수 있는 약점이 되기도 합니다.",
    20: "그래서 성문 바깥에 성벽을 한 겹 더 둘러 적이 성문에 바로 접근하지 못하도록 했습니다.",
    21: "적이 옹성 안으로 들어오면 움직일 수 있는 공간이 제한되고 주변 성벽의 공격에도 노출됩니다.",
    22: "즉 옹성은 성문의 약점을 보완하고 적의 진입을 어렵게 만들기 위한 방어 시설입니다.",
    23: "지휘관님께서는 이 옹성이 실제 방어 상황에서 어떻게 활용되는지도 직접 확인하셔야 합니다.",
    24: "이번에는 옹성의 구조를 이용해 성문을 방어하는 상황을 직접 체험해 보겠습니다.",
    25: "수고하셨습니다, 지휘관님.",
    26: "오늘 거중기와 녹로를 통해 수원화성의 건설 기술을 살펴보고 공심돈을 통해 성을 감시하고 방어하는 방법을 알아보았습니다.",
    27: "그리고 신기전과 옹성을 직접 체험하면서 수원화성이 적의 공격에 어떻게 대비했는지도 알아보았습니다.",
    28: "수원화성은 단순히 높은 성벽으로 이루어진 것이 아니라 다양한 시설과 무기를 서로 연계하여 방어력을 높인 성곽입니다.",
    29: "이제 지휘관님도 수원화성의 주요 시설과 방어 체계를 충분히 익히셨습니다.",
    30: "이것으로 교육을 모두 마치겠습니다.",
}

# A Scenario narration interaction starts at each segment's first Row. The final
# Row is Stop so it cannot spill into the next educational screen or quiz.
SEGMENTS = (
    (1,),             # 성문 앞: 축성 장치 도입
    (2, 3, 4),        # 거중기 이미지
    (5, 6, 7),        # 녹로 이미지 및 공심돈 전환
    (8, 9, 10, 11, 12),
    (13, 14, 15, 16, 17),
    (18, 19, 20, 21, 22, 23, 24),
    (25, 26, 27, 28, 29, 30),
)


def load_numbered_audio():
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    registry.scan_paths_synchronous([NARRATION_DIRECTORY], True)
    result = {}
    for asset_data in registry.get_assets_by_path(NARRATION_DIRECTORY, recursive=False):
        match = re.match(r"^(\d{2})_", str(asset_data.asset_name))
        if not match:
            continue
        index = int(match.group(1))
        asset = unreal.load_asset(str(asset_data.package_name))
        if asset:
            result[index] = asset.get_path_name()
    missing = sorted(set(SUBTITLES) - set(result))
    if missing:
        raise RuntimeError("Missing Main narration SoundWave assets: " + ", ".join(map(str, missing)))
    return result


def create_or_load_table():
    table = unreal.load_asset(NARRATION_TABLE_PATH)
    if table:
        return table
    row_struct = unreal.load_object(None, "/Script/SuwonSiegeContestVR.NarrationSequenceRow")
    if not row_struct:
        raise RuntimeError("FNarrationSequenceRow could not be loaded")
    factory = unreal.DataTableFactory()
    factory.set_editor_property("struct", row_struct)
    table = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "DT_Narration_Main", NARRATION_DIRECTORY, unreal.DataTable, factory
    )
    if not table:
        raise RuntimeError("DT_Narration_Main could not be created")
    return table


audio_by_index = load_numbered_audio()
next_by_index = {}
for segment in SEGMENTS:
    for offset, index in enumerate(segment):
        next_by_index[index] = segment[offset + 1] if offset + 1 < len(segment) else None

rows = []
for index in sorted(SUBTITLES):
    next_index = next_by_index[index]
    rows.append({
        "Name": f"MAIN_NA_{index:02d}",
        "SpeakerName": "교관",
        "Subtitle": SUBTITLES[index],
        "NarrationSound": audio_by_index[index],
        "PreviewDuration": 3.0,
        "NextRow": f"MAIN_NA_{next_index:02d}" if next_index else "None",
        "AdvanceMode": "Auto" if next_index else "Stop",
        "AdvanceDelay": 0.0,
        "CompletionEvents": [],
        "PostNarrationWidgetClass": "None",
    })

table = create_or_load_table()
if not unreal.DataTableFunctionLibrary.fill_data_table_from_json_string(
        table, json.dumps(rows, ensure_ascii=False)):
    raise RuntimeError("Could not populate DT_Narration_Main")
if not unreal.EditorAssetLibrary.save_loaded_asset(table, only_if_is_dirty=False):
    raise RuntimeError("Could not save DT_Narration_Main")

scenario = unreal.load_asset(SCENARIO_PATH)
if not isinstance(scenario, unreal.MainEducationScenarioDefinition):
    raise RuntimeError("DA_Scenario_MainEducation is missing or has the wrong class")

# Refresh only the Scenario graph from the compiled class default. EducationContent
# remains untouched so assigned images and UI metadata are preserved.
scenario_cdo = unreal.get_default_object(scenario.get_class())
scenario.set_editor_property("stages", scenario_cdo.get_editor_property("stages"))
scenario.set_editor_property("start_stage_id", scenario_cdo.get_editor_property("start_stage_id"))
scenario.set_editor_property("narration_table", table)
if not unreal.EditorAssetLibrary.save_loaded_asset(scenario, only_if_is_dirty=False):
    raise RuntimeError("Could not save DA_Scenario_MainEducation")

unreal.log("MAIN_NARRATION2 APPLY SUCCESS: 30 rows linked to DA_Scenario_MainEducation")

