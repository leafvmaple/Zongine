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
}