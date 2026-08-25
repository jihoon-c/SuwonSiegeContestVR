import json
import re

import unreal


NARRATION_DIRECTORY = "/Game/Audio/Narration"
NARRATION_TABLE_PATH = f"{NARRATION_DIRECTORY}/DT_Narration_Main"
SCENARIO_PATH = "/Game/Data/DA_Scenario_MainEducation"

SUBTITLES = {
    1: "신임 지휘관님, 수원화성에 오신 것을 환영합니다.",
    2: "수원화성은 조선 정조 때 건설된 성곽입니다. 적을 살피고 공격하기 위한 방어시설뿐 아니라, 성을 짓고 관리하는 데 필요한 여러 기술도 함께 활용되었습니다.",
    3: "이제 주요 시설과 장치를 하나씩 살펴보면서 어떤 목적으로 만들어졌고 어떻게 활용되었는지 알아보겠습니다.",
    4: "먼저 성벽에서 이루어지는 방어를 살펴보겠습니다.",
    5: "적이 성으로 접근한다면 병사들은 성벽과 여러 방어시설을 이용해 진입을 막아야 했습니다.",
    6: "수원화성이 완성된 조선 후기에는 조총과 화포와 같은 화약무기도 활용되었습니다.",
    7: "이번 교육에서는 조선의 화약 기술을 체험하기 위해, 그보다 이전 시대부터 사용된 신기전을 가상의 수원화성 방어 상황에 활용해 보겠습니다.",
    8: "화약무기가 실제 상황에서 어떤 방식으로 사용되는지 직접 확인해 보겠습니다.",
    9: "성을 지키기 위해서는 먼저 적이 어디에서 접근하는지 확인해야 합니다.",
    10: "수원화성에는 높은 위치에서 주변을 살피고, 내부에 몸을 보호하면서 적을 공격할 수도 있는 방어시설이 있습니다.",
    11: "이 시설의 이름은 무엇일까요?",
    12: "정답은 공심돈입니다.",
    13: "공심돈은 내부가 비어 있는 높은 방어시설로, 병사들이 안에서 성 밖을 관찰하고 적을 공격할 수 있도록 만들어졌습니다.",
    14: "높은 위치에 있기 때문에 주변을 넓게 살펴볼 수 있고, 병사는 시설 안에서 몸을 보호하면서 적의 움직임을 확인할 수 있었습니다.",
    15: "그렇다면 공심돈에서 실제로 경계를 서다가 적의 움직임을 발견하면 어떻게 해야 할까요?",
    16: "직접 안으로 들어가 주변을 살펴보겠습니다.",
    17: "이번에는 성문을 살펴보겠습니다.",
    18: "성문은 사람과 물자가 드나드는 중요한 통로이지만, 적이 성 안으로 진입하기 위해 집중적으로 공격할 수 있는 곳이기도 합니다.",
    19: "수원화성에서는 성문을 한 번 더 보호하기 위한 별도의 방어시설을 두었습니다.",
    20: "성문을 적의 공격으로부터 보호하기 위해 성문 바깥을 둘러싸도록 만든 방어시설은 무엇일까요?",
    21: "정답은 옹성입니다.",
    22: "옹성은 성문 바깥을 다시 성벽으로 둘러싸, 적이 성문까지 곧바로 접근하기 어렵게 만든 방어시설입니다.",
    23: "옹성이 있으면 적은 바로 성문으로 접근하지 못하고 제한된 공간을 통과해야 합니다.",
    24: "그 과정에서 성을 지키는 병사들이 적의 움직임에 더욱 효과적으로 대응할 수 있었습니다.",
    25: "그렇다면 적이 성문으로 공격해 온다면 옹성은 실제로 어떤 역할을 하게 될까요?",
    26: "직접 확인해 보겠습니다.",
    27: "성을 지키려면 병사뿐 아니라 무기와 물자도 필요한 곳으로 옮겨야 합니다.",
    28: "하지만 무거운 물건을 사람이 직접 높은 곳까지 들어 올리는 것은 쉽지 않습니다.",
    29: "이때 도르래를 이용한 장치를 활용할 수 있었습니다.",
    30: "도르래의 원리를 이용해 무거운 물건을 위아래로 움직이는 장치는 무엇일까요?",
    31: "정답은 녹로입니다.",
    32: "녹로는 도르래와 줄을 이용해 무거운 물체를 들어 올리거나 이동하는 데 활용한 장치입니다.",
    33: "도르래를 활용하면 힘의 방향을 바꾸거나 여러 줄에 힘을 나누어, 무거운 물체를 보다 효율적으로 움직일 수 있습니다.",
}

# A Scenario narration interaction starts at each segment's first Row. The final
# Row is Stop so it cannot spill into the next educational screen or quiz.
SEGMENTS = (
    (1, 2, 3),
    (4, 5, 6, 7),
    (8,),
    (9, 10, 11),
    (12, 13),
    (14,),
    (15, 16),
    (17, 18, 19, 20),
    (21, 22),
    (23, 24),
    (25, 26),
    (27, 28, 29, 30),
    (31, 32),
    (33,),
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

unreal.log("MAIN_NARRATION CREATE SUCCESS: 33 rows linked to DA_Scenario_MainEducation")

