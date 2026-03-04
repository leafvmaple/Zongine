# Zongine 开发路线图

本文档整理了 Zongine 引擎当前需要重构优化的部分，以及建议补充的功能模块，按优先级分为 P0（核心）、P1（重要）、P2（加分项）三档。

---

## 一、已完成的重构项 ✅

| # | 重构内容 | 状态 |
|---|---------|------|
| 1 | 修复 `Mananger.h` 拼写错误 → `Manager.h`，更新全部引用 | ✅ |
| 2 | 修复 `#include` 路径大小写不一致（`components/` → `Components/`） | ✅ |
| 3 | 修复消息循环：`if (PeekMessage)` → `while (PeekMessage)` 防止输入延迟 | ✅ |
| 4 | 高精度计时器：`timeGetTime` → `QueryPerformanceCounter`，统一所有 System::Tick 为 `float` 秒 | ✅ |
| 5 | 清理注释掉的死代码（Engine.cpp、AssetManager.cpp、DeviceManager.cpp、EntityManager.h） | ✅ |
| 6 | 单例重构：Meyers' Singleton → 指针注册式，Engine 持有所有 Manager 的 `unique_ptr`，生命周期可控 | ✅ |
| 7 | ComponentStorage 从 `unordered_map<EntityID, T>` 重构为 `vector<pair<EntityID, T>>` + 稀疏索引，缓存友好 | ✅ |

---

## 二、仍需重构的部分

### P0 — 核心架构问题

#### 2.1 资源管理缺乏 RAII 封装

**现状**：`DeviceManager` 中 D3D11 资源（`ID3D11Device*`、`ID3D11DeviceContext*`、`IDXGISwapChain*`）使用裸指针，依赖手动 `Release()`。泄露风险高。

**建议**：
- [ ] 引入 `ComPtr<T>`（`wrl/client.h`）包装所有 COM 对象
- [ ] SwapChain、RenderTargetView、DepthStencilView 等全部迁移到 `ComPtr`
- [ ] 移除手动 `Release()` 调用

#### 2.2 RenderGraph 缺少自动资源生命周期管理

**现状**：RenderGraph 实现了基于拓扑排序的 Pass 执行顺序，但渲染资源（RT、DS、SRV）的创建/销毁仍然手动管理，未与 Pass 的读写声明联动。

**建议**：
- [ ] 为 RenderPass 增加 `DeclareRead()` / `DeclareWrite()` 资源声明接口
- [ ] RenderGraph 编译时自动推导资源生命周期，结合引用计数自动创建/释放瞬态资源
- [ ] 添加资源别名（Aliasing）支持，减少显存占用

#### 2.3 ECS 缺少多组件联合查询

**现状**：`ForEach<T>` 只支持单组件遍历，系统中需要多组件时手动调用 `HasComponent` + `GetComponent` 逐一检查。

**建议**：
- [ ] 实现 `View<A, B, C...>` 多组件联合视图，自动取交集
- [ ] 支持 `Exclude<T>` 排除某组件的实体
- [ ] 提供 `Each(func)` 接口直接以 lambda 遍历：`view.Each([](EntityID, TransformComponent&, MeshComponent&) { ... })`

---

### P1 — 重要改进

#### 2.4 场景管理

**现状**：当前只有一个全局 Entity 树（根节点 ID=0），没有场景概念。无法分离关卡、切换场景、做场景序列化/反序列化。

**建议**：
- [ ] 抽象 `Scene` 类，持有独立的 Entity 列表和 ComponentStorage
- [ ] 实现场景序列化（JSON / 二进制）与加载
- [ ] 支持场景切换和预加载

#### 2.5 日志系统

**现状**：调试信息通过 `OutputDebugString` 或直接 `printf`，无分级、无文件输出、无可过滤的 Tag。

**建议**：
- [ ] 实现轻量级日志系统（`LOG_INFO` / `LOG_WARN` / `LOG_ERROR`）
- [ ] 支持输出到控制台、文件、VS Output
- [ ] 添加 Tag/Category 过滤
- [ ] 编辑器中集成日志面板

#### 2.6 内存分配优化

**现状**：Component 存储已迁移到连续 `vector`，但频繁的 `emplace_back` 会触发重分配。Entity 存储仍使用 `unordered_map`。

**建议**：
- [ ] ComponentStorage 预留容量（`reserve`），按需增长策略
- [ ] 考虑 Entity 存储使用 SlotMap 替代 `unordered_map`，稳定引用 + 连续内存
- [ ] 对热路径（Tick 循环中的组件访问）做性能 profiling

---

### P2 — 代码质量

#### 2.7 统一命名规范

**现状**：混合使用匈牙利命名（`m_nWidth`）、驼峰（`renderSystem`）、下划线前缀（`_UpdateWind`）等多种风格。

**建议**：
- [ ] 制定并文档化命名规范（推荐：成员变量 `m_CamelCase`，私有方法 `CamelCase`，局部变量 `camelCase`）
- [ ] 逐步统一现有代码

#### 2.8 头文件依赖清理

**现状**：部分头文件包含了不必要的完整头文件（如 `Entity.h` 包含了完整的组件头文件），增加编译时间。

**建议**：
- [ ] 头文件中尽量使用前向声明，在 `.cpp` 中 `#include`
- [ ] 避免在头文件中 `#include` 重量级第三方头文件（如 `json.hpp`、`tiny_gltf.h`）

---

## 三、建议补充的功能模块

### P0 — 核心能力展示

#### 3.1 阴影系统

**价值**：阴影是实时渲染最基本也最直观的技术点，缺少阴影场景表现力大打折扣。

**实现路线**：
- [ ] 实现 Shadow Map 基础管线
  - 额外的 Depth-Only 渲染 Pass（ShadowPass）
  - 光源视角的 ViewProjection 矩阵计算
  - 主渲染 Pass 中采样 Shadow Map 做遮挡判断
- [ ] PCF (Percentage Closer Filtering) 软阴影
- [ ] CSM (Cascaded Shadow Maps) 支持大场景的方向光阴影
- [ ] 集成到现有 RenderGraph 中作为新的 Pass 节点

#### 3.2 PBR 材质系统（基础版）

**价值**：基于物理的渲染是现代引擎标配，能极大提升画面质量和专业度。

**实现路线**：
- [ ] 实现标准 Metallic-Roughness 工作流
  - Albedo、Normal、Metallic、Roughness、AO 纹理槽位
  - Cook-Torrance BRDF（GGX 分布 + Smith 几何 + Schlick 菲涅尔）
- [ ] 基于图像的光照 (IBL)
  - 漫反射辐照度图
  - 预过滤镜面反射环境图 + BRDF LUT
- [ ] GLTF 模型材质自动映射到 PBR 参数

> 详细路线图见 [TODO.md](TODO.md) 材质系统章节

#### 3.3 延迟渲染 / Tile-Based 渲染

**价值**：展示对渲染管线架构的深入理解，有效支撑多光源场景。

**实现路线**：
- [ ] 实现 G-Buffer Pass（输出 Position、Normal、Albedo、Metallic/Roughness 到多 RT）
- [ ] 实现 Lighting Pass（全屏 Quad，在像素着色器中累加光照）
- [ ] 支持点光源、聚光灯、方向光多类型
- [ ] （可选）Tile-Based / Clustered 分桶剔除优化大量光源

---

### P1 — 重要加分项

#### 3.4 后处理管线

**价值**：后处理效果是渲染技术深度的体现。

**实现路线**：
- [ ] 在 RenderGraph 中添加 PostProcess Pass 节点
- [ ] Tone Mapping（HDR → LDR，ACES / Reinhard）
- [ ] Bloom（亮部提取 + 高斯模糊 + 合成）
- [ ] SSAO（屏幕空间环境光遮蔽）
- [ ] FXAA / TAA 抗锯齿
- [ ] 景深 (DoF)、运动模糊 (Motion Blur)（可选）

#### 3.5 GPU 性能分析与调试工具

**价值**：展示工程素养和性能意识。

**实现路线**：
- [ ] 实现基于 D3D11 Query 的 GPU 计时（每个 RenderPass 的耗时统计）
- [ ] 编辑器中显示帧时间分解图（CPU / GPU 各 Pass 耗时条形图）
- [ ] Draw Call / Triangle / Batch 等统计数据的 HUD 覆盖层
- [ ] 支持 RenderDoc 的程序化帧抓取集成

#### 3.6 骨骼动画 IK

**价值**：已有状态机 + 骨骼动画基础，添加 IK 可展示动画系统的完整度。

**实现路线**：
- [ ] 实现 Two-Bone IK（适用于手臂、腿部）
- [ ] 实现 CCD IK 或 FABRIK（适用于脊柱、尾巴等多骨骼链）
- [ ] Foot IK：角色脚部贴地面（射线检测 + IK 修正）
- [ ] Look At IK：头部/眼睛注视目标

#### 3.7 场景剔除

**价值**：展示空间数据结构和性能优化意识。

**实现路线**：
- [ ] 视锥剔除（Frustum Culling）— 基于 AABB
- [ ] 空间划分结构（BVH 或八叉树）
- [ ] 遮挡查询（Occlusion Query）粗粒度剔除

---

### P2 — 加分锦上添花

#### 3.8 粒子系统

**价值**：展示 GPU Compute 能力和视觉效果多样性。

**实现路线**：
- [ ] GPU 粒子系统（Compute Shader 更新 + DrawInstancedIndirect 渲染）
- [ ] 支持发射器类型：点、球、锥体
- [ ] 支持随时间变化的颜色/大小曲线
- [ ] 简单的力场（重力、风、涡流）

#### 3.9 地形系统

**价值**：展示大规模网格管理和 LOD 技术。

**实现路线**：
- [ ] 高度图加载与网格生成
- [ ] 基于距离的 LOD（Geo Clipmap 或 Quadtree）
- [ ] 多层纹理混合（Splat Map）
- [ ] 法线贴图自动生成

#### 3.10 Compute Shader 示例

**价值**：展示对 GPU 通用计算的理解。

**实现路线**：
- [ ] GPU 粒子模拟（与 3.8 结合）
- [ ] 或：简单 SPH 流体模拟
- [ ] 或：GPU Skinning（骨骼蒙皮计算从 CPU 转移到 CS）

---

## 四、推荐实施顺序

```
阶段         内容                      预计耗时
─────────────────────────────────────────────────
Phase 1      阴影系统 (3.1)              1-2 周
Phase 2      PBR 基础版 (3.2)            2-3 周
Phase 3      后处理管线 (3.4)            1-2 周
Phase 4      延迟渲染 (3.3)              2-3 周
Phase 5      场景剔除 (3.7)              1 周
Phase 6      GPU Profiler (3.5)          1 周
Phase 7      骨骼 IK (3.6)              1-2 周
Phase 8      粒子/地形/CS (3.8-3.10)    按兴趣选做
─────────────────────────────────────────────────
架构重构项穿插在功能开发中逐步完成
```

> **建议**：每完成一个功能模块，同步更新 README 中的 Feature 列表和截图/GIF，保持项目展示页面的完整性。
