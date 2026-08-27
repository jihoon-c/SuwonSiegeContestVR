import unreal


unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/LV_Singijeon")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
wave = next((actor for actor in actors if isinstance(actor, unreal.SingijeonEnemyWaveActor)), None)
hwacha = next((actor for actor in actors if isinstance(actor, unreal.SingijeonHwachaActor)), None)
if not wave or not hwacha:
    raise RuntimeError("LV_Singijeon must contain one Hwacha and Enemy Wave")

mesh = wave.get_editor_property("proxy_skeletal_mesh")
provider = wave.get_editor_property("proxy_animation_provider")
animation = wave.get_editor_property("foreground_run_animation")
character_instances = wave.get_editor_property("character_instances")
scene_root = wave.get_editor_property("scene_root")
spawn_volume = wave.get_editor_property("spawn_volume")
default_target = wave.get_editor_property("default_target_point")
editor_preview_components = [
    component for component in wave.get_components_by_class(unreal.SkeletalMeshComponent)
    if component.get_name().startswith("EnemyPreview_")
]

unreal.log(f"ENEMY_VIS wave_location={wave.get_actor_location()}")
unreal.log(f"ENEMY_VIS spawn_volume_world={spawn_volume.get_world_location()} "
           f"relative={spawn_volume.get_editor_property('relative_location')} "
           f"extent={spawn_volume.get_unscaled_box_extent()}")
unreal.log(f"ENEMY_VIS default_target_world={default_target.get_world_location()} "
           f"relative={default_target.get_editor_property('relative_location')}")
unreal.log(f"ENEMY_VIS hwacha_location={hwacha.get_actor_location()}")
unreal.log(f"ENEMY_VIS hwacha_forward={hwacha.get_actor_forward_vector()}")
unreal.log(f"ENEMY_VIS wave_to_hwacha={hwacha.get_actor_location() - wave.get_actor_location()}")
unreal.log(f"ENEMY_VIS mesh={mesh} provider={provider} animation={animation}")
unreal.log(f"ENEMY_VIS count={wave.get_editor_property('enemy_count')} "
           f"show_ready={wave.get_editor_property('show_enemies_while_ready')} "
           f"start_cull={wave.get_editor_property('proxy_start_cull_distance')} "
           f"end_cull={wave.get_editor_property('proxy_end_cull_distance')}")
unreal.log(f"ENEMY_VIS desired_height={wave.get_editor_property('desired_enemy_height')} "
           f"proxy_scale={wave.get_editor_property('proxy_scale')}")
unreal.log(f"ENEMY_VIS component_visible={character_instances.get_editor_property('visible')} "
           f"hidden_game={character_instances.get_editor_property('hidden_in_game')}")
for property_name in ("hidden", "hidden_editor", "hidden_editor_views", "hidden_editor_views"):
    try:
        unreal.log(f"ENEMY_VIS wave_{property_name}={wave.get_editor_property(property_name)}")
    except Exception as error:
        unreal.log(f"ENEMY_VIS wave_{property_name}=UNAVAILABLE:{error}")
unreal.log(f"ENEMY_VIS root_visible={scene_root.get_editor_property('visible')} "
           f"root_hidden_game={scene_root.get_editor_property('hidden_in_game')}")
unreal.log(f"ENEMY_VIS editor_preview_count={len(editor_preview_components)}")
if len(editor_preview_components) != wave.get_editor_property("enemy_count"):
    raise RuntimeError("Enemy Wave editor preview does not match Enemy Count")
if mesh:
    bounds = mesh.get_bounds()
    unreal.log(f"ENEMY_VIS mesh_bounds_origin={bounds.origin} extent={bounds.box_extent} "
               f"radius={bounds.sphere_radius}")
    unreal.log(f"ENEMY_VIS materials={mesh.get_editor_property('materials')}")
    skeletal_mesh_tools = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
    for lod_index in range(skeletal_mesh_tools.get_lod_count(mesh)):
        unreal.log(f"ENEMY_VIS lod_{lod_index}_sections={skeletal_mesh_tools.get_num_sections(mesh, lod_index)}")
    material = mesh.get_editor_property("materials")[0].material_interface
    base_material = material.get_base_material()
    unreal.log(f"ENEMY_VIS material={material} base={base_material} "
               f"blend={base_material.get_editor_property('blend_mode')} "
               f"two_sided={base_material.get_editor_property('two_sided')}")
    for parameter in unreal.MaterialEditingLibrary.get_texture_parameter_names(material):
        unreal.log(f"ENEMY_VIS texture_{parameter}="
                   f"{unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(material, parameter)}")

# Build the exact runtime representation in the editor world without saving it.
prepared = wave.prepare_wave()
editor_world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
proxy_actors = [
    actor
    for actor in unreal.GameplayStatics.get_all_actors_of_class(editor_world, unreal.EnemySoldierActor)
    if isinstance(actor, unreal.EnemySoldierActor) and actor.get_owner() == wave
]
skeletal_components = [
    actor.get_editor_property("mesh") for actor in proxy_actors
]
unreal.log(f"ENEMY_VIS prepared={prepared} runtime_enemy_components={len(skeletal_components)}")
if len(skeletal_components) != wave.get_editor_property("enemy_count"):
    raise RuntimeError(
        f"Expected 45 VR-safe enemy Actors, found {len(skeletal_components)}"
    )
nearest_proxy_distance = min(
    (component.get_world_location() - spawn_volume.get_world_location()).length()
    for component in skeletal_components
) if skeletal_components else float("inf")
unreal.log(f"ENEMY_VIS nearest_proxy_to_authored_spawn={nearest_proxy_distance}")
if nearest_proxy_distance > 800.0:
    raise RuntimeError(
        "Runtime enemies were projected away from the authored SpawnVolume: "
        f"nearest={nearest_proxy_distance:.1f} cm"
    )
for index, component in enumerate(skeletal_components[:5]):
    unreal.log(f"ENEMY_VIS proxy_{index}_location={component.get_world_location()} "
               f"scale={component.get_world_scale()} visible={component.get_editor_property('visible')} "
               f"hidden_game={component.get_editor_property('hidden_in_game')} "
               f"mesh={component.get_editor_property('skeletal_mesh')}")
for actor in actors:
    class_name = actor.get_class().get_name()
    if "PlayerStart" in class_name or "VRPlayerPawn" in class_name:
        unreal.log(f"ENEMY_VIS player_anchor={actor.get_name()} class={class_name} "
                   f"location={actor.get_actor_location()} forward={actor.get_actor_forward_vector()}")
unreal.log("SINGIJEON_ENEMY_VISIBILITY INSPECT SUCCESS")
