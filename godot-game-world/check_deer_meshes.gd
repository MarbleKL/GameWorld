@tool
extends EditorScript

func _run():
	print("\n=== 检查 Deer FBX 的 Mesh 节点 ===\n")

	var fbx = load("res://assets/simple_forest_animals/models/deer.fbx")
	if not fbx:
		print("❌ 无法加载 deer.fbx")
		return

	var root = fbx.instantiate()
	print_tree(root, 0)
	root.queue_free()

	print("\n=== 检查完成 ===\n")

func print_tree(node: Node, level: int):
	var indent = "  ".repeat(level)
	var node_info = "%s - %s" % [node.name, node.get_class()]

	if node is MeshInstance3D:
		node_info += " [MeshInstance3D]"

	print(indent + node_info)

	for child in node.get_children():
		print_tree(child, level + 1)
