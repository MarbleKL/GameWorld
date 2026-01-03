extends Node3D

## TerrainManager - 地形生成器
## 自动创建6个Region的地形平面

const REGION_SIZE = 100.0

const REGIONS = {
	1: {"name": "Forest", "color": Color(0.2, 0.6, 0.2), "center": Vector3(0, 0, 0)},
	2: {"name": "Plains", "color": Color(0.8, 0.8, 0.4), "center": Vector3(100, 0, 0)},
	3: {"name": "Hills", "color": Color(0.6, 0.5, 0.3), "center": Vector3(200, 0, 0)},
	4: {"name": "Swamp", "color": Color(0.3, 0.4, 0.3), "center": Vector3(0, 0, 100)},
	5: {"name": "River", "color": Color(0.3, 0.5, 0.8), "center": Vector3(100, 0, 100)},
	6: {"name": "Mountain", "color": Color(0.5, 0.5, 0.5), "center": Vector3(200, 0, 100)}
}

func _ready():
	print("TerrainManager: Generating region terrains...")

	for region_id in REGIONS.keys():
		var config = REGIONS[region_id]
		_create_region_terrain(region_id, config)

	print("  Created ", REGIONS.size(), " region terrains")

func _create_region_terrain(region_id: int, config: Dictionary):
	var mesh_instance = MeshInstance3D.new()
	mesh_instance.name = "Region%d_%s" % [region_id, config.name]

	# 创建平面网格
	var plane = PlaneMesh.new()
	plane.size = Vector2(REGION_SIZE, REGION_SIZE)
	plane.subdivide_width = 20
	plane.subdivide_depth = 20
	mesh_instance.mesh = plane

	# 设置材质
	var material = StandardMaterial3D.new()
	material.albedo_color = config.color
	material.roughness = 0.8
	mesh_instance.set_surface_override_material(0, material)

	# 设置位置
	mesh_instance.global_position = config.center

	add_child(mesh_instance)
