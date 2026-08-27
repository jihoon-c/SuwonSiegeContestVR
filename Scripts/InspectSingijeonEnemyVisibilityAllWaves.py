import unreal


LEVEL_PATH = "/Game/Maps/LV_Singijeon"

unreal.EditorLoadingAndSavingUtils.load_map(LEVEL_PATH)
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = list(actor_subsystem.get_all_level_actors())
waves = sorted(
    (actor for actor in actors if isinstance(actor, unreal.SingijeonEnemyWaveActor)),
    key=lambda actor: actor.get_actor_label(),
)
hwachas = [actor for actor in actors if isinstance(actor, unreal.SingijeonHwachaActor)]
if not waves or not hwachas:
    raise RuntimeError("LV_Singijeon must contain a Hwacha and at least one Enemy Wave")

for anchor in actors:
    class_name = anchor.get_class().get_name()
    if "PlayerStart" in class_name or "VRPlayerPawn" in class_name:
        unreal.log(
            f"ENEMY_VIS player_anchor={anchor.get_actor_label()} class={class_name} "
            f"location={anchor.get_actor_location()} forward={anchor.get_actor_forward_vector()}")

editor_world = unreal.get_editor_subsystem(
    unreal.UnrealEditorSubsystem).get_editor_world()
total_runtime_actors = 0
for wave in waves:
    label = wave.get_actor_label()
    mesh = wave.get_editor_property("proxy_skeletal_mesh")
    scene_root = wave.get_editor_property("scene_root")
    spawn_volume = wave.get_editor_property("spawn_volume")
    previews = [
        component
        for component in wave.get_components_by_class(unreal.SkeletalMeshComponent)
        if component.get_name().startswith("EnemyPreview_")
    ]
    unreal.log(
        f"ENEMY_VIS WAVE label={label} path={wave.get_path_name()} "
        f"location={wave.get_actor_location()} rotation={wave.get_actor_rotation()} "
        f"spawn={spawn_volume.get_world_location()} preview_count={len(previews)} "
        f"enemy_count={wave.get_editor_property('enemy_count')} "
        f"show_ready={wave.get_editor_property('show_enemies_while_ready')} "
        f"gpu={wave.get_editor_property('use_gpu_instanced_crowd')} mesh={mesh}")
    unreal.log(
        f"ENEMY_VIS WAVE_FLAGS label={label} "
        f"actor_hidden={wave.get_editor_property('hidden')} "
        f"root_visible={scene_root.get_editor_property('visible')} "
        f"root_hidden_game={scene_root.get_editor_property('hidden_in_game')}")

    if not mesh:
        raise RuntimeError(f"{label}: ProxySkeletalMesh is empty")
    if not wave.get_editor_property("show_enemies_while_ready"):
        raise RuntimeError(f"{label}: enemies are configured hidden while Ready")
    if wave.get_editor_property("use_gpu_instanced_crowd"):
        raise RuntimeError(f"{label}: unsafe OpenXR GPU crowd path is enabled")

    prepared = wave.prepare_wave()
    proxy_actors = [
        actor
        for actor in unreal.GameplayStatics.get_all_actors_of_class(
            editor_world, unreal.EnemySoldierActor)
        if actor.get_owner() == wave
    ]
    total_runtime_actors += len(proxy_actors)
    visible_count = 0
    hidden_actor_count = 0
    hidden_mesh_count = 0
    main_pass_count = 0
    nearest_distance = float("inf")
    for proxy_actor in proxy_actors:
        proxy_mesh = proxy_actor.get_editor_property("mesh")
        if proxy_actor.get_editor_property("hidden"):
            hidden_actor_count += 1
        else:
            visible_count += 1
        if proxy_mesh.get_editor_property("hidden_in_game"):
            hidden_mesh_count += 1
        if proxy_mesh.get_editor_property("render_in_main_pass"):
            main_pass_count += 1
        nearest_distance = min(
            nearest_distance,
            (proxy_actor.get_actor_location() - spawn_volume.get_world_location()).length(),
        )

    unreal.log(
        f"ENEMY_VIS RUNTIME label={label} prepared={prepared} "
        f"state={wave.get_wave_state()} logical={wave.get_logical_enemy_count()} "
        f"interactive_budget={wave.get_runtime_interactive_enemy_budget()} "
        f"pose_budget={wave.get_runtime_pose_leader_budget()} "
        f"owned_actors={len(proxy_actors)} actor_visible={visible_count} "
        f"actor_hidden={hidden_actor_count} mesh_hidden={hidden_mesh_count} "
        f"main_pass={main_pass_count} "
        f"nearest_to_spawn={nearest_distance:.1f}")

    expected = wave.get_editor_property("enemy_count")
    if not prepared or len(proxy_actors) != expected:
        raise RuntimeError(
            f"{label}: expected {expected} VR-safe EnemySoldier Actors, "
            f"found {len(proxy_actors)}")
    if hidden_actor_count or hidden_mesh_count:
        raise RuntimeError(f"{label}: runtime enemies were hidden after PrepareWave")
    if main_pass_count != expected:
        raise RuntimeError(f"{label}: runtime meshes are not ready for the main pass")

unreal.log(
    f"SINGIJEON_ENEMY_VISIBILITY INSPECT SUCCESS: waves={len(waves)} "
    f"runtime_enemy_actors={total_runtime_actors}")
