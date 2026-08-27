"""Applies the safe impact FX and recompiles the edited Ongseong Blueprints.

Run in Unreal Editor after compiling the C++ module:
    py D:/Github/SuwonSiegeContestVR/Scripts/ApplyOngseongRamVisualAndImpactFix.py

The script is idempotent.  It updates BP_ChongtongProjectile's explicit override so an
old saved Blueprint default cannot reintroduce the sample post-process explosion.
"""

import unreal

PROJECTILE_BP = "/GF_OngseongCrossbow/Blueprints/BP_ChongtongProjectile"
RAM_BP = "/GF_OngseongCrossbow/Blueprints/BP_OngseongRam"
SAFE_IMPACT = "/Game/NiagaraExamples/FX_Weapons/Impacts/NS_Impact_Concrete"


def require(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if not asset:
        raise RuntimeError("Missing asset: " + path)
    return asset


def compile_and_save(blueprint):
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(blueprint)


projectile = require(PROJECTILE_BP)
impact = require(SAFE_IMPACT)
projectile_cdo = unreal.get_default_object(projectile.generated_class())
projectile_cdo.set_editor_property("explosion_effect", impact)
projectile_cdo.set_editor_property("explosion_effect_scale", unreal.Vector(1.0, 1.0, 1.0))
compile_and_save(projectile)

# Recompile so the new VisualRoot and inherited red VisibilityHighlight component are serialized
# into BP_OngseongRam. Its transform and component tree remain freely editable in the Blueprint.
compile_and_save(require(RAM_BP))

unreal.log("[Ongseong] Applied safe local impact FX and ram visual-component update.")
