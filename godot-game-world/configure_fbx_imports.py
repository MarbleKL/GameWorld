#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
自动配置所有 FBX 的 .import 文件，使用帧号分割动画
"""

import os
import sys
from pathlib import Path

# 设置输出编码为UTF-8
if sys.platform == 'win32':
    import io
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

# Unity 中定义的动画帧范围
ANIMAL_ANIMATIONS = {
    "bear": {
        "Idle": (1, 120),
        "Walk": (130, 170),
        "Run": (180, 204),
        "Eat": (210, 330)
    },
    "boar": {
        "Idle": (1, 120),
        "Walk": (130, 160),
        "Run": (170, 205),
        "Eat": (210, 330)
    },
    "deer": {
        "Run": (1, 24),
        "Walk": (30, 59),
        "Eat": (70, 160),
        "Idle": (160, 280)
    },
    "fox": {
        "Idle": (1, 120),
        "Walk": (130, 165),
        "Run": (180, 195),
        "Eat": (210, 330)
    },
    "rabbit": {
        "Run": (1, 28),
        "Walk": (40, 61),
        "Idle": (70, 150),
        "Eat": (170, 250)
    }
}

IMPORT_TEMPLATE = """[remap]

importer="scene"
importer_version=1
type="PackedScene"
uid="{uid}"
path="{path}"

[deps]

source_file="res://assets/simple_forest_animals/models/{fbx_name}.fbx"
dest_files=["{path}"]

[params]

nodes/root_type=""
nodes/root_name=""
nodes/root_script=null
nodes/apply_root_scale=true
nodes/root_scale=1.0
nodes/import_as_skeleton_bones=false
nodes/use_name_suffixes=true
nodes/use_node_type_suffixes=true
meshes/ensure_tangents=true
meshes/generate_lods=true
meshes/create_shadow_meshes=true
meshes/light_baking=1
meshes/lightmap_texel_size=0.2
meshes/force_disable_compression=false
skins/use_named_skins=true
animation/import=true
animation/fps=24
animation/trimming=true
animation/remove_immutable_tracks=true
animation/import_rest_as_RESET=false
import_script/path=""
materials/extract=0
materials/extract_format=0
materials/extract_path=""
_subresources={{
"animations": {{
"Take 001": {{
"save_to_file/enabled": false,
"save_to_file/fallback_path": "",
"save_to_file/keep_custom_tracks": "",
"save_to_file/path": "",
"settings/loop_mode": 0,
{slices}
"slices/amount": {slice_count}
}}
}}
}}
fbx/importer=0
fbx/allow_geometry_helper_nodes=false
fbx/embedded_image_handling=1
fbx/naming_version=2
"""

def read_existing_import(import_path):
    """读取现有的 import 文件，提取 uid 和 path"""
    uid = None
    path = None

    if os.path.exists(import_path):
        with open(import_path, 'r', encoding='utf-8') as f:
            for line in f:
                if line.startswith('uid='):
                    uid = line.strip().split('=', 1)[1].strip('"')
                elif line.startswith('path='):
                    path = line.strip().split('=', 1)[1].strip('"')

    return uid, path

def generate_slice_config(animations):
    """生成动画切片配置"""
    lines = []
    slice_num = 1

    # slice_1: 完整动画 (所有帧)
    lines.append('"slice_1/end_frame": 330,')
    lines.append('"slice_1/loop_mode": 0,')
    lines.append('"slice_1/name": "Take 001",')
    lines.append('"slice_1/save_to_file/enabled": false,')
    lines.append('"slice_1/save_to_file/fallback_path": "",')
    lines.append('"slice_1/save_to_file/keep_custom_tracks": false,')
    lines.append('"slice_1/save_to_file/path": "",')
    lines.append('"slice_1/start_frame": 1,')
    slice_num = 2

    # slice_2 到 slice_5: 具体的切片
    for name, (start, end) in animations.items():
        lines.append(f'"slice_{slice_num}/end_frame": {end},')
        lines.append(f'"slice_{slice_num}/loop_mode": 1,')
        lines.append(f'"slice_{slice_num}/name": "{name}",')
        lines.append(f'"slice_{slice_num}/save_to_file/enabled": false,')
        lines.append(f'"slice_{slice_num}/save_to_file/fallback_path": "",')
        lines.append(f'"slice_{slice_num}/save_to_file/keep_custom_tracks": false,')
        lines.append(f'"slice_{slice_num}/save_to_file/path": "",')
        lines.append(f'"slice_{slice_num}/start_frame": {start},')
        slice_num += 1

    return "\n".join(lines), len(animations) + 1  # +1 for Take 001

def configure_fbx_import(animal_name, animations, models_dir):
    """配置单个 FBX 的导入设置"""
    import_path = os.path.join(models_dir, f"{animal_name}.fbx.import")

    # 读取现有配置
    uid, path = read_existing_import(import_path)

    # 如果没有现有配置，使用默认值
    if not uid:
        uid = f"uid://default_{animal_name}"
    if not path:
        path = f"res://.godot/imported/{animal_name}.fbx-default.scn"

    # 生成切片配置
    slices, slice_count = generate_slice_config(animations)

    # 生成完整配置
    content = IMPORT_TEMPLATE.format(
        uid=uid,
        path=path,
        fbx_name=animal_name,
        slices=slices,
        slice_count=slice_count
    )

    # 写入文件
    with open(import_path, 'w', encoding='utf-8', newline='\n') as f:
        f.write(content)

    print(f"✓ 已配置 {animal_name}.fbx.import ({slice_count} 个动画切片)")

def main():
    models_dir = r"D:\github\GameWorld\godot-game-world\assets\simple_forest_animals\models"

    print("=" * 60)
    print("配置 FBX 导入文件...")
    print("=" * 60)
    print()

    for animal_name, animations in ANIMAL_ANIMATIONS.items():
        configure_fbx_import(animal_name, animations, models_dir)

    print()
    print("=" * 60)
    print("✅ 所有配置完成！")
    print("=" * 60)
    print()
    print("下一步:")
    print("1. 在 Godot 中重新导入所有 FBX 文件（右键 -> 重新导入）")
    print("2. 修改 create_animal_scenes.gd，直接使用分割好的动画")

if __name__ == "__main__":
    main()
