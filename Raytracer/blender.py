import json
import bpy
from mathutils import Vector

name = "monkey"

cam = bpy.context.scene.camera

# No idea how this works tbh
lookingAt = cam.matrix_world @ Vector((0, 0, -1))

def transformLocation(loc):
    y = loc.z
    z = -loc.y
    return {
        "x": loc.x, 
        "y": y,
        "z": z
    }

cam_dict = {
    "position": transformLocation(cam.location),
    # TODO
    "up": {
        "x": 0,
        "y": -1,
        "z": 0
    },
    "lookingAt": transformLocation(lookingAt),
    # Is this it??
    "fov": {
        "x": cam.data.angle_x,
        "y": cam.data.angle_y
    }
}

lights = []

for val in bpy.context.scene.objects:
    if val.type == 'LIGHT':
        light = {
            "type": val.data.type.lower(),
            "pos": transformLocation(val.location),
            "color": {
                "r": val.data.color.r,
                "g": val.data.color.g,
                "b": val.data.color.b
            },
            "power": val.data.energy
        }
        lights.append(light)
        
bg = bpy.context.scene.world.node_tree.nodes.get("Background")
color = list(bg.inputs[0].default_value)
strength = bg.inputs[1].default_value

lights.append({
    "type": "ambient",
    "color": {
        "r": color[0] * strength,
        "g": color[1] * strength,
        "b": color[2] * strength
    }
})

out = {
    "title": "Cone",
    "model": f"models/{name}.obj",
    "width": bpy.context.scene.render.resolution_x,
    "height": bpy.context.scene.render.resolution_y,
    "frames": [{
        "frame": 0,
        "camera": cam_dict
    }],
    "lights": lights,
    "output": "image.png"
}

with open("scene.json", "w") as f:
    json.dump(out, f)
    
bpy.ops.export_scene.obj(filepath=f"{name}.obj")