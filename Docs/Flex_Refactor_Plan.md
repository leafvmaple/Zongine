# Flex物理系统重构计划

## 当前架构问题

### 1. 顶点颜色滥用
```cpp
// AssetManager.cpp - 用顶点颜色alpha存储invMass
const auto& diffuse = vertex.Color;
float invMass = 0.f;
if (diffuse.a >= 255)
    invMass = 0.f;
else if (diffuse.a == 0)
    invMass = 1.f;
else if (diffuse.a >= 100)
    invMass = std::pow(1.10f, 100.f - diffuse.a);
```
- 非标准，难以理解
- 占用了顶点颜色通道
- 美术无法正常使用顶点颜色

### 2. 额外顶点Buffer
```cpp
// Const.h
struct FLEX_VERTEX_EXT {
    DirectX::XMFLOAT4 FlexPosition;
    float MixFactor;
};

// FlexMeshShader.fx5
float4 FlexPosition  : FLEX_POSITION0;
float  MixFactor : MIX_FACTOR0;
```
- Shader需要额外语义
- InputLayout复杂化
- 需要维护两套顶点数据同步

### 3. 配置文件耦合
```
F1_5407h_body.mesh
F1_5407h_body.mesh.flx  <- 必须同名，强耦合
```

## 现代商业引擎做法

### Unreal Engine
- Cloth Physics是USkeletalMeshComponent的子模块
- 约束点通过Paint Tool定义（存储在独立数据资产）
- 物理模拟结果直接写入顶点Buffer

### Unity
- Cloth组件独立于SkinnedMeshRenderer
- 使用Cloth Constraint Paint定义固定点
- 配置存储在Cloth组件内

## 重构方案

### 目录结构
```
Product/assets/
├── Models/
│   └── Player/
│       └── F1_5407h_body.gltf      # 标准glTF，无物理数据
└── Physics/
    └── Cloth/
        └── F1_5407h_body.cloth.json # 独立物理配置
```

### 配置格式 (cloth.json)
```json
{
  "version": 1,
  "type": "cloth",
  "mesh": "assets/Models/Player/F1_5407h_body.gltf",
  
  "simulation": {
    "solver_iterations": 4,
    "substeps": 2
  },
  
  "material": {
    "stretch_stiffness": 0.9,
    "bend_stiffness": 0.5,
    "tether_stiffness": 0.8,
    "tether_give": 0.1,
    "damping": 0.1,
    "friction": 0.5
  },
  
  "constraints": {
    "method": "bone_weight",
    "fixed_bones": [
      "bip01 spine2",
      "bip01 l clavicle", 
      "bip01 r clavicle"
    ],
    "weight_threshold": 0.8
  },
  
  "collision": {
    "self_collision": false,
    "capsules": [
      { "bone": "bip01 spine", "radius": 0.1, "length": 0.2 }
    ]
  }
}
```

### 核心代码改动

#### 1. 删除FLEX_VERTEX_EXT
```cpp
// 删除 Const.h 中的 FLEX_VERTEX_EXT
// 删除 EffectManager.cpp 中的 FLEX_POSITION/MIX_FACTOR InputElement
```

#### 2. 修改顶点Buffer为可写
```cpp
// 改为 D3D11_USAGE_DEFAULT + UAV
desc.Usage = D3D11_USAGE_DEFAULT;
desc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_UNORDERED_ACCESS;
```

#### 3. 新建ClothComponent
```cpp
struct ClothComponent {
    std::string ConfigPath;           // cloth.json路径
    std::vector<int> FixedVertices;   // 固定点索引
    std::vector<float> InvMass;       // 每顶点质量倒数
    
    // NvFlex数据
    NvFlexBuffer* ParticleBuffer;
    NvFlexBuffer* VelocityBuffer;
};
```

#### 4. 固定点判定逻辑
```cpp
// 根据骨骼权重判定
for (int i = 0; i < vertexCount; i++) {
    float maxWeight = 0;
    int maxBoneIdx = -1;
    
    for (int j = 0; j < 4; j++) {
        if (vertex.BoneWeights[j] > maxWeight) {
            maxWeight = vertex.BoneWeights[j];
            maxBoneIdx = vertex.BoneIndices[j];
        }
    }
    
    // 如果该骨骼在fixed_bones列表中，且权重超过阈值
    if (IsFixedBone(maxBoneIdx) && maxWeight >= weightThreshold) {
        invMass[i] = 0.0f;  // 固定点
    } else {
        invMass[i] = 1.0f;  // 自由点
    }
}
```

#### 5. Compute Shader更新顶点
```hlsl
// ClothUpdate.hlsl
RWByteAddressBuffer VertexBuffer : register(u0);
StructuredBuffer<float4> SimulatedPositions : register(t0);

[numthreads(64, 1, 1)]
void CSMain(uint3 id : SV_DispatchThreadID) {
    uint vertexIdx = id.x;
    float4 newPos = SimulatedPositions[vertexIdx];
    
    // 写入顶点buffer的Position字段 (offset 0, size 12)
    VertexBuffer.Store3(vertexIdx * VERTEX_STRIDE, asuint(newPos.xyz));
}
```

#### 6. 渲染流程简化
```cpp
// 不再需要FlexMeshShader.fx5
// 直接使用SkinMeshShader.fx5
// 物理模拟后的位置已经在主顶点Buffer中
```

### player.json配置更新
```json
{
  "skeleton": "assets/Skeletons/F1.gltf",
  "mesh": {
    "head": "assets/Models/Player/F1_5407h_a_head.gltf",
    "body": "assets/Models/Player/F1_5407h_body.gltf",
    "face": "assets/Models/Player/f1_new_face.gltf"
  },
  "cloth": {
    "body": "assets/Physics/Cloth/F1_5407h_body.cloth.json"
  }
}
```

## 实施步骤

### Phase 1: 配置迁移
1. 创建 `ClothConfig` 结构体和JSON加载
2. 从 `.mesh.flx` 提取参数写入 `.cloth.json`
3. 实现骨骼权重判定固定点逻辑

### Phase 2: 渲染简化
1. 修改顶点Buffer为UAV可写
2. 删除 `FLEX_VERTEX_EXT` 相关代码
3. Shader统一使用 `SkinMeshShader`

### Phase 3: 物理更新
1. 实现Compute Shader写入顶点Buffer
2. 删除旧的 `FlexMeshShader.fx5`
3. 清理 `NvFlexComponent` 冗余字段

### Phase 4: 清理
1. 删除顶点颜色alpha作为物理参数的逻辑
2. 删除 `.mesh.flx` 文件支持（保留兼容读取）
3. 更新文档

## 兼容性考虑

- 旧的 `.mesh.flx` 可自动转换为 `.cloth.json`
- 顶点颜色alpha作为备选判定方式（逐步废弃）
- Shader保持双路径：有ClothComponent用物理位置，无则正常蒙皮

## 参考资料

- [NvFlex Documentation](https://developer.nvidia.com/flex)
- [Unreal Cloth Tool](https://docs.unrealengine.com/cloth-tool)
- [Unity Cloth Component](https://docs.unity3d.com/Manual/class-Cloth.html)
