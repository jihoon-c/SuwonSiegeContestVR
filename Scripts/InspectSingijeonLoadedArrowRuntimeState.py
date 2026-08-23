import unreal


def log_scene(label, component):
    parent = component.get_attach_parent()
    unreal.log(
        f"SINGIJEON_ARROW_STATE {label} "
        f"name={component.get_name()} parent={parent.get_name() if parent else None} "
        f"relative_location={component.get_editor_property('relative_location')} "
        f"relative_rotation={component.get_editor_property('relative_rotation')} "
        f"relative_scale={component.get_editor_property('relative_scale3d')} "
        f"absolute_location={component.get_editor_property('absolute_location')} "
        f"absolute_rotation={component.get_editor_property('absolute_rotation')} "
        f"absolute_scale={component.get_editor_property('absolute_scale')} "
        f"mobility={component.get_editor_property('mobility')}"
    )


hwacha_bp = unreal.load_asset('/GF_Singijeon/Gameplay/BP_SingijeonHwacha')
arrow_bp = unreal.load_asset('/GF_Singijeon/Gameplay/BP_SingijeonArrow')
arrow_mesh = unreal.load_asset('/GF_Singijeon/Asset/Arrow/arrowb/StaticMeshes/arrowb')
material = unreal.load_asset('/GF_Singijeon/Asset/Arrow/arrowb/Materials/M_SingijeonArrow_Runtime')
if not hwacha_bp or not arrow_bp or not arrow_mesh or not material:
    raise RuntimeError('Required Singijeon assets are missing')

unreal.BlueprintEditorLibrary.compile_blueprint(hwacha_bp)
unreal.BlueprintEditorLibrary.compile_blueprint(arrow_bp)

hwacha_cdo = unreal.get_default_object(hwacha_bp.generated_class())
for prop in ('body_mesh', 'rack_root', 'default_ammo_slot', 'auto_loaded_arrow_instances'):
    component = hwacha_cdo.get_editor_property(prop)
    log_scene(f'hwacha_cdo.{prop}', component)
    if isinstance(component, unreal.StaticMeshComponent):
        unreal.log(f'SINGIJEON_ARROW_STATE hwacha_cdo.{prop} material0={component.get_material(0)}')

arrow_cdo = unreal.get_default_object(arrow_bp.generated_class())
projectile_mesh = arrow_cdo.get_editor_property('projectile_mesh')
log_scene('arrow_cdo.projectile_mesh', projectile_mesh)
unreal.log(f'SINGIJEON_ARROW_STATE arrow_cdo mesh={projectile_mesh.get_editor_property("static_mesh")}')
unreal.log(f'SINGIJEON_ARROW_STATE arrow_cdo material0={projectile_mesh.get_material(0)}')
unreal.log(f'SINGIJEON_ARROW_STATE static_mesh material0={arrow_mesh.get_material(0)}')

for map_path in ('/GF_Singijeon/Maps/LV_Singijeon', '/Game/Maps/LV_Singijeon'):
    if not unreal.EditorAssetLibrary.does_asset_exist(map_path):
        continue
    unreal.EditorLoadingAndSavingUtils.load_map(map_path)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
    unreal.log(f'SINGIJEON_ARROW_STATE level={map_path}')
    for actor in actors:
        if isinstance(actor, unreal.SingijeonHwachaActor):
            unreal.log(f'SINGIJEON_ARROW_STATE hwacha={actor.get_actor_label()} transform={actor.get_actor_transform()}')
            for component in actor.get_components_by_class(unreal.SceneComponent):
                if component.get_name() in ('BodyMesh', 'RackRoot', 'DefaultAmmoSlot', 'AutoLoadedArrowInstances'):
                    log_scene(f'placed_hwacha.{component.get_name()}', component)
                    if isinstance(component, unreal.StaticMeshComponent):
                        unreal.log(f'SINGIJEON_ARROW_STATE placed_hwacha.{component.get_name()} material0={component.get_material(0)}')
        if isinstance(actor, unreal.SingijeonProjectileActor):
            mesh_component = actor.get_editor_property('projectile_mesh')
            log_scene(f'placed_arrow.{actor.get_actor_label()}', mesh_component)
            unreal.log(f'SINGIJEON_ARROW_STATE placed_arrow.{actor.get_actor_label()} material0={mesh_component.get_material(0)}')

unreal.log('SINGIJEON_LOADED_ARROW_RUNTIME_STATE INSPECT SUCCESS')
