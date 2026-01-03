extends Node3D

## CreatureManager - 动物实例管理器
## 使用对象池管理动物实例的生成、更新和回收

const SPECIES_SCENES = {
	1: preload("res://scenes/animals/rabbit_01.tscn"),
	2: preload("res://scenes/animals/wolf_01.tscn"),
	3: preload("res://scenes/animals/bear_01.tscn")
}

const MAX_POOL_SIZE = 200  # 每个物种的对象池大小

# 对象池: {species_id: [instance1, instance2, ...]}
var inactive_pools: Dictionary = {}

# 活跃实例: {entity_id: {instance: Node3D, species_id: int, region_id: int}}
var active_creatures: Dictionary = {}

func _ready():
	print("CreatureManager: Initializing object pools...")

	# 预分配对象池
	for species_id in SPECIES_SCENES.keys():
		inactive_pools[species_id] = []

		for i in range(MAX_POOL_SIZE):
			var instance = SPECIES_SCENES[species_id].instantiate()
			instance.visible = false
			add_child(instance)
			inactive_pools[species_id].append(instance)

	print("  Created ", MAX_POOL_SIZE * SPECIES_SCENES.size(), " instances in pools")

func update_creatures(region_id: int, creatures: Array):
	# 构建当前Region应有的entity_id集合
	var current_ids = {}

	for creature in creatures:
		var entity_id = creature.entity_id
		current_ids[entity_id] = true

		# 如果是新Creature，从池中生成
		if not active_creatures.has(entity_id):
			_spawn_creature(creature)
		else:
			# 更新已有Creature的状态
			_update_creature_state(active_creatures[entity_id].instance, creature)

	# 清理该Region中已不存在的Creature
	var to_remove = []
	for entity_id in active_creatures.keys():
		var data = active_creatures[entity_id]

		# 只清理属于该Region的Creature
		if data.region_id == region_id and not current_ids.has(entity_id):
			to_remove.append(entity_id)

	for entity_id in to_remove:
		_despawn_creature(entity_id)

func clear_region(region_id: int):
	# LQ模式：回收该Region的所有Creature
	var to_remove = []

	for entity_id in active_creatures.keys():
		var data = active_creatures[entity_id]
		if data.region_id == region_id:
			to_remove.append(entity_id)

	for entity_id in to_remove:
		_despawn_creature(entity_id)

func _spawn_creature(data: Dictionary):
	var entity_id = data.entity_id
	var species_id = data.species_id
	var region_id = data.get("region_id", 1)

	if inactive_pools[species_id].is_empty():
		push_warning("Creature pool exhausted for species ", species_id)
		return

	# 从池中取出实例
	var instance = inactive_pools[species_id].pop_back()
	instance.visible = true
	instance.global_position = data.position

	# 应用基因外观 (size_scale)
	if data.has("gene"):
		var gene = data.gene
		var scale_factor = gene.get("size_scale", 1.0)
		instance.scale = Vector3.ONE * scale_factor

	# 记录活跃实例
	active_creatures[entity_id] = {
		"instance": instance,
		"species_id": species_id,
		"region_id": region_id
	}

func _despawn_creature(entity_id: int):
	if not active_creatures.has(entity_id):
		return

	var data = active_creatures[entity_id]
	var instance = data.instance
	var species_id = data.species_id

	# 重置状态
	instance.visible = false

	# 尝试重置动画 (如果有AnimalVariantController)
	if instance.has_method("stop_eating"):
		instance.stop_eating()
	if instance.has_method("set_movement_speed"):
		instance.set_movement_speed(0.0)

	# 回收到池中
	inactive_pools[species_id].append(instance)
	active_creatures.erase(entity_id)

func _update_creature_state(instance: Node3D, data: Dictionary):
	# 更新位置
	instance.global_position = data.position

	# 如果实例有AnimalVariantController，更新动画状态
	if not instance.has_method("start_eating"):
		return  # 不是AnimalVariantController

	var hunger = data.get("hunger", 0.5)
	var health = data.get("health", 1.0)

	# 饥饿度控制进食动画
	if hunger > 0.7:
		instance.start_eating()
	else:
		instance.stop_eating()

		# 健康度控制移动速度
		var speed = lerpf(0.2, 1.0, health)
		instance.set_movement_speed(speed)
