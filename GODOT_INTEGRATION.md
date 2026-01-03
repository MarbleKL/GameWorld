# GameWorld - Godot 3D 可视化集成

## 概述

这个项目将C++种群模拟系统（ECS架构）通过GDExtension集成到Godot 4.5，实现实时3D可视化。

## 核心特性

### HQ/LQ模式切换
- **玩家走到哪个区块，哪个区块就变成HQ模式**（显示个体动物）
- 其他区块自动变成LQ模式（种群统计）
- 6个Region（Forest, Plains, Hills, Swamp, River, Mountain）
- 3个物种（Rabbit, Wolf, Bear）

### 可视化内容
- 个体动物实例（使用对象池管理，最多600个）
- 基因参数影响外观（size_scale控制缩放）
- 动画状态映射（hunger → eating, health → speed）
- 实时HUD数据面板
- 彩色地形（6个100x100单位的平面）

## 编译步骤

### 1. 配置和编译GDExtension

```bash
# 配置CMake（首次运行会下载godot-cpp，约50MB）
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 编译GDExtension
cmake --build build --target gameworld_gdextension --config Release
```

编译成功后，应在以下位置看到dll：
```
godot-game-world/addons/gameworld_simulation/bin/gameworld_gdextension.dll
```

### 2. 在Godot中创建主场景

打开Godot 4.5编辑器，创建新场景 `scenes/main_simulation.tscn`：

```
MainSimulation (Node3D)
├─ WorldEnvironment
│  └─ Environment (新建Environment资源)
├─ DirectionalLight3D
│  └─ rotation_degrees: (-45, 0, 0)
├─ TerrainManager (Node3D)
│  └─ [附加脚本: res://scripts/simulation/terrain_manager.gd]
├─ CreatureManager (Node3D)
│  └─ [附加脚本: res://scripts/simulation/creature_manager.gd]
├─ SimulationController (Node)
│  └─ [附加脚本: res://scripts/simulation/simulation_controller.gd]
├─ PlayerCamera (Camera3D)
│  └─ [附加脚本: res://scripts/simulation/camera_controller.gd]
└─ UI (CanvasLayer)
   └─ DataPanel (Control)
      ├─ VBoxContainer
      │  ├─ TimeLabel (Label) - text: "Day: 0.0"
      │  ├─ RegionLabel (Label) - text: "Region: Forest"
      │  ├─ RabbitLabel (Label) - text: "Rabbits: 0"
      │  ├─ WolfLabel (Label) - text: "Wolves: 0"
      │  ├─ BearLabel (Label) - text: "Bears: 0"
      │  └─ TotalLabel (Label) - text: "Total: 0"
      └─ [附加脚本: res://scripts/ui/data_panel.gd]
```

**UI布局建议**：
- DataPanel → Layout: Top Left
- VBoxContainer → Theme Overrides/Constants/Separation: 5

## 操作说明

### 相机控制
- **WASD**: 前后左右移动
- **Space**: 上升
- **Ctrl**: 下降
- **Shift**: 加速（3倍速度）
- **鼠标右键拖动**: 旋转视角

### HQ/LQ切换
- 走进任意Region的100x100范围内，该区域自动切换到HQ模式
- 离开后自动变回LQ模式
- HUD会显示当前所在的Region名称和模式

## 项目结构

```
GameWorld/
├── godot-extension/              # C++ GDExtension绑定层
│   ├── coordinate_mapper.h/cpp   # 坐标映射（6个Region布局）
│   ├── simulation_wrapper.h/cpp  # 核心API封装
│   └── gdextension.cpp           # GDExtension入口
├── godot-game-world/
│   ├── addons/
│   │   └── gameworld_simulation/
│   │       ├── gameworld_simulation.gdextension  # 插件配置
│   │       └── bin/                              # 编译输出
│   ├── scripts/
│   │   ├── simulation/
│   │   │   ├── simulation_controller.gd  # 模拟驱动器
│   │   │   ├── creature_manager.gd       # 对象池管理
│   │   │   ├── camera_controller.gd      # 相机控制
│   │   │   └── terrain_manager.gd        # 地形生成
│   │   └── ui/
│   │       └── data_panel.gd             # HUD面板
│   └── scenes/
│       ├── animals/                      # 动物场景（rabbit/wolf/bear）
│       └── main_simulation.tscn          # 主场景
└── src/                                  # 原C++模拟核心（不变）
```

## API文档

### SimulationWrapper (C++ → GDScript)

```gdscript
# 初始化模拟世界
simulation.initialize()

# 每帧更新（delta为模拟时间步长）
simulation.update(delta)

# 设置相机位置（自动触发HQ/LQ切换）
simulation.set_camera_position(Vector3(x, y, z))

# 获取当前相机所在的Region ID
var region_id = simulation.get_current_region(camera_position)

# 获取模拟时间（天数）
var time = simulation.get_current_time()

# 获取所有Region状态（返回Array<Dictionary>）
var regions = simulation.get_all_regions()
# regions[0] = {id: 1, name: "Forest", mode: "HQ", food_capacity: 1000, ...}

# 获取单个Region状态
var region = simulation.get_region(1)

# 获取指定Region的所有Creature（HQ模式）
var creatures = simulation.get_creatures_in_region(1)
# creatures[0] = {entity_id: 123, species_id: 1, position: Vector3(...),
#                 age: 12.5, hunger: 0.3, health: 0.9, gene: {...}}

# 获取指定Region的种群统计（LQ模式）
var populations = simulation.get_populations_in_region(1)

# 获取全局统计
var stats = simulation.get_global_statistics()
# stats = {rabbit_count: 1200, wolf_count: 150, bear_count: 60, total_count: 1410}
```

## Region布局（俯视图）

```
┌─────────┬─────────┬─────────┐
│ Region1 │ Region2 │ Region3 │
│ Forest  │ Plains  │ Hills   │
│  (0,0)  │(100,0)  │(200,0)  │
│  绿色   │  黄色   │  棕色   │
├─────────┼─────────┼─────────┤
│ Region4 │ Region5 │ Region6 │
│ Swamp   │ River   │Mountain │
│ (0,100) │(100,100)│(200,100)│
│ 深绿色  │  蓝色   │  灰色   │
└─────────┴─────────┴─────────┘
```

## 技术特点

1. **零序列化开销**: GDScript直接访问C++内存，无需JSON/WebSocket
2. **对象池优化**: 预分配600个动物实例，避免运行时创建/销毁
3. **区块化HQ/LQ**: 玩家位置自动触发区域级别切换
4. **零侵入设计**: 原C++模拟核心代码无需修改
5. **实时同步**: 模拟和渲染统一帧率，Godot驱动

## 故障排除

### 编译问题
- **CMake找不到godot-cpp**: 确保网络连接正常，首次编译会自动下载
- **编译错误**: 确保使用C++20标准，MSVC或GCC均可

### Godot问题
- **SimulationWrapper未定义**: 确保dll已编译并放在正确路径
- **场景运行无动物**: 检查控制台是否有"Simulation initialized"
- **相机移动无反应**: 确保PlayerCamera脚本已附加

### 性能问题
- **帧率低**: 减少MAX_POOL_SIZE（creature_manager.gd第8行）
- **卡顿**: 降低模拟时间倍率（simulation_controller.gd第25行）

## 下一步增强

- [ ] 添加Wolf和Bear的完整支持（当前仅Rabbit完全测试）
- [ ] 实现body_mass → 胖瘦映射
- [ ] HQ/LQ转换淡入淡出动画
- [ ] Region食物资源可视化（进度条）
- [ ] 实时种群曲线图（使用GraphEdit）
- [ ] 死亡粒子效果
- [ ] 区域天气系统（森林有雾等）

## 许可证

MIT License
