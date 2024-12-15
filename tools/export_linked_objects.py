import bpy
import sys
import os
import mathutils
import math
import struct

base_transform = mathutils.Matrix.Scale(64, 4) @ mathutils.Matrix.Rotation(-math.pi * 0.5, 4, 'X')
mesh_coordinate_convert = mathutils.Matrix.Rotation(math.pi * 0.5, 4, 'X') @ mathutils.Matrix.Rotation(math.pi, 4, 'Z')

asset_list = [
    "blockade",
    "directionsign",
    "streetlight",
    "trafficlight",
    "van",
]

to_write = []

for obj in bpy.data.objects:
    if obj.type != "MESH" or obj.name == "fast64_f3d_material_library_PlaneObject":
        continue

    mesh: bpy.types.Mesh = obj.data

    if not mesh.library:
        continue

    transform = base_transform @ obj.matrix_world @ mesh_coordinate_convert

    to_write.append([asset_list.index(mesh.library.filepath[2:-6]), transform])

def get_obj_x(obj):
    return obj[1].to_translation()[0]

to_write.sort(key=get_obj_x)

with open(sys.argv[-1], 'wb') as file:
    file.write(len(to_write).to_bytes(2, 'big'))

    for obj in to_write:
        file.write(obj[0].to_bytes(1, 'big'))

        loc, rot, scale = obj[1].decompose()

        file.write(struct.pack(">hhh", int(loc[0]), int(loc[1]), int(loc[2])))

        if rot[3] < 0:
            rot.negate()

        file.write(struct.pack(">hhh", int(rot[0] * 32000), int(rot[1] * 32000), int(rot[2] * 32000)))