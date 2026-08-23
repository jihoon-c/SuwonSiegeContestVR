import unreal
path="/GF_Singijeon/Gameplay/Props/BP_SingijeonArrowBox_Instanced"
if unreal.EditorAssetLibrary.does_asset_exist(path): unreal.EditorAssetLibrary.delete_asset(path)
factory=unreal.BlueprintFactory(); factory.set_editor_property("parent_class", unreal.load_class(None,"/Script/GF_Singijeon.SingijeonArrowBoxActor"))
bp=unreal.AssetToolsHelpers.get_asset_tools().create_asset("BP_SingijeonArrowBox_Instanced","/GF_Singijeon/Gameplay/Props",unreal.Blueprint,factory)
unreal.BlueprintEditorLibrary.compile_blueprint(bp); unreal.EditorAssetLibrary.save_loaded_asset(bp,False)
unreal.log("SINGIJEON_ARROW_BOX_ACTOR_BP SUCCESS")
