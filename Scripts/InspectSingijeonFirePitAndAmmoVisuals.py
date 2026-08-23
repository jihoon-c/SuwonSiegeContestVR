import unreal


def log_component(label, component):
    unreal.log(f"SINGIJEON_VISUAL {label}: name={component.get_name()}")
    unreal.log(f"SINGIJEON_VISUAL {label}: relative={component.get_editor_property('relative_location')}")
    unreal.log(f"SINGIJEON_VISUAL {label}: world={component.get_world_location()}")
    unreal.log(f"SINGIJEON_VISUAL {label}: scale={component.get_editor_property('relative_scale3d')}")
    if isinstance(component, unreal.NiagaraComponent):
        unreal.log(f"SINGIJEON_VISUAL {label}: system={component.get_editor_property('asset')}")
        for prop in ('auto_activate', 'force_solo', 'allow_scalability'):
            try:
                unreal.log(f"SINGIJEON_VISUAL {label}: {prop}={component.get_editor_property(prop)}")
            except Exception as error:
                unreal.log_warning(f"SINGIJEON_VISUAL {label}: {prop} unavailable: {error}")


unreal.EditorLoadingAndSavingUtils.load_map('/Game/Maps/LV_Singijeon')
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()

for actor_label in ('BP_SingijeonFirePit_Playable', 'BP_SingijeonHwacha_Playable'):
    actor = next((item for item in actors if item.get_actor_label() == actor_label), None)
    unreal.log(f"SINGIJEON_VISUAL actor={actor_label}: {actor}")
    if not actor:
        continue
    unreal.log(f"SINGIJEON_VISUAL actor={actor_label}: location={actor.get_actor_location()}")
    for component in actor.get_components_by_class(unreal.NiagaraComponent):
        log_component(f'{actor_label}.Niagara', component)
    for component in actor.get_components_by_class(unreal.InstancedStaticMeshComponent):
        log_component(f'{actor_label}.ISM', component)
        unreal.log(f"SINGIJEON_VISUAL {actor_label}.ISM: mesh={component.get_editor_property('static_mesh')}")
        unreal.log(f"SINGIJEON_VISUAL {actor_label}.ISM: material0={component.get_material(0)}")

for blueprint_path in ('/GF_Singijeon/Gameplay/BP_SingijeonFirePit', '/GF_Singijeon/Gameplay/BP_SingijeonHwacha'):
    blueprint = unreal.load_asset(blueprint_path)
    unreal.log(f"SINGIJEON_VISUAL blueprint={blueprint_path}: {blueprint}")
    if not blueprint:
        continue
    cdo = unreal.get_default_object(blueprint.generated_class())
    for component in cdo.get_components_by_class(unreal.NiagaraComponent):
        log_component(f'{blueprint_path}.CDO.Niagara', component)
    for component in cdo.get_components_by_class(unreal.InstancedStaticMeshComponent):
        log_component(f'{blueprint_path}.CDO.ISM', component)
        unreal.log(f"SINGIJEON_VISUAL {blueprint_path}.CDO.ISM: mesh={component.get_editor_property('static_mesh')}")
        unreal.log(f"SINGIJEON_VISUAL {blueprint_path}.CDO.ISM: material0={component.get_material(0)}")

unreal.log('SINGIJEON_VISUAL_INSPECTION COMPLETE')
