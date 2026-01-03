#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
直接检查FBX二进制文件的动画数据
"""
import sys
import os

# 设置输出编码为UTF-8
if sys.platform == 'win32':
    import io
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

# 尝试导入FBX SDK（如果有）
try:
    import FbxCommon
    has_fbx_sdk = True
except:
    has_fbx_sdk = False
    print("警告: FBX SDK未安装，尝试使用其他方法...")

# 使用Blender的Python API读取FBX
try:
    import bpy
    has_blender = True
except:
    has_blender = False

if not has_fbx_sdk and not has_blender:
    print("\n无法读取FBX文件：")
    print("- 需要安装FBX SDK或Blender Python API")
    print("\n建议：直接在Unity中导出带重采样的动画，或使用Blender重新导出FBX")
    sys.exit(1)

# 读取FBX文件
fbx_path = r"D:\github\GameWorld\godot-game-world\assets\simple_forest_animals\models\bear.fbx"

print("="*60)
print("FBX原始动画数据分析")
print("="*60)
print(f"文件: {fbx_path}")
print()

# TODO: 实现FBX读取逻辑
