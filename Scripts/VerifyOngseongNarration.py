import json
import unreal


TABLE_PATH = "/GF_OngseongCrossbow/Data/DT_OngseongNarration"
table = unreal.load_asset(TABLE_PATH)
if not table:
    raise RuntimeError(f"Missing {TABLE_PATH}")

result = unreal.DataTableFunctionLibrary.export_data_table_to_json_string(table)
success, payload = result if isinstance(result, tuple) else (True, result)
if not success:
    raise RuntimeError("Could not export Ongseong narration table")
rows = json.loads(payload)
names = {row["Name"] for row in rows}
expected = {f"ON_{index:02d}" for index in range(1, 24)}
if names != expected:
    raise RuntimeError(f"Narration rows mismatch: missing={expected - names}, extra={names - expected}")
for row in rows:
    # FText may export as a string or as a localized-text JSON object.
    if "교관" not in json.dumps(row["SpeakerName"], ensure_ascii=False):
        raise RuntimeError(f"Unexpected speaker in {row['Name']}")
    if not row.get("Subtitle"):
        raise RuntimeError(f"Missing subtitle in {row['Name']}")
    next_row = row.get("NextRow", "None")
    if next_row not in ("", "None") and next_row not in names:
        raise RuntimeError(f"Broken NextRow in {row['Name']}: {next_row}")
    if not row.get("NarrationSound") or row["NarrationSound"] == "None":
        raise RuntimeError(f"Missing narration sound in {row['Name']}")

expected_subtitles = {
    "ON_19": "충차가 성문에 다가가고 있습니다! 어서 총통으로 충차를 파괴하십시오",
    "ON_20": "아군이 공격받고 있습니다. 적 궁병을 처치하여 아군 총통을 보호하십시오",
}
for row_name, expected_subtitle in expected_subtitles.items():
    if rows_by_name := next((row for row in rows if row["Name"] == row_name), None):
        if expected_subtitle not in json.dumps(rows_by_name["Subtitle"], ensure_ascii=False):
            raise RuntimeError(f"Unexpected revised subtitle in {row_name}")
    else:
        raise RuntimeError(f"Missing {row_name}")

unreal.log("ONGSEONG_NARRATION_VERIFY SUCCESS: 23 rows, 23 sounds, and revised subtitles validated")
