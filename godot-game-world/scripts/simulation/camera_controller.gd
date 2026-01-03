extends Camera3D

## CameraController - 玩家相机控制器
## WASD移动，鼠标右键旋转，Space/Ctrl升降

@export var move_speed: float = 50.0
@export var mouse_sensitivity: float = 0.1

var rotation_x: float = -45.0
var rotation_y: float = 0.0
var mouse_captured: bool = false

func _ready():
	# 初始位置：Region 2/5 边界上空
	global_position = Vector3(150, 80, 50)
	rotation_degrees = Vector3(rotation_x, rotation_y, 0)

	print("Camera initialized at position: ", global_position)

func _input(event):
	# 右键按下/释放控制鼠标捕获
	if event is InputEventMouseButton:
		if event.button_index == MOUSE_BUTTON_RIGHT:
			if event.pressed:
				mouse_captured = true
				Input.mouse_mode = Input.MOUSE_MODE_CAPTURED
			else:
				mouse_captured = false
				Input.mouse_mode = Input.MOUSE_MODE_VISIBLE

	# 鼠标移动控制旋转
	if event is InputEventMouseMotion and mouse_captured:
		rotation_y -= event.relative.x * mouse_sensitivity
		rotation_x = clamp(rotation_x - event.relative.y * mouse_sensitivity, -89, 89)
		rotation_degrees = Vector3(rotation_x, rotation_y, 0)

func _process(delta):
	# WASD移动
	var input_dir = Vector3.ZERO

	if Input.is_key_pressed(KEY_W):
		input_dir -= transform.basis.z
	if Input.is_key_pressed(KEY_S):
		input_dir += transform.basis.z
	if Input.is_key_pressed(KEY_A):
		input_dir -= transform.basis.x
	if Input.is_key_pressed(KEY_D):
		input_dir += transform.basis.x

	# 上下移动
	if Input.is_key_pressed(KEY_SPACE):
		input_dir.y += 1
	if Input.is_key_pressed(KEY_CTRL):
		input_dir.y -= 1

	# 加速键
	var speed_multiplier = 1.0
	if Input.is_key_pressed(KEY_SHIFT):
		speed_multiplier = 3.0

	if input_dir.length() > 0:
		global_position += input_dir.normalized() * move_speed * speed_multiplier * delta
