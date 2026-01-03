#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Unity Prefab 到 Godot 场景转换器
将 Unity 的 Simple Forest Animals prefab 转换为 Godot .tscn 文件
"""

import os
import re
import yaml
import sys
from pathlib import Path

# 设置输出编码为UTF-8
if sys.platform == 'win32':
    import io
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

# Unity prefab 到 Godot 动物名称映射
ANIMAL_MAPPING = {
    "Bear": {"fbx": "bear.fbx", "mesh_prefix": "SM_Bear_", "scale": 10.0},
    "Boar": {"fbx": "boar.fbx", "mesh_prefix": "SM_Boar_", "scale": 10.0},
    "Fox": {"fbx": "fox.fbx", "mesh_prefix": "SM_Fox_", "scale": 10.0},
    "Rabbit": {"fbx": "rabbit.fbx", "mesh_prefix": "SM_Rabbit_", "scale": 10.0},
    "Doe": {"fbx": "deer.fbx", "mesh_prefix": "SM_Doe_", "scale": 10.0},
    "Stag": {"fbx": "deer.fbx", "mesh_prefix": "SM_Stag_", "scale": 10.0},
    "Moose_Female": {"fbx": "deer.fbx", "mesh_prefix": "SM_Moose_Female_", "scale": 10.0},
    "Moose_Male": {"fbx": "deer.fbx", "mesh_prefix": "SM_Moose_Male_", "scale": 10.0},
    "Raccoon": {"fbx": "fox.fbx", "mesh_prefix": "SM_Raccoon_", "scale": 7.0},
    "Skunk": {"fbx": "fox.fbx", "mesh_prefix": "SM_Skunk_", "scale": 5.0},
    "Wolf": {"fbx": "fox.fbx", "mesh_prefix": "SM_Wolf_", "scale": 13.0},
}

def parse_unity_prefab(prefab_path):
    """解析 Unity prefab 文件"""
    with open(prefab_path, 'r', encoding='utf-8') as f:
        content = f.read()

    # 使用 PyYAML 解析（Unity YAML 格式）
    documents = list(yaml.safe_load_all(content))

    prefab_data = {
        'name': '',
        'scale': [1, 1, 1],
        'meshes': [],
        'has_animator': False
    }

    # 查找根 GameObject
    for doc in documents:
        if doc and 'GameObject' in doc:
            if 'm_Father' in doc.get('Transform', {}) or 'm_Father' in doc.get('RectTransform', {}):
                father_ref = doc.get('Transform', {}).get('m_Father') or doc.get('RectTransform', {}).get('m_Father')
                if father_ref and father_ref.get('fileID') == 0:
                    # 这是根节点
                    prefab_data['name'] = doc['GameObject'].get('m_Name', '')
                    transform = doc.get('Transform', {})
                    if 'm_LocalScale' in transform:
                        scale = transform['m_LocalScale']
                        prefab_data['scale'] = [scale['x'], scale['y'], scale['z']]

        # 查找 Animator
        if doc and 'Animator' in doc:
            prefab_data['has_animator'] = True

        # 查找 SkinnedMeshRenderer (mesh 变体)
        if doc and 'SkinnedMeshRenderer' in doc:
            # 找到对应的 GameObject
            gameobject_id = doc['SkinnedMeshRenderer'].get('m_GameObject', {}).get('fileID')
            for doc2 in documents:
                if doc2 and 'GameObject' in doc2:
                    if str(doc2.get('GameObject', {}).get('m_ObjectHideFlags', '')) == str(gameobject_id):
                        mesh_name = doc2['GameObject'].get('m_Name', '')
                        is_active = doc2['GameObject'].get('m_IsActive', 1) == 1
                        prefab_data['meshes'].append({
                            'name': mesh_name,
                            'active': is_active
                        })
                        break

    return prefab_data

def extract_animal_type_and_variant(prefab_name):
    """从 prefab 名称提取动物类型和变体号
    例如: Bear_01 -> (Bear, 01)
    """
    match = re.match(r'(.+?)_(\d+)', prefab_name)
    if match:
        return match.group(1), match.group(2)
    return prefab_name, "01"

def convert_prefab_to_godot(prefab_path, output_dir):
    """将单个 Unity prefab 转换为 Godot 场景"""
    prefab_name = Path(prefab_path).stem
    animal_type, variant_num = extract_animal_type_and_variant(prefab_name)

    print(f"转换 {prefab_name}...")

    # 解析 Unity prefab
    try:
        prefab_data = parse_unity_prefab(prefab_path)
    except Exception as e:
        print(f"  ❌ 解析失败: {e}")
        return

    # 查找对应的 Godot FBX 配置
    if animal_type not in ANIMAL_MAPPING:
        print(f"  ⚠️ 未找到映射: {animal_type}")
        return

    config = ANIMAL_MAPPING[animal_type]
    fbx_path = f"res://assets/simple_forest_animals/models/{config['fbx']}"
    mesh_prefix = config['mesh_prefix']
    scale = config['scale']

    # 生成 Godot 场景文件内容
    scene_content = generate_godot_scene(
        prefab_name,
        fbx_path,
        mesh_prefix,
        variant_num,
        scale,
        prefab_data
    )

    # 保存场景文件
    output_path = os.path.join(output_dir, f"{prefab_name.lower()}.tscn")
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(scene_content)

    print(f"  ✓ 已生成: {output_path}")

def generate_godot_scene(name, fbx_path, mesh_prefix, variant_num, scale, prefab_data):
    """生成 Godot .tscn 文件内容"""
    # 这里简化处理，实际应该使用完整的 Godot 场景生成
    # 由于生成完整的 .tscn 需要很多细节，建议使用 create_animal_scenes.gd

    content = f"""[gd_scene load_steps=2 format=3]

[ext_resource type="PackedScene" path="{fbx_path}" id="1"]

[node name="{name}" type="Node3D"]
transform = Transform3D({scale}, 0, 0, 0, {scale}, 0, 0, 0, {scale}, 0, 0, 0)

[node name="Skeleton" type="Skeleton3D" parent="."]

"""

    # 添加 mesh 变体
    for i in range(1, 4):
        mesh_name = f"{mesh_prefix}{i:02d}"
        is_visible = (i == int(variant_num))

        content += f"""[node name="Variant{i:02d}" type="MeshInstance3D" parent="."]
visible = {str(is_visible).lower()}
skeleton = NodePath("../Skeleton")

"""

    return content

def main():
    prefabs_dir = r"D:\github\UnityGameWorld\Assets\SimpleForestAnimal\Prefabs"
    output_dir = r"D:\github\GameWorld\godot-game-world\scenes\animals\from_unity"

    # 创建输出目录
    os.makedirs(output_dir, exist_ok=True)

    print("=" * 60)
    print("Unity Prefab 到 Godot 转换器")
    print("=" * 60)
    print()

    # 转换所有 prefab
    prefab_files = sorted(Path(prefabs_dir).glob("*.prefab"))

    for prefab_path in prefab_files:
        convert_prefab_to_godot(str(prefab_path), output_dir)

    print()
    print("=" * 60)
    print(f"✅ 转换完成！共处理 {len(prefab_files)} 个 prefab")
    print("=" * 60)
    print()
    print("注意: 由于 Godot 场景格式复杂，建议:")
    print("1. 检查生成的场景文件")
    print("2. 或者继续使用 create_animal_scenes.gd 生成完整场景")

if __name__ == "__main__":
    main()
