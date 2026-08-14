import unreal


unreal.EditorLoadingAndSavingUtils.load_map("/GF_Singijeon/Maps/LV_Singijeon")
for actor in unreal.EditorLevelLibrary.get_all_level_actors():
    unreal.log(
        f"SINGIJEON_LEVEL_ACTOR label={actor.get_actor_label()} "
        f"class={actor.get_class().get_name()} location={actor.get_actor_location()}"
    )
