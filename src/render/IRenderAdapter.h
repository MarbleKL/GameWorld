#pragma once

#include "geometry/MeshData.h"
#include "core/Types.h"

// ============================================================
// 渲染适配器接口
// 将引擎无关的几何数据上传到具体渲染后端
// ============================================================

namespace render {

class IRenderAdapter {
public:
    virtual ~IRenderAdapter() = default;
    
    // 创建Avatar并上传到渲染后端
    // 返回AvatarId句柄，用于后续引用
    virtual AvatarId CreateAvatar(const AvatarBundle& bundle) = 0;
    
    // 销毁Avatar（释放渲染资源）
    virtual void DestroyAvatar(AvatarId avatar_id) = 0;
    
    // 更新Avatar（未来扩展：LOD切换、动画等）
    // virtual void UpdateAvatar(AvatarId avatar_id, const AvatarBundle& bundle) = 0;
};

} // namespace render
