import json
import unreal


TABLE_PATH = "/GF_OngseongCrossbow/Data/DT_OngseongNarration"
SPEAKER = "교관"

# The image supplied by the scenario team is authoritative for these subtitles.
LINES = {
    1: "지금부터 성문을 지키는 법을 익히도록 합시다.",
    2: "성문은 성에서 가장 중요한 곳인 동시에, 적이 가장 먼저 노리는 곳입니다.\n그래서 성문 앞을 이렇게 성벽으로 한 번 더 둘러 방어하죠.\n이것이 바로 옹성입니다.",
    3: "옹성은 성문을 반달 모양으로 감싸는 방어 시설입니다.\n적이 성문을 공격하려면 먼저 이 안으로 들어와야 하죠.",
    4: "안으로 들어온 적은 사방의 성벽에 둘러싸이게 되고 수비병들은 높은 성벽 위에서 적을 내려다보며 공격할 수 있지요.",
    5: "즉, 성문을 공격하러 들어온 적을 오히려 한곳에 몰아넣어 공격하는 구조인 셈입니다.\n그리고 옹성에 들어온 적을 상대할 때 사용할 무기가 바로 총통입니다.",
    6: "총통은 화약의 힘으로 탄환을 발사하는 화포이죠.\n적이 몰려오는 곳을 잘 살펴 적절한 위치에서 발사한다면, 여러 적을 효과적으로 막아낼 수 있습니다.",
    7: "오늘 지휘관님이 직접 이 총통을 다뤄보게 될 것입니다.",
    8: "지금부터 실제 상황을 가정한 수비 훈련을 시작하겠습니다.",
    9: "지휘관님은 옹성 위의 총통을 맡으십시오.",
    10: "적이 쳐들어오고있습니다!",
    11: "당황하지 마십시오! 총통을 이용해 적을 물리치는겁니다.",
    12: "먼저 총통을 집어 발사를 준비하십시오.",
    13: "화약을 집어 총통의 포구에 넣으십시오.",
    14: "수시개를 잡고 포구 안으로 세 번 밀어 넣어 화약을 다지십시오.",
    15: "대포알을 집어 총통의 포구에 넣으십시오.",
    16: "화승을 장전부에 가져다 대어 발사를 준비하십시오.",
    17: "좋습니다! 이제 총통을 발사할 수 있습니다. 모든 대포알을 발사했으면 다시 아까의 과정을 통해 장전할 수 있습니다.",
    18: "대포알을 모두 발사했습니다. 화약을 넣어 재장전을 하십시오.",
    19: "적이 총차를 견조중입니다! 어서 총통으로 그들을 격파하십시오",
    20: "아군이 공격받고 있습니다. 적 공병을 처치하여 아군을 보호하십시오",
    21: "성문이 공격받고 있습니다. 어서 총차를 파괴하십시오!",
    22: "적의 공세를 성공적으로 막았습니다. 대단하십니다 지휘관님!",
    23: "성문이 파괴되었습니다. 일단 후퇴합니다!",
}

# Only uninterrupted instructional groups are linked. Gameplay and situation changes
# enter through UOngseongNarrationComponent's named-event queue.
NEXT_ROWS = {
    **{index: index + 1 for index in range(1, 9)},
    10: 11,
    11: 12,
    12: 13,
    16: 17,
}


def create_or_load_table():
    table = unreal.load_asset(TABLE_PATH)
    if table:
        return table

    row_struct = unreal.load_object(None, "/Script/SuwonSiegeContestVR.NarrationSequenceRow")
    if not row_struct:
        raise RuntimeError("Could not load FNarrationSequenceRow")
    factory = unreal.DataTableFactory()
    factory.set_editor_property("struct", row_struct)
    table = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "DT_OngseongNarration",
        "/GF_OngseongCrossbow/Data",
        unreal.DataTable,
        factory,
    )
    if not table:
        raise RuntimeError("Could not create DT_OngseongNarration")
    return table


table = create_or_load_table()
rows = []
for index, subtitle in LINES.items():
    next_index = NEXT_ROWS.get(index)
    rows.append(
        {
            "Name": f"ON_{index:02d}",
            "SpeakerName": SPEAKER,
            "Subtitle": subtitle,
            "NarrationSound": "None",
            "PreviewDuration": max(2.5, min(12.0, len(subtitle.replace("\n", "")) / 8.0)),
            "NextRow": f"ON_{next_index:02d}" if next_index else "None",
            "AdvanceMode": "Auto" if next_index else "Stop",
            "AdvanceDelay": 0.15 if next_index else 0.0,
            "CompletionEvents": [],
            "PostNarrationWidgetClass": "None",
        }
    )

if not unreal.DataTableFunctionLibrary.fill_data_table_from_json_string(
    table, json.dumps(rows, ensure_ascii=False)
):
    raise RuntimeError("Could not populate DT_OngseongNarration")
if not unreal.EditorAssetLibrary.save_loaded_asset(table, only_if_is_dirty=False):
    raise RuntimeError("Could not save DT_OngseongNarration")
unreal.log("ONGSEONG_NARRATION_ASSET_SETUP SUCCESS: 23 rows")
