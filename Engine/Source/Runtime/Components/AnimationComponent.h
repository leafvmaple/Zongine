#pragma once

#include <DirectXMath.h>
#include <string>
#include <vector>

namespace Zongine {
    /**
     * 动画组件
     * 
     * 支持两种使用方式:
     * 1. 直接播放单一动画 (设置Path和nPlayTime)
     * 2. 通过AnimationStateMachineComponent使用状态机控制
     */
    struct AnimationComponent {
        std::string Path{};          // 当前播放的动画路径
        int nPlayTime{};             // 当前播放时间（毫秒）
        float Speed = 1.0f;          // 播放速度倍率
        bool Loop = true;            // 是否循环播放

        std::vector<DirectX::XMFLOAT4X4> ModelTransforms{};  // 骨骼模型空间变换矩阵
    };

    /**
     * 动画状态机组件
     * 
     * 类似 Unity 的 Animator 组件，存储状态机资源路径
     * 由 AssetManager 在初始化时加载状态机配置到实体
     */
    struct AnimStateMachineComponent {
        std::string StateMachineAssetPath{};  // 状态机 JSON 文件路径
        bool IsLoaded = false;                // 是否已加载
    };
}