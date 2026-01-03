#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

#include "simulation_wrapper.h"

using namespace godot;

// ============================================================
// GDExtension 入口文件
// 注册 SimulationWrapper 类到 Godot
// ============================================================

void initialize_gameworld_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    // 注册 SimulationWrapper 类
    ClassDB::register_class<SimulationWrapper>();
}

void uninitialize_gameworld_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    // 清理工作 (如有需要)
}

extern "C" {

// GDExtension 初始化入口点
GDExtensionBool GDE_EXPORT gameworld_gdextension_init(
    GDExtensionInterfaceGetProcAddress p_get_proc_address,
    GDExtensionClassLibraryPtr p_library,
    GDExtensionInitialization *r_initialization)
{
    godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(initialize_gameworld_module);
    init_obj.register_terminator(uninitialize_gameworld_module);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}

}
