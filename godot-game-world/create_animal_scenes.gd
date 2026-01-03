@tool
extends EditorScript

## 动物场景生成器 - 基于 Unity Prefab 一对一翻译
## 在Godot编辑器中运行: 文件 -> 运行 -> 选择此脚本
## 配置由 analyze_unity_prefabs.py 自动生成

# 从外部文件加载 prefab 配置
const prefab_configs_script = preload("res://prefab_configs.gd")

func _run():
	print("\n" + "=".repeat(60))
	print("开始生成动物场景（基于 Unity Prefab）...")
	print("=".repeat(60) + "\n")

	var PREFAB_CONFIGS = prefab_configs_script.PREFAB_CONFIGS

	for prefab_name in PREFAB_CONFIGS.keys():
		create_prefab_scene(prefab_name, PREFAB_CONFIGS[prefab_name])

	print("\n" + "=".repeat(60))
	print("✅ 所有场景生成完成！")
	print("=".repeat(60))

func create_prefab_scene(prefab_name: String, config: Dictionary):
	print("🔨 生成 %s.tscn..." % prefab_name.to_lower())

	# 加载FBX场景
	var fbx_scene = load(config.fbx)
	if not fbx_scene:
		print("  ❌ 无法加载FBX: %s" % config.fbx)
		return

	var fbx_root = fbx_scene.instantiate()

	# 创建场景根节点
	var root = Node3D.new()
	root.name = prefab_name
	root.scale = Vector3(config.scale, config.scale, config.scale)

	# 附加控制脚本
	var script = load("res://scripts/animals/animal_variant_controller.gd")
	if script:
		root.set_script(script)

	# 查找并复制Skeleton
	var source_skeleton = find_skeleton(fbx_root)
	if source_skeleton:
		var skeleton = source_skeleton.duplicate(Node.DUPLICATE_USE_INSTANTIATION | Node.DUPLICATE_SCRIPTS | Node.DUPLICATE_GROUPS)
		skeleton.name = "Skeleton"
		root.add_child(skeleton, true)
		skeleton.owner = root
		print("  ✓ 复制了Skeleton (%d骨骼)" % skeleton.get_bone_count())
	else:
		print("  ⚠️ 未找到Skeleton")
		fbx_root.queue_free()
		root.queue_free()
		return

	# 加载材质
	var material = load("res://assets/simple_forest_animals/materials/forest_animal.tres")

	# 只添加这个 prefab 中启用的mesh
	var active_mesh_name = config.active_mesh
	var source_mesh = find_mesh_by_name(fbx_root, active_mesh_name)

	if source_mesh:
		var mesh_instance = MeshInstance3D.new()
		mesh_instance.name = active_mesh_name
		mesh_instance.mesh = source_mesh.mesh
		mesh_instance.skeleton = NodePath("../Skeleton")
		mesh_instance.skin = source_mesh.skin
		mesh_instance.material_override = material
		mesh_instance.visible = true

		root.add_child(mesh_instance, true)
		mesh_instance.owner = root
		print("  ✓ 添加了 %s" % active_mesh_name)
	else:
		print("  ❌ 未找到mesh: %s" % active_mesh_name)
		fbx_root.queue_free()
		root.queue_free()
		return

	# 复制AnimationPlayer（直接使用Godot分割的动画）
	var source_anim_player = find_animation_player(fbx_root)
	if source_anim_player:
		var anim_player = AnimationPlayer.new()
		anim_player.name = "AnimationPlayer"
		anim_player.root_node = NodePath("..")

		# 获取所有动画库
		var lib_names = source_anim_player.get_animation_library_list()
		var anim_count = 0

		for lib_name in lib_names:
			var source_lib = source_anim_player.get_animation_library(lib_name)
			var new_lib = AnimationLibrary.new()

			# 遍历库中的所有动画
			var anim_names = source_lib.get_animation_list()
			for anim_name in anim_names:
				# 跳过 "Take 001" 完整动画，只使用切片
				if anim_name == "Take 001":
					continue

				var source_anim = source_lib.get_animation(anim_name)

				# 创建新动画并复制轨道
				var new_anim = Animation.new()
				new_anim.length = source_anim.length
				new_anim.loop_mode = Animation.LOOP_LINEAR

				# 复制所有轨道
				for track_idx in range(source_anim.get_track_count()):
					var track_path = source_anim.track_get_path(track_idx)
					var track_type = source_anim.track_get_type(track_idx)

					# 修复路径：从 "CowRig/...Skeleton:bone" -> "Skeleton:bone"
					var path_string = str(track_path)
					if ":" in path_string:
						var parts = path_string.split(":", false)
						if parts.size() >= 2:
							track_path = NodePath("Skeleton:" + parts[-1])

					# 创建并复制轨道
					var new_track_idx = new_anim.add_track(track_type)
					new_anim.track_set_path(new_track_idx, track_path)
					new_anim.track_set_interpolation_type(new_track_idx, source_anim.track_get_interpolation_type(track_idx))

					# 复制所有关键帧
					var key_count = source_anim.track_get_key_count(track_idx)
					for key_idx in range(key_count):
						var time = source_anim.track_get_key_time(track_idx, key_idx)
						var value = source_anim.track_get_key_value(track_idx, key_idx)
						new_anim.track_insert_key(new_track_idx, time, value)

				# 添加到新的动画库
				new_lib.add_animation(anim_name, new_anim)
				anim_count += 1

			# 添加动画库到AnimationPlayer
			anim_player.add_animation_library(lib_name, new_lib)

		root.add_child(anim_player, true)
		anim_player.owner = root

		print("  ✓ 复制了AnimationPlayer (%d个动画)" % anim_count)
	else:
		print("  ⚠️ 未找到AnimationPlayer")

	# 创建AnimationTree
	var anim_tree = create_animation_tree()
	if anim_tree:
		root.add_child(anim_tree, true)
		anim_tree.owner = root
		print("  ✓ 创建了AnimationTree")

	# 保存场景
	var packed_scene = PackedScene.new()
	var result = packed_scene.pack(root)

	if result == OK:
		var scene_path = "res://scenes/animals/%s.tscn" % prefab_name.to_lower()
		var save_result = ResourceSaver.save(packed_scene, scene_path)
		if save_result == OK:
			print("  ✅ 保存成功: %s\n" % scene_path)
		else:
			print("  ❌ 保存失败 (错误码: %d, 路径: %s)\n" % [save_result, scene_path])
	else:
		print("  ❌ 打包场景失败 (错误码: %d)\n" % result)

	# 清理
	fbx_root.queue_free()
	root.queue_free()

func create_animation_tree() -> AnimationTree:
	var tree = AnimationTree.new()
	tree.name = "AnimationTree"
	tree.anim_player = NodePath("../AnimationPlayer")

	# 创建StateMachine
	var state_machine = AnimationNodeStateMachine.new()

	# 创建BlendSpace1D用于移动
	var blend_space = AnimationNodeBlendSpace1D.new()
	blend_space.min_space = 0.0
	blend_space.max_space = 1.0
	blend_space.sync = true

	# 添加移动动画
	var idle = AnimationNodeAnimation.new()
	idle.animation = "Idle"
	blend_space.add_blend_point(idle, 0.0)

	var walk = AnimationNodeAnimation.new()
	walk.animation = "Walk"
	blend_space.add_blend_point(walk, 0.5)

	var run = AnimationNodeAnimation.new()
	run.animation = "Run"
	blend_space.add_blend_point(run, 1.0)

	# 创建Eat动画
	var eat = AnimationNodeAnimation.new()
	eat.animation = "Eat"

	# 添加到StateMachine
	state_machine.add_node("Locomotion", blend_space, Vector2(200, 100))
	state_machine.add_node("Eat", eat, Vector2(400, 100))

	# 添加转换
	var trans_to_eat = AnimationNodeStateMachineTransition.new()
	state_machine.add_transition("Locomotion", "Eat", trans_to_eat)

	var trans_to_loco = AnimationNodeStateMachineTransition.new()
	state_machine.add_transition("Eat", "Locomotion", trans_to_loco)

	tree.tree_root = state_machine
	tree.active = true

	return tree

# 辅助函数
func find_skeleton(node: Node) -> Skeleton3D:
	if node is Skeleton3D:
		return node
	for child in node.get_children():
		var result = find_skeleton(child)
		if result:
			return result
	return null

func find_mesh_by_name(node: Node, target_name: String) -> MeshInstance3D:
	if node.name == target_name and node is MeshInstance3D:
		return node
	for child in node.get_children():
		var result = find_mesh_by_name(child, target_name)
		if result:
			return result
	return null

func find_animation_player(node: Node) -> AnimationPlayer:
	if node is AnimationPlayer:
		return node
	for child in node.get_children():
		var result = find_animation_player(child)
		if result:
			return result
	return null
