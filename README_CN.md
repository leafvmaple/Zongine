# Zongine

<div align="center">

**现代化的 DirectX 11 游戏引擎**

[![License](https://img.shields.io/badge/license-BSD--3--Clause-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)](https://www.microsoft.com/windows)
[![DirectX](https://img.shields.io/badge/DirectX-11-green.svg)](https://docs.microsoft.com/en-us/windows/win32/direct3d11/atoc-dx-graphics-direct3d-11)

[English](README.md) | 简体中文

</div>

## 📖 简介

Zongine 是一个基于 DirectX 11 开发的现代化游戏引擎，采用实体组件系统（ECS）架构设计。该引擎专注于提供高性能的渲染能力和灵活的编辑器工具，支持高级渲染技术和物理模拟。

### 核心特性

- 🎨 **现代渲染管线**
  - 基于 RenderGraph 的帧图渲染系统
  - 支持不透明渲染（Opaque）和顺序无关透明（OIT）渲染
  - 自动资源管理和依赖解析
  
- 🏗️ **ECS 架构**
  - 实体组件系统（Entity Component System）设计模式
  - 模块化系统架构（渲染、物理、动画、输入等）
  - 灵活的组件组合方式

- 🎮 **功能完整的编辑器**
  - 基于 Qt 6 的可视化编辑界面
  - 实时场景编辑和预览
  - 实体树视图和组件属性面板

- ⚡ **物理模拟**
  - 集成 NVIDIA Flex 物理引擎
  - 支持软体、流体等高级物理效果
  
- 🎬 **资产系统**
  - LLoader 资产加载库
  - 支持模型、纹理、材质、骨骼动画等多种资源格式
  - DirectXTex 纹理处理

## 🛠️ 技术栈

### 核心技术

- **图形 API**: DirectX 11
- **编程语言**: C++
- **UI 框架**: Qt 6
- **构建工具**: Visual Studio 2019+, CMake

### 第三方库

- **DirectXTex**: 纹理加载和处理
- **Effects11 (FX11)**: Shader 效果系统
- **NVIDIA Flex**: 高级物理模拟
- **Qt 6**: 编辑器 UI 框架

## 🏗️ 架构设计

### 引擎系统

```text
Engine (核心引擎类)
├── RenderSystem      (渲染系统)
│   └── RenderGraph   (帧图渲染管线)
├── PhysicsSystem     (物理系统)
├── NvFlexSystem      (Flex 物理系统)
├── AnimationSystem   (动画系统)
├── CameraSystem      (相机系统)
├── TransformSystem   (变换系统)
└── InputSystem       (输入系统)
```

### 渲染管线 (RenderGraph)

RenderGraph 是引擎的核心渲染系统，提供：

- 自动依赖管理和拓扑排序
- 灵活的 Pass 系统（ClearPass, OpaquePass, OITPass, CompositePass, PresentPass）
- 资源自动管理（RenderTarget, DepthStencil, ShaderResource）

详细使用指南请参考 [RenderGraph 指南](Docs/RenderGraph_Guide.md)

### 组件系统

引擎支持以下组件类型：

- `MeshComponent`: 网格渲染
- `MaterialComponent`: 材质属性
- `SkeletonComponent`: 骨骼动画
- `NvFlexComponent`: Flex 物理模拟
- `CameraComponent`: 相机
- `TransformComponent`: 变换矩阵
- `NameComponent`: 实体命名

## 🚀 快速开始

### 环境要求

- Windows 10/11
- Visual Studio 2019 或更高版本
- DirectX 11 兼容的显卡
- Qt 6.x SDK（用于编辑器）
- CMake 3.15+（可选）

### 构建步骤

1. **克隆仓库**

```powershell
git clone https://github.com/leafvmaple/Zongine.git
cd Zongine
```

2. **初始化子模块**（如果有）

```powershell
git submodule update --init --recursive
```

3. **打开解决方案**

```powershell
# 使用 Visual Studio 打开
start Zongine.sln
```

4. **配置 Qt 环境**
   - 确保安装了 Qt Visual Studio Tools 扩展
   - 配置 Qt 6 路径

5. **构建项目**
   - 选择 `Debug` 或 `Release` 配置
   - 构建 `Editor` 项目或 `Simple` 示例项目

### 运行编辑器

构建完成后，运行 `Editor.exe`：

```powershell
.\x64\Debug\Editor.exe
```

## 📁 项目结构

```text
Zongine/
├── Docs/                      # 文档
│   ├── Reference.md          # 参考文档
│   ├── RenderGraph_Guide.md  # RenderGraph 使用指南
│   └── TODO.md               # 开发路线图
│
├── Engine/                    # 引擎源代码
│   ├── 3rd/                  # 第三方库
│   │   ├── DirectXTex/       # 纹理处理库
│   │   ├── FX11/             # Effects11
│   │   └── NVFlex/           # NVIDIA Flex
│   │
│   ├── Include/              # 公共头文件
│   ├── Shader/               # Shader 源代码
│   │
│   └── Source/
│       ├── Editor/           # Qt 编辑器
│       ├── Runtime/          # 运行时引擎
│       │   ├── Components/   # 组件定义
│       │   ├── Systems/      # 系统实现
│       │   ├── RenderGraph/  # 渲染图
│       │   └── Entities/     # 实体管理
│       ├── LLoader/          # 资产加载器
│       ├── Maths/            # 数学库
│       └── Simple/           # 简单示例
│
├── Lib/                      # 编译库文件
├── Product/                  # 产品输出
│   └── data/                 # 资源数据
└── x64/                      # 构建输出
```

## 📚 文档

- [RenderGraph 使用指南](Docs/RenderGraph_Guide.md) - 详细的渲染管线使用文档
- [待办事项](Docs/TODO.md) - 功能开发和路线图

## 🎯 开发路线图

- [ ] 改进材质系统
- [ ] PBR 渲染支持
- [ ] 后处理效果系统
- [ ] 更多物理特效
- [ ] 场景序列化和加载
- [ ] 资源热重载

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

## 📄 许可证

本项目采用 [BSD 3-Clause License](LICENSE) 开源许可证。

Copyright (c) 2025, Zohar Lee

## 🙏 致谢

感谢以下开源项目：

- [DirectXTex](https://github.com/microsoft/DirectXTex)
- [Effects11](https://github.com/microsoft/FX11)
- [NVIDIA Flex](https://developer.nvidia.com/flex)
- [Qt](https://www.qt.io/)

## 📧 联系方式

- 作者: Zohar Lee (leafvmaple)
- 项目地址: [https://github.com/leafvmaple/Zongine](https://github.com/leafvmaple/Zongine)

---

<div align="center">
Made with ❤️ by Zohar Lee
</div>
