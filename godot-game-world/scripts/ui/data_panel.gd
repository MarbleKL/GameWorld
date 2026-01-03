extends Control

## DataPanel - HUD数据面板
## 显示实时模拟统计数据

@onready var time_label = $VBoxContainer/TimeLabel
@onready var region_label = $VBoxContainer/RegionLabel
@onready var rabbit_label = $VBoxContainer/RabbitLabel
@onready var wolf_label = $VBoxContainer/WolfLabel
@onready var bear_label = $VBoxContainer/BearLabel
@onready var total_label = $VBoxContainer/TotalLabel

var simulation = null
var player_camera = null

func _ready():
	# 获取SimulationController的simulation实例
	var sim_controller = get_node_or_null("/root/MainSimulation/SimulationController")

	if sim_controller and sim_controller.simulation:
		simulation = sim_controller.simulation
		print("DataPanel connected to simulation")
	else:
		push_warning("DataPanel: SimulationController not found!")

	# 获取玩家相机
	player_camera = get_node_or_null("/root/MainSimulation/PlayerCamera")

func _process(_delta):
	if simulation == null:
		return

	# 更新时间
	var current_time = simulation.get_current_time()
	time_label.text = "Day: %.1f" % current_time

	# 显示当前所在Region
	if player_camera:
		var current_region_id = simulation.get_current_region(player_camera.global_position)
		var region_data = simulation.get_region(current_region_id)
		var region_name = region_data.get("name", "Unknown")
		var region_mode = region_data.get("mode", "LQ")
		region_label.text = "Region: %s (ID:%d) [%s]" % [region_name, current_region_id, region_mode]

	# 获取全局统计
	var stats = simulation.get_global_statistics()

	rabbit_label.text = "Rabbits: %d" % stats.get("rabbit_count", 0)
	wolf_label.text = "Wolves: %d" % stats.get("wolf_count", 0)
	bear_label.text = "Bears: %d" % stats.get("bear_count", 0)
	total_label.text = "Total: %d" % stats.get("total_count", 0)
