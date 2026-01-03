@tool
extends Node3D
class_name AnimalVariantController

## Controls animal mesh variant visibility and animation parameters
## Supports 3+ mesh variants and AnimationTree parameter control

@export_group("Variant Selection")
@export_range(1, 12) var active_variant: int = 1:
	set(value):
		active_variant = value
		_update_variant_visibility()

@export_group("Animation Parameters")
@export_range(0.0, 1.0) var speed: float = 0.0:
	set(value):
		speed = clamp(value, 0.0, 1.0)
		_update_blend_position(speed)

@export var is_eating: bool = false:
	set(value):
		is_eating = value
		_update_state(is_eating)

## Internal node references
var variant_nodes: Array[Node3D] = []
var animation_tree: AnimationTree = null
var skeleton: Skeleton3D = null

func _ready():
	# Find all variant mesh nodes and skeleton
	_discover_variant_nodes()

	# Find AnimationTree
	animation_tree = _find_child_of_type(self, AnimationTree)

	# Find Skeleton3D
	skeleton = _find_child_of_type(self, Skeleton3D)

	# Initialize
	_update_variant_visibility()
	if animation_tree:
		animation_tree.active = true

func _discover_variant_nodes():
	"""Auto-discover all mesh variant nodes (MeshInstance3D children)"""
	variant_nodes.clear()
	for child in get_children():
		if child is MeshInstance3D:
			variant_nodes.append(child)

func _find_child_of_type(node: Node, type) -> Node:
	"""Recursively find first child of given type"""
	for child in node.get_children():
		if is_instance_of(child, type):
			return child
		var result = _find_child_of_type(child, type)
		if result:
			return result
	return null

func _update_variant_visibility():
	"""Show only the active variant, hide all others"""
	if variant_nodes.is_empty():
		_discover_variant_nodes()

	for i in range(variant_nodes.size()):
		if variant_nodes[i] != null:
			variant_nodes[i].visible = (i + 1 == active_variant)

func _update_blend_position(blend_value: float):
	"""Update BlendSpace1D blend position for locomotion speed"""
	if animation_tree and animation_tree.active:
		# Godot 4.x: BlendSpace1D parameter path
		animation_tree.set("parameters/Locomotion/blend_position", blend_value)

func _update_state(eating: bool):
	"""Switch between Locomotion and Eat states"""
	if animation_tree and animation_tree.active:
		var playback = animation_tree.get("parameters/playback")
		if playback:
			if eating:
				playback.travel("Eat")
			else:
				playback.travel("Locomotion")

## Public API for external control

func set_variant(variant_index: int):
	"""Set active mesh variant (1-based index)"""
	active_variant = clamp(variant_index, 1, variant_nodes.size())

func set_movement_speed(movement_speed: float):
	"""Set movement speed (0.0=idle, 0.5=walk, 1.0=run)"""
	speed = movement_speed

func start_eating():
	"""Transition to eating animation"""
	is_eating = true

func stop_eating():
	"""Return to locomotion animations"""
	is_eating = false

func get_variant_count() -> int:
	"""Return total number of available variants"""
	if variant_nodes.is_empty():
		_discover_variant_nodes()
	return variant_nodes.size()
