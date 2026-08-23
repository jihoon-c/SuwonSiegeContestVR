import unreal

ASSET_PATH = "/GF_Singijeon/Gameplay/Enemy/Samurai/DA_SingijeonSamuraiRifleRun_GPU"
MESH_PATH = "/GF_Singijeon/Gameplay/Enemy/Samurai/SKM_Low_Poly_Samurai_VR"
RUN_PATH = "/GF_Singijeon/Gameplay/Enemy/Samurai/MF_Rifle_Jog_Fwd_Samurai"
VARIANT_COUNT = 12

mesh = unreal.load_asset(MESH_PATH)
run = unreal.load_asset(RUN_PATH)
if not mesh or not run:
    raise RuntimeError("Samurai mesh or retargeted Rifle Jog is missing")

provider = unreal.load_asset(ASSET_PATH)
if not provider:
    factory = unreal.TransformProviderDataFactory()
    factory.set_editor_property("provider_data_class", unreal.AnimSequenceTransformProviderData)
    provider = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "DA_SingijeonSamuraiRifleRun_GPU",
        "/GF_Singijeon/Gameplay/Enemy/Samurai",
        unreal.AnimSequenceTransformProviderData, factory)
if not provider:
    raise RuntimeError("Could not create Samurai GPU animation provider")

length = max(float(run.get_play_length()), 0.01)
sequences = []
for index in range(VARIANT_COUNT):
    sequence = unreal.AnimSequenceTransformProviderSequence()
    sequence.set_editor_property("sequence", run)
    sequence.set_editor_property("play_rate", 0.86 + index * (0.28 / (VARIANT_COUNT - 1)))
    sequence.set_editor_property("position", length * index / VARIANT_COUNT)
    sequences.append(sequence)
provider.set_editor_property("skinned_asset", mesh)
provider.set_editor_property("sequences", sequences)
provider.set_editor_property("layers", [unreal.AnimSequenceTransformProviderLayer()])
provider.modify()
unreal.EditorAssetLibrary.save_asset(ASSET_PATH, only_if_is_dirty=False)
unreal.log(f"SAMURAI_GPU_PROVIDER SUCCESS variants={VARIANT_COUNT}")
