import unreal


ASSET_PATH = "/GF_Singijeon/Gameplay/Enemy/DA_SingijeonEnemyRun_GPU"
MESH_PATH = "/Game/NiagaraExamples/Gallery/SkeletalMesh/Mannequins/Meshes/SKM_Manny_Simple"
RUN_PATH = "/Game/NiagaraExamples/Gallery/SkeletalMesh/Mannequins/Anims/Rifle/Jog/MF_Rifle_Jog_Fwd"
PHASE_COUNT = 8

mesh = unreal.load_asset(MESH_PATH)
run_animation = unreal.load_asset(RUN_PATH)
if not mesh or not run_animation:
    raise RuntimeError("Manny mesh or rifle jog animation is missing")

provider = unreal.load_asset(ASSET_PATH)
if not provider:
    factory = unreal.TransformProviderDataFactory()
    factory.set_editor_property(
        "provider_data_class", unreal.AnimSequenceTransformProviderData)
    provider = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "DA_SingijeonEnemyRun_GPU",
        "/GF_Singijeon/Gameplay/Enemy",
        unreal.AnimSequenceTransformProviderData,
        factory,
    )
if not provider:
    raise RuntimeError("Could not create GPU enemy animation provider")

play_length = max(float(run_animation.get_play_length()), 0.01)
sequences = []
for phase_index in range(PHASE_COUNT):
    sequence = unreal.AnimSequenceTransformProviderSequence()
    sequence.set_editor_property("sequence", run_animation)
    sequence.set_editor_property("play_rate", 0.96 + (phase_index % 5) * 0.02)
    sequence.set_editor_property(
        "position", play_length * float(phase_index) / float(PHASE_COUNT))
    sequences.append(sequence)

provider.set_editor_property("skinned_asset", mesh)
provider.set_editor_property("sequences", sequences)
provider.set_editor_property("layers", [unreal.AnimSequenceTransformProviderLayer()])
provider.modify()

if not unreal.EditorAssetLibrary.save_asset(ASSET_PATH, only_if_is_dirty=False):
    raise RuntimeError("Could not save GPU enemy animation provider")

unreal.log(
    f"SINGIJEON GPU ENEMY PROVIDER SUCCESS: {PHASE_COUNT} shared run phases")
