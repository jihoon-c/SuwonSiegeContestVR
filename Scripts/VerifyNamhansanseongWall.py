import unreal


ROOT = "/GF_Singijeon/Asset/NamhansanseongWall"
errors = []


def check(condition, message):
    if condition:
        unreal.log(f"NAMHANSANSEONG_WALL_VERIFY PASS: {message}")
    else:
        errors.append(message)
        unreal.log_error(f"NAMHANSANSEONG_WALL_VERIFY FAIL: {message}")


mesh = unreal.load_asset(ROOT + "/Meshes/SM_Stone_Barrier")
material_a = unreal.load_asset(ROOT + "/Materials/M_Stone_Barrier01a")
material_b = unreal.load_asset(ROOT + "/Materials/M_Stone_Barrier01b")
check(mesh is not None, "SM_Stone_Barrier loads")
check(material_a is not None, "M_Stone_Barrier01a loads")
check(material_b is not None, "M_Stone_Barrier01b loads")

for texture_set in ("a", "b"):
    for texture_kind in ("BC", "NM", "RN"):
        texture = unreal.load_asset(
            f"{ROOT}/Textures/T_Stone_Barrier01{texture_set}_{texture_kind}"
        )
        check(texture is not None, f"Texture 01{texture_set}_{texture_kind} loads")
        if texture and texture_kind != "BC":
            check(not texture.get_editor_property("srgb"),
                  f"Texture 01{texture_set}_{texture_kind} has sRGB disabled")

if mesh:
    static_materials = list(mesh.get_editor_property("static_materials"))
    check(len(static_materials) > 0, "Static Mesh has material slots")
    assigned = [item.get_editor_property("material_interface") for item in static_materials]
    check(all(item is not None for item in assigned), "Every mesh slot has a material")
    check(material_a in assigned, "Mesh uses material set A")
    if len(static_materials) > 1:
        check(material_b in assigned, "Mesh uses material set B")

if errors:
    raise RuntimeError("Namhansanseong wall verification failed: " + "; ".join(errors))
unreal.log("NAMHANSANSEONG_WALL_VERIFY SUCCESS")
