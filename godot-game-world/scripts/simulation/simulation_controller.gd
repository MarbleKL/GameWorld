extends Node

## SimulationController - 模拟控制器
## 负责驱动C++模拟核心并同步数据到Godot场景

var simulation = null  # SimulationWrapper实例

@onready var creature_manager = get_node_or_null("../CreatureManager")
@onready var player_camera = get_node_or_null("../PlayerCamera")

func _ready():
	# 创建C++模拟实例
	simulation = SimulationWrapper.new()

	if simulation == null:
		push_error("Failed to create SimulationWrapper! Check if GDExtension is loaded.")
		return

	# 初始化模拟世界
	simulation.initialize()
	print("✓ Simulation initialized at time: ", simulation.get_current_time())

	# 输出初始统计
	var stats = simulation.get_global_statistics()
	print("  Initial population - Rabbits: ", stats.rabbit_count,
		  ", Wolves: ", stats.wolf_count,
		  ", Bears: ", stats.bear_count)

func _process(delta):
	if simulation == null:
		return

	# 1. 更新相机位置到C++
	if player_camera:
		simulation.set_camera_position(player_camera.global_position)

	# 2. 驱动模拟更新 (模拟时间可以加速)
	var sim_delta = delta * 1.0  # 可调整加速倍率 (1.0 = 实时)
	simulation.update(sim_delta)

	# 3. 同步Creature数据到场景
	if creature_manager:
		_sync_creatures()

func _sync_creatures():
	# 获取所有Region状态
	var regions = simulation.get_all_regions()

	for region in regions:
		var region_id = region.id
		var mode = region.mode

		if mode == "HQ":
			# HQ模式：获取个体数据并更新CreatureManager
			var creatures = simulation.get_creatures_in_region(region_id)
			creature_manager.update_creatures(region_id, creatures)
		else:
			# LQ模式：清空该区域的个体
			creature_manager.clear_region(region_id)
