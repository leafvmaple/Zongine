#!/usr/bin/env python3
"""
Zongine to glTF Converter
支持将Zongine私有格式(.mesh, .ani)转换为标准glTF 2.0格式

使用方法:
  python zongine_to_gltf_converter.py mesh <input.mesh> <output.gltf>
  python zongine_to_gltf_converter.py animation <input.ani> <output.gltf>
  python zongine_to_gltf_converter.py batch <input_dir> <output_dir>

作者: Zongine Team
"""

import sys
import struct
import os
import json
import math
from typing import List, Tuple, Dict, Optional, Any
from dataclasses import dataclass, field
from pathlib import Path


# ============= Animation 数据结构 =============

ANI_FILE_MASK = 0x414E494D  # "ANIM"
ANI_FILE_MASK_V2 = 0x324E494D  # "MIN2"
ANI_STRING_SIZE = 30


@dataclass
class RTS:
    """旋转平移缩放数据"""
    translation: List[float] = field(default_factory=lambda: [0.0, 0.0, 0.0])
    scale: List[float] = field(default_factory=lambda: [1.0, 1.0, 1.0])
    rotation: List[float] = field(default_factory=lambda: [0.0, 0.0, 0.0, 1.0])  # xyzw
    s_rotation: List[float] = field(default_factory=lambda: [0.0, 0.0, 0.0, 1.0])


@dataclass
class AnimationTrack:
    """单个骨骼的动画轨道"""
    bone_name: str
    frames: List[RTS]


@dataclass
class AnimationData:
    """动画数据"""
    name: str = ""
    frame_count: int = 0
    frame_length: float = 33.333  # ms per frame (30fps default)
    tracks: List[AnimationTrack] = field(default_factory=list)


# ============= Mesh 数据结构 =============

@dataclass
class Vertex:
    """顶点数据"""
    position: List[float] = field(default_factory=lambda: [0.0, 0.0, 0.0])
    normal: List[float] = field(default_factory=lambda: [0.0, 1.0, 0.0])
    tangent: List[float] = field(default_factory=lambda: [1.0, 0.0, 0.0, 1.0])
    color: List[float] = field(default_factory=lambda: [1.0, 1.0, 1.0, 1.0])
    texcoord: List[float] = field(default_factory=lambda: [0.0, 0.0])
    joints: List[int] = field(default_factory=lambda: [0, 0, 0, 0])
    weights: List[float] = field(default_factory=lambda: [0.0, 0.0, 0.0, 0.0])


@dataclass
class Bone:
    """骨骼数据"""
    name: str = ""
    children: List[str] = field(default_factory=list)
    offset_matrix: List[float] = field(default_factory=lambda: [1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1])
    parent_matrix: List[float] = field(default_factory=lambda: [1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1])
    inv_px_pose: List[float] = field(default_factory=lambda: [1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1])


@dataclass
class Socket:
    """插槽数据"""
    name: str = ""
    parent: str = ""
    matrix: List[float] = field(default_factory=lambda: [1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1])


@dataclass
class MeshData:
    """Mesh数据"""
    vertices: List[Vertex] = field(default_factory=list)
    indices: List[int] = field(default_factory=list)
    bones: List[Bone] = field(default_factory=list)
    sockets: List[Socket] = field(default_factory=list)
    subset_indices: List[int] = field(default_factory=list)
    
    has_positions: bool = False
    has_normals: bool = False
    has_tangents: bool = False
    has_colors: bool = False
    has_texcoords: bool = False
    has_skin_data: bool = False


# ============= Animation 解析器 =============

class AnimationParser:
    """Zongine动画文件解析器"""
    
    @staticmethod
    def _short4_to_float4(wzyx: List[int]) -> List[float]:
        """压缩旋转转换为四元数"""
        CRPRECISION = 1.0 / 32767.0
        return [
            wzyx[0] * CRPRECISION,
            wzyx[1] * CRPRECISION, 
            wzyx[2] * CRPRECISION,
            wzyx[3] * CRPRECISION
        ]
    
    @staticmethod
    def _quaternion_inverse(q: List[float]) -> List[float]:
        """四元数求逆"""
        # q = [x, y, z, w]
        return [-q[0], -q[1], -q[2], q[3]]
    
    def parse(self, filename: str) -> Optional[AnimationData]:
        """解析动画文件"""
        try:
            with open(filename, 'rb') as f:
                return self._parse_file(f, filename)
        except Exception as e:
            print(f"解析动画文件失败: {e}")
            import traceback
            traceback.print_exc()
            return None
    
    def _parse_file(self, f, filename: str) -> Optional[AnimationData]:
        """解析文件内容"""
        # 读取文件头
        mask = struct.unpack('<I', f.read(4))[0]
        block_length = struct.unpack('<I', f.read(4))[0]
        num_animations = struct.unpack('<I', f.read(4))[0]
        anim_type = struct.unpack('<I', f.read(4))[0]
        desc = f.read(ANI_STRING_SIZE).decode('ascii', errors='ignore').rstrip('\x00')
        
        print(f"动画文件信息:")
        print(f"  Mask: 0x{mask:08X}")
        print(f"  Type: {anim_type}")
        print(f"  Description: {desc}")
        
        animation = AnimationData()
        animation.name = Path(filename).stem
        
        if mask == ANI_FILE_MASK:
            return self._parse_v1(f, animation)
        elif mask == ANI_FILE_MASK_V2:
            return self._parse_v2(f, animation)
        else:
            print(f"未知的动画文件格式: 0x{mask:08X}")
            return None
    
    def _parse_v1(self, f, animation: AnimationData) -> AnimationData:
        """解析V1格式动画"""
        # 读取骨骼动画头
        num_bones = struct.unpack('<I', f.read(4))[0]
        num_frames = struct.unpack('<I', f.read(4))[0]
        frame_length = struct.unpack('<f', f.read(4))[0]
        
        animation.frame_count = num_frames
        animation.frame_length = frame_length
        
        print(f"  骨骼数: {num_bones}")
        print(f"  帧数: {num_frames}")
        print(f"  帧长度: {frame_length}ms")
        
        # 读取骨骼名称
        bone_names = []
        for i in range(num_bones):
            name = f.read(ANI_STRING_SIZE).decode('ascii', errors='ignore').rstrip('\x00').lower()
            bone_names.append(name)
        
        # 读取RTS数据
        for i in range(num_bones):
            track = AnimationTrack(bone_name=bone_names[i], frames=[])
            
            for j in range(num_frames):
                # RTS结构: Translation(3f), Scale(3f), Rotation(4f), Sign(f), SRotation(4f)
                translation = list(struct.unpack('<fff', f.read(12)))
                scale = list(struct.unpack('<fff', f.read(12)))
                rotation = list(struct.unpack('<ffff', f.read(16)))
                sign = struct.unpack('<f', f.read(4))[0]
                s_rotation = list(struct.unpack('<ffff', f.read(16)))
                
                # V1格式需要对旋转取逆
                rotation = self._quaternion_inverse(rotation)
                
                rts = RTS(
                    translation=translation,
                    scale=scale,
                    rotation=rotation,
                    s_rotation=s_rotation
                )
                track.frames.append(rts)
            
            animation.tracks.append(track)
        
        return animation
    
    def _parse_v2(self, f, animation: AnimationData) -> AnimationData:
        """解析V2格式动画（压缩格式）"""
        # 读取骨骼动画头V2
        num_bones = struct.unpack('<I', f.read(4))[0]
        real_ani_bones = struct.unpack('<I', f.read(4))[0]
        num_frames = struct.unpack('<I', f.read(4))[0]
        frame_length = struct.unpack('<f', f.read(4))[0]
        has_bone_name = struct.unpack('<I', f.read(4))[0]
        bone_map_hash = struct.unpack('<I', f.read(4))[0]
        
        animation.frame_count = num_frames
        animation.frame_length = frame_length
        
        print(f"  骨骼数: {num_bones}")
        print(f"  实际动画骨骼数: {real_ani_bones}")
        print(f"  帧数: {num_frames}")
        print(f"  帧长度: {frame_length}ms")
        
        assert has_bone_name, "V2格式需要骨骼名称"
        
        # 读取骨骼名称
        bone_names = []
        for i in range(num_bones):
            name = f.read(ANI_STRING_SIZE).decode('ascii', errors='ignore').rstrip('\x00').lower()
            bone_names.append(name)
        
        # 读取初始RTS数据 (_RTSV2)
        # _RTSV2结构体: BYTE nFlag + 3字节padding + Translation(12) + Scale(12) + Rotation(16) + SRotation(16)
        # 总大小 = 4 + 12 + 12 + 16 + 16 = 60字节 (由于对齐)
        init_rts_data = []
        for i in range(num_bones):
            flag = struct.unpack('<B', f.read(1))[0]
            f.seek(3, 1)  # 跳过padding (对齐到4字节)
            translation = list(struct.unpack('<fff', f.read(12)))
            scale = list(struct.unpack('<fff', f.read(12)))
            rotation = list(struct.unpack('<ffff', f.read(16)))
            s_rotation = list(struct.unpack('<ffff', f.read(16)))
            
            init_rts_data.append({
                'flag': flag,
                'translation': translation,
                'scale': scale,
                'rotation': rotation,
                's_rotation': s_rotation
            })
        
        # 读取骨骼到动画的索引映射
        bone_to_anim_indices = list(struct.unpack(f'<{num_bones}i', f.read(num_bones * 4)))
        
        # 读取动画索引标志
        anim_index_flags = list(struct.unpack(f'<{real_ani_bones}B', f.read(real_ani_bones)))
        
        # 标志位定义
        ONLY_ROTATION = 1
        ONLY_ROTATION_TRANSLATION = 2
        ONLY_ROTATION_TRANSLATION_AFFINESCALE = 3
        ROTATION = 1 << 2
        TRANSLATION = 1 << 3
        AFFINESCALE = 1 << 4
        SCALE = 1 << 5
        SROTATION = 1 << 6
        
        # 解析初始RTS
        init_rts = []
        for i in range(num_bones):
            data = init_rts_data[i]
            flag = data['flag']
            
            rts = RTS()
            
            if flag in [ONLY_ROTATION, ONLY_ROTATION_TRANSLATION, ONLY_ROTATION_TRANSLATION_AFFINESCALE] or (flag & ROTATION):
                rts.rotation = data['rotation']
            
            if flag in [ONLY_ROTATION_TRANSLATION, ONLY_ROTATION_TRANSLATION_AFFINESCALE] or (flag & TRANSLATION):
                rts.translation = data['translation']
            
            if flag == ONLY_ROTATION_TRANSLATION_AFFINESCALE or (flag & AFFINESCALE):
                rts.scale = data['scale']
            elif flag & SCALE:
                rts.scale = data['scale']
                if flag & SROTATION:
                    rts.s_rotation = data['s_rotation']
            
            init_rts.append(rts)
        
        # 解析每个骨骼的动画数据
        for i in range(num_bones):
            track = AnimationTrack(bone_name=bone_names[i], frames=[])
            
            # 初始化所有帧为初始RTS
            for j in range(num_frames):
                rts = RTS(
                    translation=init_rts[i].translation.copy(),
                    scale=init_rts[i].scale.copy(),
                    rotation=init_rts[i].rotation.copy(),
                    s_rotation=init_rts[i].s_rotation.copy()
                )
                track.frames.append(rts)
            
            anim_index = bone_to_anim_indices[i]
            
            if anim_index >= 0 and anim_index < real_ani_bones:
                flag = anim_index_flags[anim_index]
                
                # 读取旋转数据
                rotations = None
                if flag in [ONLY_ROTATION, ONLY_ROTATION_TRANSLATION, ONLY_ROTATION_TRANSLATION_AFFINESCALE] or (flag & ROTATION):
                    rotations = []
                    for j in range(num_frames):
                        compressed = list(struct.unpack('<4h', f.read(8)))
                        rotations.append(self._short4_to_float4(compressed))
                
                # 读取平移数据
                translations = None
                if flag in [ONLY_ROTATION_TRANSLATION, ONLY_ROTATION_TRANSLATION_AFFINESCALE] or (flag & TRANSLATION):
                    translations = []
                    for j in range(num_frames):
                        translations.append(list(struct.unpack('<fff', f.read(12))))
                
                # 读取缩放数据
                scales = None
                affine_scales = None
                s_rotations = None
                
                if flag == ONLY_ROTATION_TRANSLATION_AFFINESCALE or (flag & AFFINESCALE):
                    affine_scales = list(struct.unpack(f'<{num_frames}f', f.read(num_frames * 4)))
                elif flag & SCALE:
                    scales = []
                    for j in range(num_frames):
                        scales.append(list(struct.unpack('<fff', f.read(12))))
                    
                    if flag & SROTATION:
                        s_rotations = []
                        for j in range(num_frames):
                            compressed = list(struct.unpack('<4h', f.read(8)))
                            s_rotations.append(self._short4_to_float4(compressed))
                
                # 应用动画数据到帧
                for j in range(num_frames):
                    rts = track.frames[j]
                    
                    if rotations:
                        rts.rotation = rotations[j]
                    if translations:
                        rts.translation = translations[j]
                    if scales:
                        rts.scale = scales[j]
                    elif affine_scales:
                        s = affine_scales[j]
                        rts.scale = [s, s, s]
                    if s_rotations:
                        rts.s_rotation = s_rotations[j]
            
            animation.tracks.append(track)
        
        return animation


# ============= Mesh 解析器 =============

class MeshParser:
    """Zongine Mesh文件解析器"""
    
    def parse(self, filename: str) -> Optional[MeshData]:
        """解析Mesh文件"""
        try:
            with open(filename, 'rb') as f:
                return self._parse_file(f, filename)
        except Exception as e:
            print(f"解析Mesh文件失败: {e}")
            import traceback
            traceback.print_exc()
            return None
    
    def _parse_file(self, f, filename: str) -> Optional[MeshData]:
        """解析文件内容"""
        # 跳过通用头部（84字节）
        f.seek(84)
        
        # 读取版本头部
        file_mask = struct.unpack('<I', f.read(4))[0]
        if file_mask != 0x4D455348:  # "MESH"
            print(f"无效的Mesh文件格式: 0x{file_mask:08X}")
            return None
        
        block_length = struct.unpack('<I', f.read(4))[0]
        version = struct.unpack('<I', f.read(4))[0]
        
        # 跳过动画块指针和扩展块指针
        f.seek(44, 1)  # 4 + 40 bytes
        mesh_count = struct.unpack('<I', f.read(4))[0]
        
        # 读取mesh头部
        vertex_count = struct.unpack('<I', f.read(4))[0]
        face_count = struct.unpack('<I', f.read(4))[0]
        subset_count = struct.unpack('<I', f.read(4))[0]
        
        print(f"Mesh文件信息:")
        print(f"  顶点数: {vertex_count}")
        print(f"  面数: {face_count}")
        print(f"  材质组数: {subset_count}")
        print(f"  版本: 0x{version:08X}")
        
        # 读取数据块偏移（30个DWORD）
        block_names = [
            'PositionBlock', 'NormalBlock', 'DiffuseBlock', 'TextureUVW1Block',
            'TextureUVW2Block', 'TextureUVW3Block', 'FacesIndexBlock', 'SubsetIndexBlock',
            'SkinInfoBlock', 'LODInfoBlock', 'FlexibleBodyBlock', 'BBoxBlock',
            'BlendMeshBlock', 'TangentBlock', 'VetexRemapBlock', 'TextureUVW9Block',
            'EmissiveBlock'
        ]
        
        blocks = {}
        for i in range(30):
            offset = struct.unpack('<I', f.read(4))[0]
            if i < len(block_names):
                blocks[block_names[i]] = offset
        
        # 版本标志位解析 - 匹配C++逻辑
        VERSION_BIT_NEWBIT = 0
        VERSION_BIT_ADDITIVE_PX_POSE = 5
        VERSION_BIT_SUBSETSHORT = 2
        
        if version & (0x80000000 >> VERSION_BIT_NEWBIT):  # 新格式
            has_px_pose = bool(version & (0x80000000 >> VERSION_BIT_ADDITIVE_PX_POSE))
            subset_short = bool(version & (0x80000000 >> VERSION_BIT_SUBSETSHORT))
        else:  # 旧格式
            has_px_pose = version >= 2
            subset_short = version >= 1
        
        has_bbox = blocks.get('BBoxBlock', 0) > 0
        
        print(f"  has_px_pose: {has_px_pose}, has_bbox: {has_bbox}, subset_short: {subset_short}")
        
        mesh = MeshData()
        mesh.vertices = [Vertex() for _ in range(vertex_count)]
        
        # 解析各数据块
        if blocks.get('PositionBlock', 0):
            self._parse_positions(f, blocks['PositionBlock'], mesh, vertex_count)
        
        if blocks.get('NormalBlock', 0):
            self._parse_normals(f, blocks['NormalBlock'], mesh, vertex_count)
        
        if blocks.get('TangentBlock', 0):
            self._parse_tangents(f, blocks['TangentBlock'], mesh, vertex_count)
        
        if blocks.get('DiffuseBlock', 0):
            self._parse_colors(f, blocks['DiffuseBlock'], mesh, vertex_count)
        
        if blocks.get('TextureUVW1Block', 0):
            self._parse_texcoords(f, blocks['TextureUVW1Block'], mesh, vertex_count)
        
        if blocks.get('FacesIndexBlock', 0):
            self._parse_indices(f, blocks['FacesIndexBlock'], mesh, face_count)
        
        if blocks.get('SubsetIndexBlock', 0):
            self._parse_subset_indices(f, blocks['SubsetIndexBlock'], mesh, face_count)
        
        if blocks.get('SkinInfoBlock', 0):
            self._parse_skin_info(f, blocks['SkinInfoBlock'], mesh, vertex_count, 
                                 has_px_pose, has_bbox)
        
        return mesh
    
    def _parse_positions(self, f, offset: int, mesh: MeshData, count: int):
        """解析顶点位置"""
        f.seek(offset)
        for i in range(count):
            mesh.vertices[i].position = list(struct.unpack('<fff', f.read(12)))
        mesh.has_positions = True
    
    def _parse_normals(self, f, offset: int, mesh: MeshData, count: int):
        """解析法线"""
        f.seek(offset)
        for i in range(count):
            mesh.vertices[i].normal = list(struct.unpack('<fff', f.read(12)))
        mesh.has_normals = True
    
    def _parse_tangents(self, f, offset: int, mesh: MeshData, count: int):
        """解析切线"""
        f.seek(offset)
        for i in range(count):
            mesh.vertices[i].tangent = list(struct.unpack('<ffff', f.read(16)))
        mesh.has_tangents = True
    
    def _parse_colors(self, f, offset: int, mesh: MeshData, count: int):
        """解析颜色"""
        f.seek(offset)
        for i in range(count):
            color = struct.unpack('<I', f.read(4))[0]
            r = ((color >> 16) & 0xFF) / 255.0
            g = ((color >> 8) & 0xFF) / 255.0
            b = (color & 0xFF) / 255.0
            a = ((color >> 24) & 0xFF) / 255.0
            mesh.vertices[i].color = [r, g, b, a]
        mesh.has_colors = True
    
    def _parse_texcoords(self, f, offset: int, mesh: MeshData, count: int):
        """解析纹理坐标"""
        f.seek(offset)
        for i in range(count):
            u, v, _ = struct.unpack('<fff', f.read(12))  # UVW -> UV
            mesh.vertices[i].texcoord = [u, v]
        mesh.has_texcoords = True
    
    def _parse_indices(self, f, offset: int, mesh: MeshData, face_count: int):
        """解析索引"""
        f.seek(offset)
        index_count = face_count * 3
        mesh.indices = list(struct.unpack(f'<{index_count}I', f.read(index_count * 4)))
    
    def _parse_subset_indices(self, f, offset: int, mesh: MeshData, face_count: int):
        """解析材质组索引"""
        f.seek(offset)
        try:
            # 尝试DWORD格式
            data = f.read(face_count * 4)
            indices = list(struct.unpack(f'<{face_count}I', data))
            if max(indices) < 1000:
                mesh.subset_indices = indices
                return
        except:
            pass
        
        # 尝试WORD格式
        f.seek(offset)
        try:
            data = f.read(face_count * 2)
            mesh.subset_indices = list(struct.unpack(f'<{face_count}H', data))
        except:
            mesh.subset_indices = []
    
    def _parse_skin_info(self, f, offset: int, mesh: MeshData, vertex_count: int,
                        has_px_pose: bool, has_bbox: bool):
        """解析蒙皮信息"""
        f.seek(offset)
        
        bone_count = struct.unpack('<I', f.read(4))[0]
        print(f"  骨骼数: {bone_count}")
        
        if bone_count > 500:  # 合理的骨骼数量上限
            print(f"  警告: 骨骼数量异常 ({bone_count}), 跳过蒙皮数据")
            return
        
        for bone_idx in range(bone_count):
            bone = Bone()
            
            # 读取骨骼名称 (30字节) + 跳过填充 (30字节)
            bone.name = f.read(30).decode('ascii', errors='ignore').rstrip('\x00').lower()
            f.seek(30, 1)  # 跳过填充（另外30字节，通常为0）
            
            # 读取子骨骼数量
            child_count = struct.unpack('<I', f.read(4))[0]
            
            if child_count > 0:
                if child_count > 100:  # 合理的上限检查
                    print(f"  警告: 骨骼 {bone.name} 的子骨骼数量异常: {child_count}")
                    # 尝试恢复，假设这是格式错误
                    break
                
                # 每个子骨骼名称是30字节 (NAME_STRING)
                for _ in range(child_count):
                    child_name = f.read(30).decode('ascii', errors='ignore').rstrip('\x00').lower()
                    bone.children.append(child_name)
            
            # 读取变换矩阵 mOffset (64字节 XMFLOAT4X4)
            bone.offset_matrix = list(struct.unpack('<16f', f.read(64)))
            
            # 读取变换矩阵 mOffset2Parent (64字节)
            bone.parent_matrix = list(struct.unpack('<16f', f.read(64)))
            
            # 如果有物理姿势，读取 mInvPxPose (64字节)
            if has_px_pose:
                bone.inv_px_pose = list(struct.unpack('<16f', f.read(64)))
            
            # 读取受影响的顶点数量
            ref_vertex_count = struct.unpack('<I', f.read(4))[0]
            
            # 健壮性检查
            if ref_vertex_count > vertex_count * 10 or ref_vertex_count > 1000000:
                print(f"  警告: 骨骼 {bone.name} 的顶点引用数量异常: {ref_vertex_count}")
                # 这通常意味着解析出错了
                mesh.bones.append(bone)
                break
            
            if ref_vertex_count > 0:
                vertex_indices = list(struct.unpack(f'<{ref_vertex_count}I', 
                                                    f.read(ref_vertex_count * 4)))
                weights = list(struct.unpack(f'<{ref_vertex_count}f', 
                                            f.read(ref_vertex_count * 4)))
                
                for i, vertex_idx in enumerate(vertex_indices):
                    if vertex_idx < vertex_count:
                        v = mesh.vertices[vertex_idx]
                        for j in range(4):
                            if v.weights[j] == 0.0:
                                v.joints[j] = bone_idx
                                v.weights[j] = weights[i]
                                break
            
            # 如果有边界框数据
            if has_bbox:
                f.seek(64, 1)  # 跳过变换矩阵 (XMFLOAT4X4)
                f.seek(24, 1)  # 跳过BoundingBox (6 floats = 24 bytes)
                f.seek(4, 1)   # 跳过BOOL (4 bytes)
            
            mesh.bones.append(bone)
        
        # 读取Socket信息
        try:
            socket_count = struct.unpack('<I', f.read(4))[0]
            for _ in range(socket_count):
                socket = Socket()
                socket.name = f.read(30).decode('ascii', errors='ignore').rstrip('\x00').lower()
                socket.parent = f.read(30).decode('ascii', errors='ignore').rstrip('\x00').lower()
                socket.matrix = list(struct.unpack('<16f', f.read(64)))
                mesh.sockets.append(socket)
        except:
            pass
        
        mesh.has_skin_data = bone_count > 0


# ============= Skeleton 数据结构 =============

@dataclass
class SkeletonBone:
    """骨架骨骼数据"""
    name: str = ""
    children: List[str] = field(default_factory=list)
    parent_index: int = -1


@dataclass
class SkeletonData:
    """骨架数据"""
    bones: List[SkeletonBone] = field(default_factory=list)
    root_indices: List[int] = field(default_factory=list)


# ============= Skeleton 解析器 =============

class SkeletonParser:
    """Zongine骨架文件解析器（.txt格式）"""
    
    def parse(self, filename: str) -> Optional[SkeletonData]:
        """解析骨架文件"""
        try:
            with open(filename, 'r', encoding='utf-8') as f:
                lines = f.readlines()
            
            if not lines:
                print(f"骨架文件为空: {filename}")
                return None
            
            # 第一行：骨骼数量
            bone_count = int(lines[0].strip())
            print(f"骨架文件信息:")
            print(f"  骨骼数量: {bone_count}")
            
            skeleton = SkeletonData()
            bone_name_to_idx = {}
            
            # 解析每个骨骼行
            for i in range(1, bone_count + 1):
                if i >= len(lines):
                    break
                    
                line = lines[i].strip()
                if not line:
                    continue
                
                # 用双空格分隔
                parts = line.split('  ')
                bone_name = parts[0].lower()
                children = [p.lower() for p in parts[1:] if p]
                
                bone = SkeletonBone(name=bone_name, children=children)
                bone_name_to_idx[bone_name] = len(skeleton.bones)
                skeleton.bones.append(bone)
            
            # 建立父子关系
            for i, bone in enumerate(skeleton.bones):
                for child_name in bone.children:
                    child_idx = bone_name_to_idx.get(child_name, -1)
                    if child_idx >= 0:
                        skeleton.bones[child_idx].parent_index = i
            
            # 读取根骨骼信息（在骨骼定义之后）
            root_line_idx = bone_count + 1
            if root_line_idx < len(lines):
                try:
                    root_count = int(lines[root_line_idx].strip())
                    for j in range(root_count):
                        if root_line_idx + 1 + j < len(lines):
                            root_idx = int(lines[root_line_idx + 1 + j].strip())
                            skeleton.root_indices.append(root_idx)
                except ValueError:
                    pass
            
            # 如果没有明确的根骨骼，找parent_index=-1的骨骼
            if not skeleton.root_indices:
                for i, bone in enumerate(skeleton.bones):
                    if bone.parent_index < 0:
                        skeleton.root_indices.append(i)
            
            print(f"  根骨骼数量: {len(skeleton.root_indices)}")
            return skeleton
            
        except Exception as e:
            print(f"解析骨架文件失败 {filename}: {e}")
            import traceback
            traceback.print_exc()
            return None


# ============= glTF 导出器 =============

class GltfExporter:
    """glTF导出器"""
    
    FLOAT = 5126
    UNSIGNED_INT = 5125
    UNSIGNED_SHORT = 5123
    UNSIGNED_BYTE = 5121
    ARRAY_BUFFER = 34962
    ELEMENT_ARRAY_BUFFER = 34963
    
    def export_mesh(self, mesh: MeshData, output_path: str) -> bool:
        """导出Mesh为glTF"""
        try:
            gltf_data = self._build_mesh_gltf(mesh)
            return self._save_gltf(gltf_data, output_path)
        except Exception as e:
            print(f"导出Mesh失败: {e}")
            import traceback
            traceback.print_exc()
            return False
    
    def export_animation(self, animation: AnimationData, output_path: str) -> bool:
        """导出Animation为glTF"""
        try:
            gltf_data = self._build_animation_gltf(animation)
            return self._save_gltf(gltf_data, output_path)
        except Exception as e:
            print(f"导出Animation失败: {e}")
            import traceback
            traceback.print_exc()
            return False
    
    def export_skeleton(self, skeleton: SkeletonData, output_path: str) -> bool:
        """导出骨架为glTF（纯骨骼层级，无mesh）"""
        try:
            gltf_data = self._build_skeleton_gltf(skeleton)
            return self._save_gltf(gltf_data, output_path)
        except Exception as e:
            print(f"导出骨架失败: {e}")
            import traceback
            traceback.print_exc()
            return False
    
    def _build_skeleton_gltf(self, skeleton: SkeletonData) -> Dict:
        """构建纯骨架的glTF数据"""
        nodes = []
        
        # 创建所有骨骼节点
        for i, bone in enumerate(skeleton.bones):
            node = {"name": bone.name}
            
            # 获取子节点索引
            children = []
            for child_name in bone.children:
                # 找到子骨骼的索引
                for j, b in enumerate(skeleton.bones):
                    if b.name == child_name:
                        children.append(j)
                        break
            
            if children:
                node["children"] = children
            
            nodes.append(node)
        
        # 构建skin（包含所有骨骼的引用）
        joints = list(range(len(skeleton.bones)))
        
        # 创建inverseBindMatrices accessor（单位矩阵）
        binary_data = bytearray()
        for _ in skeleton.bones:
            # 单位矩阵 (column-major)
            identity = [1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1]
            binary_data.extend(struct.pack('<16f', *identity))
        
        buffer_views = [{
            "buffer": 0,
            "byteOffset": 0,
            "byteLength": len(binary_data)
        }]
        
        accessors = [{
            "bufferView": 0,
            "componentType": self.FLOAT,
            "count": len(skeleton.bones),
            "type": "MAT4"
        }]
        
        skin = {
            "joints": joints,
            "inverseBindMatrices": 0
        }
        
        # 设置skeleton根节点
        if skeleton.root_indices:
            skin["skeleton"] = skeleton.root_indices[0]
        
        # 构建glTF结构
        gltf = {
            "asset": {
                "version": "2.0",
                "generator": "Zongine Skeleton Converter"
            },
            "scenes": [{"nodes": skeleton.root_indices if skeleton.root_indices else [0]}],
            "scene": 0,
            "nodes": nodes,
            "skins": [skin],
            "accessors": accessors,
            "bufferViews": buffer_views,
            "buffers": [{"byteLength": len(binary_data)}]
        }
        
        return {"json": gltf, "binary": binary_data}
    
    def _build_mesh_gltf(self, mesh: MeshData) -> Dict:
        """构建Mesh的glTF数据"""
        binary_data = bytearray()
        accessors = []
        buffer_views = []
        
        vertex_count = len(mesh.vertices)
        
        # 位置数据
        if mesh.has_positions:
            pos_data = bytearray()
            min_pos = [float('inf')] * 3
            max_pos = [float('-inf')] * 3
            
            for v in mesh.vertices:
                pos_data.extend(struct.pack('<fff', *v.position))
                for i in range(3):
                    min_pos[i] = min(min_pos[i], v.position[i])
                    max_pos[i] = max(max_pos[i], v.position[i])
            
            bv_idx = self._add_buffer_view(binary_data, buffer_views, pos_data, self.ARRAY_BUFFER)
            self._add_accessor(accessors, bv_idx, self.FLOAT, vertex_count, "VEC3", min_pos, max_pos)
        
        # 法线数据
        if mesh.has_normals:
            norm_data = bytearray()
            for v in mesh.vertices:
                norm_data.extend(struct.pack('<fff', *v.normal))
            bv_idx = self._add_buffer_view(binary_data, buffer_views, norm_data, self.ARRAY_BUFFER)
            self._add_accessor(accessors, bv_idx, self.FLOAT, vertex_count, "VEC3")
        
        # 纹理坐标
        if mesh.has_texcoords:
            tex_data = bytearray()
            for v in mesh.vertices:
                tex_data.extend(struct.pack('<ff', *v.texcoord))
            bv_idx = self._add_buffer_view(binary_data, buffer_views, tex_data, self.ARRAY_BUFFER)
            self._add_accessor(accessors, bv_idx, self.FLOAT, vertex_count, "VEC2")
        
        # 颜色数据
        if mesh.has_colors:
            color_data = bytearray()
            for v in mesh.vertices:
                color_data.extend(struct.pack('<ffff', *v.color))
            bv_idx = self._add_buffer_view(binary_data, buffer_views, color_data, self.ARRAY_BUFFER)
            self._add_accessor(accessors, bv_idx, self.FLOAT, vertex_count, "VEC4")
        
        # Tangent数据
        tangent_accessor_idx = None
        if mesh.has_tangents:
            tan_data = bytearray()
            for v in mesh.vertices:
                tan_data.extend(struct.pack('<ffff', *v.tangent))
            bv_idx = self._add_buffer_view(binary_data, buffer_views, tan_data, self.ARRAY_BUFFER)
            tangent_accessor_idx = len(accessors)
            self._add_accessor(accessors, bv_idx, self.FLOAT, vertex_count, "VEC4")
        
        # 蒙皮数据
        joints_accessor_idx = None
        weights_accessor_idx = None
        if mesh.has_skin_data:
            joints_data = bytearray()
            weights_data = bytearray()
            
            for v in mesh.vertices:
                joints_data.extend(struct.pack('<HHHH', *[min(j, 65535) for j in v.joints]))
                
                # 归一化权重
                total = sum(v.weights)
                if total > 0:
                    weights = [w / total for w in v.weights]
                else:
                    weights = v.weights
                weights_data.extend(struct.pack('<ffff', *weights))
            
            bv_idx = self._add_buffer_view(binary_data, buffer_views, joints_data, self.ARRAY_BUFFER)
            joints_accessor_idx = len(accessors)
            self._add_accessor(accessors, bv_idx, self.UNSIGNED_SHORT, vertex_count, "VEC4")
            
            bv_idx = self._add_buffer_view(binary_data, buffer_views, weights_data, self.ARRAY_BUFFER)
            weights_accessor_idx = len(accessors)
            self._add_accessor(accessors, bv_idx, self.FLOAT, vertex_count, "VEC4")
        
        # 按材质组(subset)分组索引数据
        primitives = []
        materials = []
        
        if mesh.subset_indices and len(mesh.subset_indices) > 0:
            # 有subset信息，按subset分组
            face_count = len(mesh.indices) // 3
            num_subsets = max(mesh.subset_indices) + 1 if mesh.subset_indices else 1
            
            # 为每个subset收集索引
            subset_indices_groups = [[] for _ in range(num_subsets)]
            for face_idx, subset_id in enumerate(mesh.subset_indices):
                if face_idx * 3 + 2 < len(mesh.indices):
                    subset_indices_groups[subset_id].extend([
                        mesh.indices[face_idx * 3],
                        mesh.indices[face_idx * 3 + 1],
                        mesh.indices[face_idx * 3 + 2]
                    ])
            
            print(f"  Mesh有 {num_subsets} 个材质组(subsets)")
            
            # 为每个subset创建索引accessor和primitive
            # 注意：必须保持顺序，即使subset是空的也不能跳过
            for subset_id, subset_indices in enumerate(subset_indices_groups):
                if not subset_indices:
                    # 空的subset也要保留占位，否则材质索引会错位
                    print(f"    Subset {subset_id}: 空 (跳过但保留占位)")
                    primitives.append(None)  # 占位符
                    materials.append({
                        "pbrMetallicRoughness": {
                            "baseColorFactor": [1.0, 1.0, 1.0, 1.0],
                            "metallicFactor": 0.0,
                            "roughnessFactor": 1.0
                        },
                        "name": f"Material_{subset_id}"
                    })
                    continue
                    
                indices_data = bytearray()
                for idx in subset_indices:
                    indices_data.extend(struct.pack('<I', idx))
                
                bv_idx = self._add_buffer_view(binary_data, buffer_views, indices_data, self.ELEMENT_ARRAY_BUFFER)
                indices_accessor_idx = len(accessors)
                self._add_accessor(accessors, bv_idx, self.UNSIGNED_INT, len(subset_indices), "SCALAR")
                
                primitives.append({
                    "attributes": {},  # 稍后填充
                    "indices": indices_accessor_idx,
                    "material": len(materials)
                })
                
                materials.append({
                    "pbrMetallicRoughness": {
                        "baseColorFactor": [1.0, 1.0, 1.0, 1.0],
                        "metallicFactor": 0.0,
                        "roughnessFactor": 1.0
                    },
                    "name": f"Material_{subset_id}"
                })
                
                print(f"    Subset {subset_id}: {len(subset_indices)} 个索引")
        else:
            # 没有subset信息，创建单个primitive
            indices_data = bytearray()
            for idx in mesh.indices:
                indices_data.extend(struct.pack('<I', idx))
            bv_idx = self._add_buffer_view(binary_data, buffer_views, indices_data, self.ELEMENT_ARRAY_BUFFER)
            indices_accessor_idx = len(accessors)
            self._add_accessor(accessors, bv_idx, self.UNSIGNED_INT, len(mesh.indices), "SCALAR")
            
            primitives.append({
                "attributes": {},
                "indices": indices_accessor_idx,
                "material": 0
            })
            materials.append({
                "pbrMetallicRoughness": {
                    "baseColorFactor": [1.0, 1.0, 1.0, 1.0],
                    "metallicFactor": 0.0,
                    "roughnessFactor": 1.0
                }
            })
        
        # 构建primitive attributes
        attributes = {}
        attr_idx = 0
        if mesh.has_positions:
            attributes["POSITION"] = attr_idx
            attr_idx += 1
        if mesh.has_normals:
            attributes["NORMAL"] = attr_idx
            attr_idx += 1
        if mesh.has_texcoords:
            attributes["TEXCOORD_0"] = attr_idx
            attr_idx += 1
        if mesh.has_colors:
            attributes["COLOR_0"] = attr_idx
            attr_idx += 1
        if mesh.has_tangents and tangent_accessor_idx is not None:
            attributes["TANGENT"] = tangent_accessor_idx
        if mesh.has_skin_data:
            attributes["JOINTS_0"] = joints_accessor_idx
            attributes["WEIGHTS_0"] = weights_accessor_idx
        
        # 将attributes应用到所有非空primitives
        for prim in primitives:
            if prim is not None:
                prim["attributes"] = attributes.copy()
        
        # 过滤掉空的primitives，但保留对应的材质索引关系
        # 注意：这里我们不能直接过滤，因为C++加载器期望primitive索引与材质索引一一对应
        # 所以我们需要保持所有非空primitives，材质顺序也要保持
        filtered_primitives = [p for p in primitives if p is not None]
        # materials已经是完整的列表，不需要过滤
        
        # 构建glTF JSON
        # 过滤掉空的primitives（None占位符）
        valid_primitives = [p for p in primitives if p is not None]
        
        gltf = {
            "asset": {"version": "2.0", "generator": "Zongine Converter"},
            "scenes": [{"nodes": [0]}],
            "scene": 0,
            "nodes": [{"mesh": 0}],
            "meshes": [{
                "primitives": valid_primitives
            }],
            "materials": materials,
            "accessors": accessors,
            "bufferViews": buffer_views,
            "buffers": [{"byteLength": len(binary_data)}]
        }
        
        # 添加蒙皮数据
        if mesh.has_skin_data and mesh.bones:
            bone_nodes = []
            joint_indices = []
            
            for i, bone in enumerate(mesh.bones):
                node_idx = len(gltf["nodes"])
                joint_indices.append(node_idx)
                
                bone_node = {
                    "name": bone.name,
                    "matrix": bone.parent_matrix,
                    "extras": {
                        "invPxPose": bone.inv_px_pose
                    }
                }
                gltf["nodes"].append(bone_node)
                bone_nodes.append(bone_node)
            
            # 构建骨骼层次
            for i, bone in enumerate(mesh.bones):
                children = []
                for child_name in bone.children:
                    for j, other_bone in enumerate(mesh.bones):
                        if other_bone.name == child_name:
                            children.append(joint_indices[j])
                            break
                if children:
                    bone_nodes[i]["children"] = children
            
            # 逆绑定矩阵
            # 注意：保持原始的行主序格式，因为C++端直接按行主序读取
            inv_bind_data = bytearray()
            for bone in mesh.bones:
                for val in bone.offset_matrix:
                    inv_bind_data.extend(struct.pack('<f', val))
            
            bv_idx = self._add_buffer_view(binary_data, buffer_views, inv_bind_data)
            inv_bind_accessor = len(accessors)
            self._add_accessor(accessors, bv_idx, self.FLOAT, len(mesh.bones), "MAT4")
            
            gltf["skins"] = [{
                "joints": joint_indices,
                "inverseBindMatrices": inv_bind_accessor
            }]
            gltf["nodes"][0]["skin"] = 0
            
            # 更新buffer长度
            gltf["buffers"][0]["byteLength"] = len(binary_data)
        
        # 添加Socket节点
        for socket in mesh.sockets:
            socket_node = {
                "name": f"Socket_{socket.name}",
                "matrix": socket.matrix,
                "extras": {"parent": socket.parent}
            }
            gltf["nodes"].append(socket_node)
        
        return {"json": gltf, "binary": binary_data}
    
    def _build_animation_gltf(self, animation: AnimationData) -> Dict:
        """构建Animation的glTF数据"""
        binary_data = bytearray()
        accessors = []
        buffer_views = []
        
        # 计算时间戳
        fps = 1000.0 / animation.frame_length if animation.frame_length > 0 else 30.0
        duration = (animation.frame_count - 1) / fps if animation.frame_count > 1 else 0.0
        
        # 时间输入数据（所有通道共享）
        time_data = bytearray()
        for i in range(animation.frame_count):
            t = i / fps
            time_data.extend(struct.pack('<f', t))
        
        time_bv_idx = self._add_buffer_view(binary_data, buffer_views, time_data)
        time_accessor_idx = len(accessors)
        self._add_accessor(accessors, time_bv_idx, self.FLOAT, animation.frame_count, "SCALAR",
                          [0.0], [duration])
        
        # 构建节点和通道
        nodes = []
        channels = []
        samplers = []
        
        for track_idx, track in enumerate(animation.tracks):
            node_idx = track_idx
            nodes.append({"name": track.bone_name})
            
            # 平移通道
            trans_data = bytearray()
            for rts in track.frames:
                trans_data.extend(struct.pack('<fff', *rts.translation))
            
            trans_bv_idx = self._add_buffer_view(binary_data, buffer_views, trans_data)
            trans_accessor_idx = len(accessors)
            self._add_accessor(accessors, trans_bv_idx, self.FLOAT, animation.frame_count, "VEC3")
            
            sampler_idx = len(samplers)
            samplers.append({
                "input": time_accessor_idx,
                "output": trans_accessor_idx,
                "interpolation": "LINEAR"
            })
            channels.append({
                "sampler": sampler_idx,
                "target": {"node": node_idx, "path": "translation"}
            })
            
            # 旋转通道
            rot_data = bytearray()
            for rts in track.frames:
                # glTF使用xyzw顺序
                rot_data.extend(struct.pack('<ffff', *rts.rotation))
            
            rot_bv_idx = self._add_buffer_view(binary_data, buffer_views, rot_data)
            rot_accessor_idx = len(accessors)
            self._add_accessor(accessors, rot_bv_idx, self.FLOAT, animation.frame_count, "VEC4")
            
            sampler_idx = len(samplers)
            samplers.append({
                "input": time_accessor_idx,
                "output": rot_accessor_idx,
                "interpolation": "LINEAR"
            })
            channels.append({
                "sampler": sampler_idx,
                "target": {"node": node_idx, "path": "rotation"}
            })
            
            # 缩放通道
            scale_data = bytearray()
            for rts in track.frames:
                scale_data.extend(struct.pack('<fff', *rts.scale))
            
            scale_bv_idx = self._add_buffer_view(binary_data, buffer_views, scale_data)
            scale_accessor_idx = len(accessors)
            self._add_accessor(accessors, scale_bv_idx, self.FLOAT, animation.frame_count, "VEC3")
            
            sampler_idx = len(samplers)
            samplers.append({
                "input": time_accessor_idx,
                "output": scale_accessor_idx,
                "interpolation": "LINEAR"
            })
            channels.append({
                "sampler": sampler_idx,
                "target": {"node": node_idx, "path": "scale"}
            })
        
        # 构建glTF JSON
        gltf = {
            "asset": {"version": "2.0", "generator": "Zongine Converter"},
            "scene": 0,
            "scenes": [{"nodes": list(range(len(nodes)))}],
            "nodes": nodes,
            "animations": [{
                "name": animation.name,
                "channels": channels,
                "samplers": samplers
            }],
            "accessors": accessors,
            "bufferViews": buffer_views,
            "buffers": [{"byteLength": len(binary_data)}],
            "extras": {
                "frameCount": animation.frame_count,
                "frameLength": animation.frame_length,
                "fps": fps
            }
        }
        
        return {"json": gltf, "binary": binary_data}
    
    def _add_buffer_view(self, binary_data: bytearray, buffer_views: List[Dict],
                        data: bytearray, target: int = None) -> int:
        """添加缓冲区视图"""
        offset = len(binary_data)
        binary_data.extend(data)
        
        bv = {"buffer": 0, "byteOffset": offset, "byteLength": len(data)}
        if target:
            bv["target"] = target
        
        buffer_views.append(bv)
        return len(buffer_views) - 1
    
    def _add_accessor(self, accessors: List[Dict], buffer_view: int, component_type: int,
                     count: int, accessor_type: str, min_vals: List = None, 
                     max_vals: List = None) -> int:
        """添加访问器"""
        accessor = {
            "bufferView": buffer_view,
            "componentType": component_type,
            "count": count,
            "type": accessor_type
        }
        if min_vals:
            accessor["min"] = min_vals
        if max_vals:
            accessor["max"] = max_vals
        
        accessors.append(accessor)
        return len(accessors) - 1
    
    def _save_gltf(self, gltf_data: Dict, output_path: str) -> bool:
        """保存glTF文件"""
        json_data = gltf_data["json"]
        binary_data = gltf_data["binary"]
        
        if output_path.lower().endswith('.glb'):
            return self._save_glb(json_data, binary_data, output_path)
        else:
            return self._save_gltf_json(json_data, binary_data, output_path)
    
    def _save_gltf_json(self, json_data: Dict, binary_data: bytearray, output_path: str) -> bool:
        """保存为glTF + bin格式"""
        bin_path = output_path.rsplit('.', 1)[0] + '.bin'
        json_data["buffers"][0]["uri"] = os.path.basename(bin_path)
        
        with open(bin_path, 'wb') as f:
            f.write(binary_data)
        
        with open(output_path, 'w', encoding='utf-8') as f:
            json.dump(json_data, f, indent=2, ensure_ascii=False)
        
        print(f"已保存: {output_path}")
        return True
    
    def _save_glb(self, json_data: Dict, binary_data: bytearray, output_path: str) -> bool:
        """保存为GLB格式"""
        json_str = json.dumps(json_data, separators=(',', ':')).encode('utf-8')
        
        # 4字节对齐
        json_padding = (4 - len(json_str) % 4) % 4
        json_str += b' ' * json_padding
        
        bin_padding = (4 - len(binary_data) % 4) % 4
        binary_data += b'\x00' * bin_padding
        
        total_length = 12 + 8 + len(json_str) + 8 + len(binary_data)
        
        with open(output_path, 'wb') as f:
            # Header
            f.write(struct.pack('<III', 0x46546C67, 2, total_length))
            # JSON chunk
            f.write(struct.pack('<II', len(json_str), 0x4E4F534A))
            f.write(json_str)
            # BIN chunk
            f.write(struct.pack('<II', len(binary_data), 0x004E4942))
            f.write(binary_data)
        
        print(f"已保存: {output_path}")
        return True


# ============= 批量转换器 =============

class BatchConverter:
    """批量转换器"""
    
    def __init__(self):
        self.mesh_parser = MeshParser()
        self.anim_parser = AnimationParser()
        self.skeleton_parser = SkeletonParser()
        self.exporter = GltfExporter()
    
    def convert_mesh(self, input_path: str, output_path: str) -> bool:
        """转换单个Mesh文件"""
        print(f"\n转换Mesh: {input_path}")
        mesh = self.mesh_parser.parse(input_path)
        if not mesh:
            return False
        return self.exporter.export_mesh(mesh, output_path)
    
    def convert_animation(self, input_path: str, output_path: str) -> bool:
        """转换单个动画文件"""
        print(f"\n转换动画: {input_path}")
        animation = self.anim_parser.parse(input_path)
        if not animation:
            return False
        return self.exporter.export_animation(animation, output_path)
    
    def convert_skeleton(self, input_path: str, output_path: str) -> bool:
        """转换单个骨架文件"""
        print(f"\n转换骨架: {input_path}")
        skeleton = self.skeleton_parser.parse(input_path)
        if not skeleton:
            return False
        return self.exporter.export_skeleton(skeleton, output_path)
    
    def convert_all(self, input_dir: str, output_dir: str) -> Tuple[int, int]:
        """批量转换目录中的所有文件"""
        input_path = Path(input_dir)
        output_path = Path(output_dir)
        
        success_count = 0
        fail_count = 0
        
        # 转换Mesh文件
        for mesh_file in input_path.rglob('*.mesh'):
            rel_path = mesh_file.relative_to(input_path)
            out_file = output_path / rel_path.with_suffix('.gltf')
            out_file.parent.mkdir(parents=True, exist_ok=True)
            
            if self.convert_mesh(str(mesh_file), str(out_file)):
                success_count += 1
            else:
                fail_count += 1
        
        # 转换动画文件
        for ani_file in input_path.rglob('*.ani'):
            rel_path = ani_file.relative_to(input_path)
            out_file = output_path / rel_path.with_suffix('.gltf')
            out_file.parent.mkdir(parents=True, exist_ok=True)
            
            if self.convert_animation(str(ani_file), str(out_file)):
                success_count += 1
            else:
                fail_count += 1
        
        return success_count, fail_count


def main():
    """主函数"""
    if len(sys.argv) < 2:
        print("Zongine to glTF Converter")
        print("\n使用方法:")
        print("  python zongine_to_gltf_converter.py mesh <input.mesh> <output.gltf>")
        print("  python zongine_to_gltf_converter.py animation <input.ani> <output.gltf>")
        print("  python zongine_to_gltf_converter.py skeleton <input.txt> <output.gltf>")
        print("  python zongine_to_gltf_converter.py batch <input_dir> <output_dir>")
        sys.exit(1)
    
    command = sys.argv[1].lower()
    converter = BatchConverter()
    
    if command == "mesh" and len(sys.argv) >= 3:
        input_file = sys.argv[2]
        output_file = sys.argv[3] if len(sys.argv) > 3 else input_file.rsplit('.', 1)[0] + '.gltf'
        
        if converter.convert_mesh(input_file, output_file):
            print("\n转换成功!")
        else:
            print("\n转换失败!")
            sys.exit(1)
    
    elif command == "animation" and len(sys.argv) >= 3:
        input_file = sys.argv[2]
        output_file = sys.argv[3] if len(sys.argv) > 3 else input_file.rsplit('.', 1)[0] + '.gltf'
        
        if converter.convert_animation(input_file, output_file):
            print("\n转换成功!")
        else:
            print("\n转换失败!")
            sys.exit(1)
    
    elif command == "skeleton" and len(sys.argv) >= 3:
        input_file = sys.argv[2]
        output_file = sys.argv[3] if len(sys.argv) > 3 else input_file.rsplit('.', 1)[0] + '.gltf'
        
        if converter.convert_skeleton(input_file, output_file):
            print("\n转换成功!")
        else:
            print("\n转换失败!")
            sys.exit(1)
    
    elif command == "batch" and len(sys.argv) >= 4:
        input_dir = sys.argv[2]
        output_dir = sys.argv[3]
        
        success, fail = converter.convert_all(input_dir, output_dir)
        print(f"\n批量转换完成: 成功 {success}, 失败 {fail}")
        
        if fail > 0:
            sys.exit(1)
    
    else:
        print("无效的命令或参数")
        sys.exit(1)


if __name__ == "__main__":
    main()
