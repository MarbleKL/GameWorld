@tool
extends EditorScript

func _run():
	print("\n=== 检查 Bear FBX 导入的动画 ===\n")

	var fbx = load("res://assets/simple_forest_animals/models/bear.fbx")
	if not fbx:
		print("❌ 无法加载 bear.fbx")
		return

	var root = fbx.instantiate()
	var anim_player = find_animation_player(root)

	if not anim_player:
		print("❌ 未找到 AnimationPlayer")
		root.queue_free()
		return

	print("AnimationPlayer 找到！")
	print("\n所有动画库:")
	var lib_names = anim_player.get_animation_library_list()
	for lib_name in lib_names:
		var display_name = lib_name if lib_name != "" else "[默认库]"
		print("  库: %s" % display_name)

		var lib = anim_player.get_animation_library(lib_name)
		var anim_names = lib.get_animation_list()

		print("    动画数量: %d" % anim_names.size())
		for anim_name in anim_names:
			var anim = lib.get_animation(anim_name)
			print("      - %s (长度: %.2fs, 轨道数: %d)" % [anim_name, anim.length, anim.get_track_count()])

	root.queue_free()
	print("\n=== 检查完成 ===\n")

func find_animation_player(node: Node) -> AnimationPlayer:
	if node is AnimationPlayer:
		return node
	for child in node.get_children():
		var result = find_animation_player(child)
		if result:
			return result
	return null
