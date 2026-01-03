@tool
extends EditorScript

func _run():
	print("\n=== 调试 Bear FBX 导入 ===\n")

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
		print("\n库: %s" % display_name)

		var lib = anim_player.get_animation_library(lib_name)
		var anim_names = lib.get_animation_list()

		print("  动画数量: %d" % anim_names.size())

		for anim_name in anim_names:
			var anim = lib.get_animation(anim_name)
			print("\n  动画: %s" % anim_name)
			print("    长度: %.2fs (%.0f 帧 @ 24fps)" % [anim.length, anim.length * 24])
			print("    循环模式: %d" % anim.loop_mode)
			print("    轨道数: %d" % anim.get_track_count())

			# 检查第一个轨道的关键帧范围
			if anim.get_track_count() > 0:
				var first_key_time = anim.track_get_key_time(0, 0)
				var last_key_time = anim.track_get_key_time(0, anim.track_get_key_count(0) - 1)
				print("    第一个关键帧时间: %.2fs (帧 %.0f)" % [first_key_time, first_key_time * 24])
				print("    最后关键帧时间: %.2fs (帧 %.0f)" % [last_key_time, last_key_time * 24])

	root.queue_free()
	print("\n=== 调试完成 ===\n")

func find_animation_player(node: Node) -> AnimationPlayer:
	if node is AnimationPlayer:
		return node
	for child in node.get_children():
		var result = find_animation_player(child)
		if result:
			return result
	return null
