#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
分析 Unity Prefab 结构
提取每个 prefab 中实际启用的 mesh 信息，用于指导 Godot 场景生成
"""

import os
import re
import sys
from pathlib import Path

# 设置输出编码为UTF-8
if sys.platform == 'win32':
    import io
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

def parse_unity_prefab(prefab_path):
    """简单解析 Unity prefab，提取 GameObject 和 SkinnedMeshRenderer 信息"""
    with open(prefab_path, 'r', encoding='utf-8') as f:
        content = f.read()

    # 将文件分割成 YAML 文档
    documents = content.split('--- !u!')

    gameobjects = {}  # fileID -> GameObject 数据
    root_gameobject = None
    root_scale = 10.0  # 默认值

    for doc in documents:
        if not doc.strip():
            continue

        # 提取 GameObject
        if doc.startswith('1 &'):
            match_id = re.search(r'1 &(\d+)', doc)
            match_name = re.search(r'm_Name: (.+)', doc)
            match_active = re.search(r'm_IsActive: (\d)', doc)

            if match_id and match_name:
                file_id = match_id.group(1)
                name = match_name.group(1)
                is_active = match_active.group(1) == '1' if match_active else True

                gameobjects[file_id] = {
                    'name': name,
                    'active': is_active
                }

        # 提取根节点的 Transform 和 scale
        if doc.startswith('4 &'):
            match_father = re.search(r'm_Father: \{fileID: (\d+)\}', doc)
            if match_father and match_father.group(1) == '0':
                # 这是根节点的 Transform
                match_scale = re.search(r'm_LocalScale: \{x: ([\d.]+)', doc)
                if match_scale:
                    root_scale = float(match_scale.group(1))

                # 找到根节点的 GameObject
                match_gameobject = re.search(r'm_GameObject: \{fileID: (\d+)\}', doc)
                if match_gameobject:
                    root_gameobject = match_gameobject.group(1)

    # 提取实际启用的 mesh 名称
    active_mesh = None
    for file_id, go_data in gameobjects.items():
        if go_data['active'] and go_data['name'].startswith('SM_'):
            active_mesh = go_data['name']
            break  # 每个 prefab 应该只有一个 active mesh

    root_name = gameobjects.get(root_gameobject, {}).get('name', '')

    result = {
        'root_name': root_name,
        'scale': root_scale,
        'active_mesh': active_mesh
    }

    return result

def analyze_all_prefabs(prefabs_dir):
    """分析所有 prefab 文件"""
    prefab_files = sorted(Path(prefabs_dir).glob("*.prefab"))

    analysis_results = {}

    for prefab_path in prefab_files:
        prefab_name = prefab_path.stem
        try:
            data = parse_unity_prefab(str(prefab_path))
            analysis_results[prefab_name] = data
        except Exception as e:
            print(f"❌ 解析 {prefab_name} 失败: {e}")

    return analysis_results

def print_analysis_summary(results):
    """打印分析摘要"""
    print("\n" + "=" * 70)
    print("Unity Prefab 分析结果")
    print("=" * 70)
    print()

    # FBX 文件映射
    fbx_mapping = {
        "Bear": "bear.fbx",
        "Boar": "boar.fbx",
        "Fox": "fox.fbx",
        "Rabbit": "rabbit.fbx",
        "Doe": "deer.fbx",
        "Stag": "deer.fbx",
        "Moose_Female": "deer.fbx",
        "Moose_Male": "deer.fbx",
        "Raccoon": "fox.fbx",
        "Skunk": "fox.fbx",
        "Wolf": "fox.fbx",
    }

    # 按动物类型分组
    animals = {}
    for prefab_name, data in results.items():
        match = re.match(r'(.+?)_(\d+)', prefab_name)
        if match:
            animal_type = match.group(1)
            variant = match.group(2)
        else:
            animal_type = prefab_name
            variant = "01"

        if animal_type not in animals:
            animals[animal_type] = []

        animals[animal_type].append({
            'variant': variant,
            'data': data
        })

    # 打印每种动物
    for animal_type in sorted(animals.keys()):
        variants_list = sorted(animals[animal_type], key=lambda x: x['variant'])
        scale = variants_list[0]['data']['scale']
        fbx_file = fbx_mapping.get(animal_type, "unknown.fbx")

        print(f"【{animal_type}】")
        print(f"  FBX: {fbx_file}")
        print(f"  Scale: {scale}")
        print(f"  Prefabs: {len(variants_list)}个")

        for item in variants_list:
            variant_num = item['variant']
            active_mesh = item['data']['active_mesh'] or "无"
            prefab_name = f"{animal_type}_{variant_num}"
            print(f"    {prefab_name}: {active_mesh}")

        print()

def generate_godot_config(results, output_path):
    """生成 Godot 配置数据（GDScript 格式）并保存到文件"""

    # 按动物类型分组
    animals = {}
    for prefab_name, data in results.items():
        match = re.match(r'(.+?)_(\d+)', prefab_name)
        if match:
            animal_type = match.group(1)
            variant = match.group(2)
        else:
            animal_type = prefab_name
            variant = "01"

        if animal_type not in animals:
            animals[animal_type] = {}

        animals[animal_type][variant] = data

    # FBX 文件映射
    fbx_mapping = {
        "Bear": "bear.fbx",
        "Boar": "boar.fbx",
        "Fox": "fox.fbx",
        "Rabbit": "rabbit.fbx",
        "Doe": "deer.fbx",
        "Stag": "deer.fbx",
        "Moose_Female": "deer.fbx",
        "Moose_Male": "deer.fbx",
        "Raccoon": "fox.fbx",
        "Skunk": "fox.fbx",
        "Wolf": "fox.fbx",
    }

    lines = []
    lines.append("# Unity Prefab 配置（由 analyze_unity_prefabs.py 自动生成）")
    lines.append("# 每个 prefab 对应一个场景，只包含启用的 mesh")
    lines.append("")
    lines.append("const PREFAB_CONFIGS = {")

    for animal_type in sorted(animals.keys()):
        variants = animals[animal_type]
        fbx_file = fbx_mapping.get(animal_type, "unknown.fbx")

        for variant_num in sorted(variants.keys()):
            variant_data = variants[variant_num]
            prefab_name = f"{animal_type}_{variant_num}"
            active_mesh = variant_data['active_mesh']
            scale = variant_data['scale']

            if not active_mesh:
                continue

            lines.append(f'    "{prefab_name}": {{')
            lines.append(f'        "fbx": "res://assets/simple_forest_animals/models/{fbx_file}",')
            lines.append(f'        "scale": {scale},')
            lines.append(f'        "active_mesh": "{active_mesh}"')
            lines.append(f'    }},')

    lines.append("}")

    # 写入文件
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write('\n'.join(lines))

    print("\n" + "=" * 70)
    print(f"✅ Godot 配置已保存到: {output_path}")
    print("=" * 70)

def main():
    prefabs_dir = r"D:\github\UnityGameWorld\Assets\SimpleForestAnimal\Prefabs"
    output_path = r"D:\github\GameWorld\godot-game-world\prefab_configs.gd"

    print("正在分析 Unity Prefab 文件...")
    results = analyze_all_prefabs(prefabs_dir)

    print_analysis_summary(results)
    generate_godot_config(results, output_path)

    print()
    print("=" * 70)
    print("下一步:")
    print("1. 查看生成的 prefab_configs.gd")
    print("2. 修改 create_animal_scenes.gd 使用这个配置")
    print("=" * 70)

if __name__ == "__main__":
    main()
