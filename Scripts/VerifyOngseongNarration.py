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
unreal.log("ONGSEONG_NARRATION_VERIFY SUCCESS: 23 rows and all links valid")
