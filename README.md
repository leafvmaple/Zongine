# Zongine

<div align="center">

**A Modern DirectX 11 Game Engine**

[![License](https://img.shields.io/badge/license-BSD--3--Clause-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)](https://www.microsoft.com/windows)
[![DirectX](https://img.shields.io/badge/DirectX-11-green.svg)](https://docs.microsoft.com/en-us/windows/win32/direct3d11/atoc-dx-graphics-direct3d-11)

English | [简体中文](README_CN.md)

</div>

## 📖 Overview

Zongine is a modern game engine built on DirectX 11, designed with an Entity Component System (ECS) architecture. The engine focuses on providing high-performance rendering capabilities and flexible editor tools, supporting advanced rendering techniques and physics simulation.

### Key Features

- 🎨 **Modern Rendering Pipeline**
  - Frame graph rendering system based on RenderGraph
  - Support for Opaque and Order-Independent Transparency (OIT) rendering
  - Automatic resource management and dependency resolution
  
- 🏗️ **ECS Architecture**
  - Entity Component System design pattern
  - Modular system architecture (Rendering, Physics, Animation, Input, etc.)
  - Flexible component composition

- 🎮 **Full-Featured Editor**
  - Visual editing interface based on Qt 6
  - Real-time scene editing and preview
  - Entity tree view and component property panel

- ⚡ **Physics Simulation**
  - Integrated NVIDIA Flex physics engine
  - Support for soft bodies, fluids, and other advanced physics effects
  
- 🎬 **Asset System**
  - LLoader asset loading library
  - Support for models, textures, materials, skeletal animations, and more
  - DirectXTex texture processing

## 🛠️ Tech Stack

### Core Technologies

- **Graphics API**: DirectX 11
- **Programming Language**: C++
- **UI Framework**: Qt 6
- **Build Tools**: Visual Studio 2019+, CMake

### Third-Party Libraries

- **DirectXTex**: Texture loading and processing
- **Effects11 (FX11)**: Shader effects system
- **NVIDIA Flex**: Advanced physics simulation
- **Qt 6**: Editor UI framework

## 🏗️ Architecture

### Engine Systems

```text
Engine (Core Engine Class)
├── RenderSystem      (Rendering System)
│   └── RenderGraph   (Frame Graph Rendering Pipeline)
├── PhysicsSystem     (Physics System)
├── NvFlexSystem      (Flex Physics System)
├── AnimationSystem   (Animation System)
├── CameraSystem      (Camera System)
├── TransformSystem   (Transform System)
└── InputSystem       (Input System)
```

### Rendering Pipeline (RenderGraph)

RenderGraph is the core rendering system of the engine, providing:

- Automatic dependency management and topological sorting
- Flexible Pass system (ClearPass, OpaquePass, OITPass, CompositePass, PresentPass)
- Automatic resource management (RenderTarget, DepthStencil, ShaderResource)

For detailed usage guide, please refer to [RenderGraph Guide](Docs/RenderGraph_Guide.md)

### Component System

The engine supports the following component types:

- `MeshComponent`: Mesh rendering
- `MaterialComponent`: Material properties
- `SkeletonComponent`: Skeletal animation
- `NvFlexComponent`: Flex physics simulation
- `CameraComponent`: Camera
- `TransformComponent`: Transform matrix
- `NameComponent`: Entity naming

## 🚀 Getting Started

### Requirements

- Windows 10/11
- Visual Studio 2019 or higher
- DirectX 11 compatible graphics card
- Qt 6.x SDK (for editor)
- CMake 3.15+ (optional)

### Build Instructions

1. **Clone the repository**

```powershell
git clone https://github.com/leafvmaple/Zongine.git
cd Zongine
```

2. **Initialize submodules** (if any)

```powershell
git submodule update --init --recursive
```

3. **Open the solution**

```powershell
# Open with Visual Studio
start Zongine.sln
```

4. **Configure Qt environment**
   - Ensure Qt Visual Studio Tools extension is installed
   - Configure Qt 6 path

5. **Build the project**
   - Select `Debug` or `Release` configuration
   - Build the `Editor` project or `Simple` example project

### Running the Editor

After building, run `Editor.exe`:

```powershell
.\x64\Debug\Editor.exe
```

## 📁 Project Structure

```text
Zongine/
├── Docs/                      # Documentation
│   ├── Reference.md          # Reference documentation
│   ├── RenderGraph_Guide.md  # RenderGraph usage guide
│   └── TODO.md               # Development roadmap
│
├── Engine/                    # Engine source code
│   ├── 3rd/                  # Third-party libraries
│   │   ├── DirectXTex/       # Texture processing library
│   │   ├── FX11/             # Effects11
│   │   └── NVFlex/           # NVIDIA Flex
│   │
│   ├── Include/              # Public header files
│   ├── Shader/               # Shader source code
│   │
│   └── Source/
│       ├── Editor/           # Qt editor
│       ├── Runtime/          # Runtime engine
│       │   ├── Components/   # Component definitions
│       │   ├── Systems/      # System implementations
│       │   ├── RenderGraph/  # Render graph
│       │   └── Entities/     # Entity management
│       ├── LLoader/          # Asset loader
│       ├── Maths/            # Math library
│       └── Simple/           # Simple example
│
├── Lib/                      # Compiled library files
├── Product/                  # Product output
│   └── data/                 # Asset data
└── x64/                      # Build output
```

## 📚 Documentation

- [RenderGraph Guide](Docs/RenderGraph_Guide.md) - Detailed rendering pipeline usage documentation
- [TODO](Docs/TODO.md) - Feature development and roadmap

## 🎯 Roadmap

- [ ] Improve material system
- [ ] PBR rendering support
- [ ] Post-processing effects system
- [ ] More physics effects
- [ ] Scene serialization and loading
- [ ] Asset hot-reloading

## 🤝 Contributing

Issues and Pull Requests are welcome!

## 📄 License

This project is licensed under the [BSD 3-Clause License](LICENSE).

Copyright (c) 2025, Zohar Lee

## 🙏 Acknowledgments

Thanks to the following open source projects:

- [DirectXTex](https://github.com/microsoft/DirectXTex)
- [Effects11](https://github.com/microsoft/FX11)
- [NVIDIA Flex](https://developer.nvidia.com/flex)
- [Qt](https://www.qt.io/)

## 📧 Contact

- Author: Zohar Lee (leafvmaple)
- Repository: [https://github.com/leafvmaple/Zongine](https://github.com/leafvmaple/Zongine)

---

<div align="center">
Made with ❤️ by Zohar Lee
</div>
