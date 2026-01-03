#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
分析Unity FBX的动画配置，为Godot生成动画分割配置
"""

import os
import re
import sys
from pathlib import Path

# 设置输出编码为UTF-8
if sys.platform == 'win32':
    import io
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

UNITY_MODELS_DIR = r"D:\github\UnityGameWorld\Assets\SimpleForestAnimal\Models"

def parse_fbx_meta(meta_path):
    """解析Unity的.meta文件，提取动画片段信息"""
    with open(meta_path, 'r', encoding='utf-8') as f:
        content = f.read()

    animations = []

    # 查找clipAnimations部分
    # 格式类似:
    # clipAnimations:
    # - serializedVersion: 16
    #   name: Bear_Idle
    #   takeName: Take 001
    #   firstFrame: 0
    #   lastFrame: 120

    clip_pattern = r'- serializedVersion:.*?name: (\w+).*?firstFrame: ([\d.]+).*?lastFrame: ([\d.]+)'
    matches = re.finditer(clip_pattern, content, re.DOTALL)

    for match in matches:
        name = match.group(1)
        first_frame = float(match.group(2))
        last_frame = float(match.group(3))

        animations.append({
            'name': name,
            'start_frame': first_frame,
            'end_frame': last_frame,
            'start_time': first_frame / 30.0,  # 假设30fps
            'end_time': last_frame / 30.0
        })

    return animations

def generate_godot_import_config(fbx_name, animations):
    """生成Godot的动画分割配置"""
    if not animations:
        print(f"  Warning: {fbx_name} - No animations found in Unity meta")
        return

    print(f"\n{fbx_name}:")
    print("  Animations found in Unity:")
    for anim in animations:
        duration = anim['end_time'] - anim['start_time']
        print(f"    - {anim['name']}: frames {anim['start_frame']}-{anim['end_frame']} ({duration:.2f}s)")

    print(f"\n  Godot import config (_subresources section):")
    print("  _subresources={")

    for anim in animations:
        print(f'    "animation/{anim["name"]}": {{')
        print(f'      "save_to_file/enabled": true,')
        print(f'      "save_to_file/path": "res://assets/simple_forest_animals/animations/{fbx_name.lower()}_{anim["name"].lower()}.res",')
        print(f'      "slice_{anim["start_frame"]}_{anim["end_frame"]}/save_to_file/enabled": true,')
        print(f'      "slice_{anim["start_frame"]}_{anim["end_frame"]}/save_to_file/path": "res://assets/simple_forest_animals/animations/{fbx_name.lower()}_{anim["name"].lower()}.res"')
        print('    },')

    print("  }")

def main():
    print("="*60)
    print("Unity FBX动画配置分析 (用于Godot)")
    print("="*60)

    # 查找所有FBX的meta文件
    meta_files = list(Path(UNITY_MODELS_DIR).glob("*.fbx.meta"))

    if not meta_files:
        print(f"Error: No .fbx.meta files found in {UNITY_MODELS_DIR}")
        return

    for meta_file in sorted(meta_files):
        fbx_name = meta_file.stem.replace('.fbx', '')
        animations = parse_fbx_meta(meta_file)

        if animations:
            generate_godot_import_config(fbx_name, animations)

    print("\n" + "="*60)
    print("注意事项:")
    print("="*60)
    print("1. Godot需要手动在导入面板中配置动画分割")
    print("2. 或者修改.import文件的_subresources部分")
    print("3. 推荐方案: 使用AnimationLibrary在代码中手动分割")

if __name__ == "__main__":
    main()
