# RenderGraph 使用指南

## 概述
RenderGraph 是 Zongine 引擎中用于管理渲染流程的框架，基于现代渲染引擎的帧图（Frame Graph）概念实现。

## 特性

### 自动依赖管理
RenderGraph 会自动分析各个 Pass 之间的资源依赖关系，并使用拓扑排序确定最优的执行顺序。

### 资源管理
- 自动创建和管理渲染资源（RenderTarget、DepthStencil等）
- 支持导入外部资源
- 自动生成所需的视图（RTV、DSV、SRV、UAV）

### 灵活的Pass系统
- 基于继承的Pass设计，易于扩展
- 支持动态启用/禁用Pass
- 清晰的Setup和Execute阶段分离

## 快速开始

### 1. 基本使用流程

```cpp
// 创建 RenderGraph
auto renderGraph = std::make_unique<RenderGraph>();

// 导入外部资源
renderGraph->ImportRenderTarget("BackBuffer", swapChainRTV);
renderGraph->ImportDepthStencil("DepthStencil", depthStencilView);

// 创建内部资源
ResourceDesc colorDesc;
colorDesc.Name = "ColorBuffer";
colorDesc.Type = ResourceType::RenderTarget;
colorDesc.Width = 1920;
colorDesc.Height = 1080;
colorDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
renderGraph->CreateResource(colorDesc);

// 添加Pass
auto clearPass = renderGraph->AddPass<ClearPass>("Clear", "ColorBuffer");
auto renderPass = renderGraph->AddPass<OpaquePass>("Opaque", renderSystem);

// 编译并执行
renderGraph->Compile();
renderGraph->Execute(context);
```

### 2. 创建自定义Pass

```cpp
// MyCustomPass.h
class MyCustomPass : public RenderPass {
public:
    MyCustomPass(/* 构造参数 */);
    
    void Setup(RenderGraph& graph) override;
    void Execute(ComPtr<ID3D11DeviceContext> context, RenderGraph& graph) override;

private:
    // 成员变量
};

// MyCustomPass.cpp
void MyCustomPass::Setup(RenderGraph& graph) {
    // 声明输入资源
    AddInput("InputTexture");
    
    // 声明输出资源
    AddOutput("OutputRT");
}

void MyCustomPass::Execute(ComPtr<ID3D11DeviceContext> context, RenderGraph& graph) {
    // 获取资源
    auto outputRTV = graph.GetRenderTarget("OutputRT");
    auto inputSRV = graph.GetShaderResource("InputTexture");
    
    // 设置渲染状态
    context->OMSetRenderTargets(1, outputRTV.GetAddressOf(), nullptr);
    context->PSSetShaderResources(0, 1, inputSRV.GetAddressOf());
    
    // 执行渲染操作
    // ...
}
```

## 内置Pass详解

### ClearPass
用于清除 RenderTarget 和 DepthStencil。

```cpp
auto clearPass = renderGraph->AddPass<ClearPass>(
    "ClearPass",
    "TargetName",                          // 目标资源名称
    XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)      // 清除颜色
);
clearPass->SetClearDepth(true);            // 是否清除深度
```

### OpaquePass
渲染不透明物体。需要传入 RenderSystem 指针。

```cpp
auto opaquePass = renderGraph->AddPass<OpaquePass>("OpaquePass", renderSystem);
```

### OITPass
Order Independent Transparency - 顺序无关透明渲染。

```cpp
auto oitPass = renderGraph->AddPass<OITPass>("OITPass", renderSystem);
```

### CompositePass
合成多个渲染结果到最终输出。

```cpp
auto compositePass = renderGraph->AddPass<CompositePass>("CompositePass");
```

### PresentPass
呈现最终结果到交换链。

```cpp
auto presentPass = renderGraph->AddPass<PresentPass>("PresentPass");
```

## 完整示例：后处理效果

```cpp
class BloomPass : public RenderPass {
public:
    BloomPass(float threshold, float intensity) 
        : m_Threshold(threshold), m_Intensity(intensity) {}
    
    void Setup(RenderGraph& graph) override {
        AddInput("SceneColor");
        AddOutput("BloomResult");
    }
    
    void Execute(ComPtr<ID3D11DeviceContext> context, RenderGraph& graph) override {
        auto sceneColor = graph.GetShaderResource("SceneColor");
        auto bloomRTV = graph.GetRenderTarget("BloomResult");
        
        // 提取亮区
        // ...
        
        // 高斯模糊
        // ...
        
        // 合成
        // ...
    }

private:
    float m_Threshold;
    float m_Intensity;
};

// 使用
void SetupPostProcessing(RenderGraph& graph) {
    // 创建Bloom资源
    ResourceDesc bloomDesc;
    bloomDesc.Name = "BloomResult";
    bloomDesc.Type = ResourceType::RenderTarget;
    bloomDesc.Width = 1920;
    bloomDesc.Height = 1080;
    bloomDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    graph.CreateResource(bloomDesc);
    
    // 添加Bloom Pass
    auto bloomPass = graph.AddPass<BloomPass>("Bloom", 1.0f, 0.5f);
    
    // 可以动态控制
    bloomPass->SetEnabled(enableBloom);
}
```

## 最佳实践

### 1. 资源命名
使用清晰、描述性的资源名称：
- `SceneColor` - 场景颜色缓冲
- `DepthStencil` - 深度模板缓冲
- `GBuffer_Albedo` - G-Buffer 反照率
- `Shadow_Directional` - 方向光阴影图

### 2. Pass组织
将功能相关的Pass组织在一起：
```cpp
// Geometry Phase
auto gBufferPass = graph.AddPass<GBufferPass>("GBuffer");

// Lighting Phase
auto deferredLightingPass = graph.AddPass<DeferredLightingPass>("Lighting");

// Post-processing Phase
auto toneMappingPass = graph.AddPass<ToneMappingPass>("ToneMapping");
auto fxaaPass = graph.AddPass<FXAAPass>("FXAA");
```

### 3. 性能优化
- 使用适当的资源格式（避免过高精度）
- 合理设置资源尺寸
- 考虑使用瞬态资源（未来版本支持）

### 4. 调试技巧
- 为每个Pass设置有意义的名称
- 使用 `SetEnabled()` 动态关闭Pass进行调试
- 可以添加DebugPass来可视化中间结果

## 架构图

```
RenderGraph
├── Resources (资源管理)
│   ├── Imported Resources (外部导入)
│   └── Created Resources (内部创建)
├── Passes (渲染Pass)
│   ├── Setup Phase (依赖声明)
│   └── Execute Phase (执行渲染)
└── Compilation (编译优化)
    ├── Dependency Analysis (依赖分析)
    ├── Topological Sort (拓扑排序)
    └── Resource Allocation (资源分配)
```

## 技术细节

### 依赖解析
RenderGraph 使用 Kahn 算法进行拓扑排序：
1. 收集所有Pass的输入输出依赖
2. 计算每个Pass的入度
3. 从入度为0的Pass开始执行
4. 执行后更新相关Pass的入度
5. 重复直到所有Pass执行完成

### 资源视图管理
根据资源类型自动创建所需视图：
- RenderTarget → RTV + SRV
- DepthStencil → DSV + SRV (使用TYPELESS格式)
- Texture2D → SRV
- Buffer → SRV/UAV

## 常见问题

### Q: 如何在RenderGraph中访问RenderSystem的渲染队列？
A: 通过在Pass构造函数中传入RenderSystem指针，然后调用其公开的接口。

### Q: 可以动态修改RenderGraph吗？
A: 修改后需要重新调用 `Compile()` 方法。

### Q: 如何切换传统渲染和RenderGraph渲染？
A: 在RenderSystem中设置 `m_bUseRenderGraph` 标志。

### Q: 性能开销如何？
A: RenderGraph的编译开销主要在初始化时，运行时开销很小。拓扑排序是一次性的。

## 未来计划

- [ ] 资源别名（Resource Aliasing）
- [ ] 瞬态资源（Transient Resources）
- [ ] 异步计算Pass
- [ ] GPU时间戳查询
- [ ] 可视化调试工具
- [ ] 自动Mipmap生成
- [ ] 资源状态追踪（Vulkan/DX12风格）
