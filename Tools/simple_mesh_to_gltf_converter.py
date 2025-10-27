#!/usr/bin/env python3
"""
简化版 Zongine Mesh to glTF Converter
不依赖外部库，直接生成glTF JSON和二进制数据

使用方法:
python simple_mesh_to_gltf_converter.py input_mesh_file.mesh output_file.gltf

特点:
- 无外部依赖
- 直接生成glTF 2.0格式
- 支持顶点、面、骨骼信息转换

作者: Zongine Team
"""

import sys
import struct
import os
import json
import base64
from typing import List, Tuple, Dict, Optional, Any
import math


class SimpleMeshToGLTFConverter:
    """简化版Mesh到glTF转换器"""
    
    def __init__(self):
        self.reset()
    
    def reset(self):
        """重置转换器状态"""
        self.vertices = []
        self.indices = []
        self.bones = []
        self.sockets = []
        self.subset_indices = []
        
        # 解析出的数据
        self.vertex_count = 0
        self.face_count = 0
        self.subset_count = 0
        self.bone_count = 0
        self.socket_count = 0
        
        # 顶点属性标志
        self.has_positions = False
        self.has_normals = False
        self.has_tangents = False
        self.has_colors = False
        self.has_texcoords = False
        self.has_skin_data = False
        
    def convert_mesh_to_gltf(self, input_file: str, output_file: str) -> bool:
        """主转换函数"""
        print(f"开始转换: {input_file} -> {output_file}")
        
        # 加载mesh文件
        if not self._load_mesh_file(input_file):
            print("加载mesh文件失败")
            return False
        
        # 构建glTF
        gltf_data = self._build_gltf_json()
        if not gltf_data:
            print("构建glTF失败")
            return False
        
        # 保存文件
        try:
            if output_file.lower().endswith('.glb'):
                return self._save_glb(output_file, gltf_data)
            else:
                return self._save_gltf(output_file, gltf_data)
        except Exception as e:
            print(f"保存文件失败: {e}")
            return False
    
    def _load_mesh_file(self, filename: str) -> bool:
        """加载Zongine mesh文件"""
        try:
            with open(filename, 'rb') as f:
                return self._parse_mesh_file(f)
        except Exception as e:
            print(f"读取文件失败: {e}")
            return False
    
    def _parse_mesh_file(self, f) -> bool:
        """解析mesh文件"""
        # 读取文件头
        header = self._read_file_header(f)
        if not header:
            return False
        
        print(f"解析到顶点数: {header['vertex_count']}")
        print(f"解析到面数: {header['face_count']}")
        print(f"解析到材质组数: {header['subset_count']}")
        
        # 检查文件大小，给出警告
        if header['vertex_count'] > 50000:
            print(f"警告：文件较大，转换可能需要一些时间...")
        
        # 解析数据块
        return self._parse_data_blocks(f, header)
    
    def _read_file_header(self, f) -> Optional[Dict]:
        """读取文件头"""
        try:
            # 直接从文件开始读取，跳过通用头部（84字节全为0）
            f.seek(84)
            
            # 读取版本头部
            file_mask = struct.unpack('<I', f.read(4))[0]
            if file_mask != 0x4d455348:  # "MESH"
                print("错误：无效的mesh文件格式")
                return None
            
            block_length = struct.unpack('<I', f.read(4))[0]
            version = struct.unpack('<I', f.read(4))[0]
            
            # 跳过动画块指针
            animation_block = struct.unpack('<I', f.read(4))[0]
            
            # 跳过扩展块指针 (10个DWORD = 40字节)
            f.seek(40, 1)
            
            mesh_count = struct.unpack('<I', f.read(4))[0]
            
            # 读取mesh头部
            vertex_count = struct.unpack('<I', f.read(4))[0]
            face_count = struct.unpack('<I', f.read(4))[0]
            subset_count = struct.unpack('<I', f.read(4))[0]
            
            # 读取数据块偏移 (30个DWORD)
            blocks = {}
            block_names = [
                'PositionBlock', 'NormalBlock', 'DiffuseBlock', 'TextureUVW1Block',
                'TextureUVW2Block', 'TextureUVW3Block', 'FacesIndexBlock', 'SubsetIndexBlock',
                'SkinInfoBlock', 'LODInfoBlock', 'FlexibleBodyBlock', 'BBoxBlock',
                'BlendMeshBlock', 'TangentBlock', 'VetexRemapBlock', 'TextureUVW9Block',
                'EmissiveBlock'
            ]
            
            for i in range(30):
                offset = struct.unpack('<I', f.read(4))[0]
                if i < len(block_names):
                    blocks[block_names[i]] = offset
                else:
                    blocks[f'ExtendBlock{i-17}'] = offset
            
            self.vertex_count = vertex_count
            self.face_count = face_count
            self.subset_count = subset_count
            
            return {
                'version': version,
                'vertex_count': vertex_count,
                'face_count': face_count,
                'subset_count': subset_count,
                'blocks': blocks
            }
            
        except Exception as e:
            print(f"读取文件头失败: {e}")
            return None
    
    def _parse_data_blocks(self, f, header: Dict) -> bool:
        """解析数据块"""
        blocks = header['blocks']
        
        # 初始化顶点数据
        self.vertices = [{} for _ in range(self.vertex_count)]
        
        # 解析位置数据
        if blocks.get('PositionBlock', 0) > 0:
            if not self._parse_positions(f, blocks['PositionBlock']):
                return False
        
        # 解析法线数据
        if blocks.get('NormalBlock', 0) > 0:
            if not self._parse_normals(f, blocks['NormalBlock']):
                return False
        
        # 解析切线数据
        if blocks.get('TangentBlock', 0) > 0:
            if not self._parse_tangents(f, blocks['TangentBlock']):
                return False
        
        # 解析颜色数据
        if blocks.get('DiffuseBlock', 0) > 0:
            if not self._parse_colors(f, blocks['DiffuseBlock']):
                return False
        
        # 解析纹理坐标
        if blocks.get('TextureUVW1Block', 0) > 0:
            if not self._parse_texcoords(f, blocks['TextureUVW1Block']):
                return False
        
        # 解析索引数据
        if blocks.get('FacesIndexBlock', 0) > 0:
            if not self._parse_indices(f, blocks['FacesIndexBlock']):
                return False
        
        # 解析Subset索引数据
        if blocks.get('SubsetIndexBlock', 0) > 0:
            if not self._parse_subset_indices(f, blocks['SubsetIndexBlock']):
                return False
        
        # 解析骨骼信息
        if blocks.get('SkinInfoBlock', 0) > 0:
            if not self._parse_skin_info(f, blocks['SkinInfoBlock'], header['version'], blocks):
                return False
        
        return True
    
    def _parse_positions(self, f, offset: int) -> bool:
        """解析顶点位置"""
        try:
            f.seek(offset)
            print(f"解析位置数据... ({self.vertex_count} 个顶点)")
            
            # 批量读取以提高效率
            batch_size = min(1000, self.vertex_count)
            for batch_start in range(0, self.vertex_count, batch_size):
                batch_end = min(batch_start + batch_size, self.vertex_count)
                batch_data = f.read((batch_end - batch_start) * 12)
                
                for i in range(batch_end - batch_start):
                    idx = batch_start + i
                    offset_in_batch = i * 12
                    x, y, z = struct.unpack('<fff', batch_data[offset_in_batch:offset_in_batch + 12])
                    self.vertices[idx]['position'] = [x, y, z]
                
                if batch_start % 10000 == 0:
                    print(f"  处理进度: {batch_end}/{self.vertex_count}")
            
            self.has_positions = True
            print("解析位置数据完成")
            return True
        except Exception as e:
            print(f"解析位置数据失败: {e}")
            return False
    
    def _parse_normals(self, f, offset: int) -> bool:
        """解析法线"""
        try:
            f.seek(offset)
            for i in range(self.vertex_count):
                x, y, z = struct.unpack('<fff', f.read(12))
                self.vertices[i]['normal'] = [x, y, z]
            
            self.has_normals = True
            print("解析法线数据完成")
            return True
        except Exception as e:
            print(f"解析法线数据失败: {e}")
            return False
    
    def _parse_tangents(self, f, offset: int) -> bool:
        """解析切线"""
        try:
            f.seek(offset)
            for i in range(self.vertex_count):
                x, y, z, w = struct.unpack('<ffff', f.read(16))
                self.vertices[i]['tangent'] = [x, y, z, w]
            
            self.has_tangents = True
            print("解析切线数据完成")
            return True
        except Exception as e:
            print(f"解析切线数据失败: {e}")
            return False
    
    def _parse_colors(self, f, offset: int) -> bool:
        """解析顶点颜色"""
        try:
            f.seek(offset)
            for i in range(self.vertex_count):
                # XMCOLOR格式 (BGRA 8-bit each)
                color_data = struct.unpack('<I', f.read(4))[0]
                r = ((color_data >> 16) & 0xFF) / 255.0
                g = ((color_data >> 8) & 0xFF) / 255.0
                b = (color_data & 0xFF) / 255.0
                a = ((color_data >> 24) & 0xFF) / 255.0
                self.vertices[i]['color'] = [r, g, b, a]
            
            self.has_colors = True
            print("解析颜色数据完成")
            return True
        except Exception as e:
            print(f"解析颜色数据失败: {e}")
            return False
    
    def _parse_texcoords(self, f, offset: int) -> bool:
        """解析纹理坐标"""
        try:
            f.seek(offset)
            for i in range(self.vertex_count):
                # 读取3个float但只使用前2个 (XMFLOAT3->XMFLOAT2)
                u, v, _ = struct.unpack('<fff', f.read(12))
                self.vertices[i]['texcoord'] = [u, v]
            
            self.has_texcoords = True
            print("解析纹理坐标完成")
            return True
        except Exception as e:
            print(f"解析纹理坐标失败: {e}")
            return False
    
    def _parse_indices(self, f, offset: int) -> bool:
        """解析索引数据"""
        try:
            f.seek(offset)
            index_count = self.face_count * 3
            print(f"解析索引数据... ({index_count} 个索引)")
            
            # 一次性读取所有索引数据
            index_data = f.read(index_count * 4)
            self.indices = list(struct.unpack(f'<{index_count}I', index_data))
            
            print(f"解析索引数据完成，共{len(self.indices)}个索引")
            return True
        except Exception as e:
            print(f"解析索引数据失败: {e}")
            return False
    
    def _parse_subset_indices(self, f, offset: int) -> bool:
        """解析Subset索引数据"""
        try:
            f.seek(offset)
            print(f"解析Subset索引数据... ({self.face_count} 个面)")
            
            # 先尝试读取4字节看是否合理，如果不合理则尝试2字节
            current_pos = f.tell()
            
            # 尝试DWORD格式
            try:
                subset_data = f.read(self.face_count * 4)
                if len(subset_data) == self.face_count * 4:
                    test_indices = list(struct.unpack(f'<{self.face_count}I', subset_data))
                    # 检查数据是否合理（材质ID应该是小的正数）
                    max_id = max(test_indices) if test_indices else 0
                    if max_id < 1000:  # 合理的材质ID范围
                        self.subset_indices = test_indices
                        print(f"解析Subset索引完成（DWORD格式），共{len(self.subset_indices)}个面材质ID")
                        unique_subsets = set(self.subset_indices)
                        print(f"发现 {len(unique_subsets)} 个不同的材质组: {sorted(unique_subsets)}")
                        return True
            except:
                pass
            
            # 回到原位置，尝试WORD格式
            f.seek(current_pos)
            try:
                subset_data = f.read(self.face_count * 2)
                if len(subset_data) == self.face_count * 2:
                    self.subset_indices = list(struct.unpack(f'<{self.face_count}H', subset_data))
                    print(f"解析Subset索引完成（WORD格式），共{len(self.subset_indices)}个面材质ID")
                    unique_subsets = set(self.subset_indices)
                    print(f"发现 {len(unique_subsets)} 个不同的材质组: {sorted(unique_subsets)}")
                    return True
            except:
                pass
            
            print("无法解析Subset索引数据，跳过subset处理")
            self.subset_indices = []
            return True  # 不算错误，只是没有subset数据
            
        except Exception as e:
            print(f"解析Subset索引数据失败: {e}")
            self.subset_indices = []
            return True  # 不算错误
    
    def _create_subset_primitives(self, binary_data: bytearray, accessors: list, buffer_views: list):
        """根据subset信息创建多个primitives和materials"""
        print("创建subset primitives...")
        
        if not self.subset_indices or len(self.subset_indices) != self.face_count:
            # 如果没有subset数据，创建单一primitive
            print("没有subset数据，创建单一primitive")
            return [{
                "attributes": {},
                "indices": len(accessors) - 1,  # 索引访问器
                "material": 0
            }], [{
                "pbrMetallicRoughness": {
                    "baseColorFactor": [1.0, 1.0, 1.0, 1.0],
                    "metallicFactor": 0.0,
                    "roughnessFactor": 1.0
                },
                "name": "DefaultMaterial"
            }]
        
        # 按材质ID分组索引
        from collections import defaultdict
        subset_groups = defaultdict(list)
        
        print(f"处理{len(self.subset_indices)}个面的材质分组...")
        for face_idx, material_id in enumerate(self.subset_indices):
            # 每个面有3个索引
            start_idx = face_idx * 3
            if start_idx + 2 < len(self.indices):
                subset_groups[material_id].extend([
                    self.indices[start_idx],
                    self.indices[start_idx + 1],
                    self.indices[start_idx + 2]
                ])
            
            # 防止处理时间过长，每处理1000个面打印一次进度
            if face_idx % 1000 == 0 and face_idx > 0:
                print(f"  处理进度: {face_idx}/{len(self.subset_indices)}")
        
        print(f"发现{len(subset_groups)}个材质组")
        
        primitives = []
        materials = []
        
        # 限制材质数量防止过多
        max_materials = 10
        material_count = 0
        
        # 为每个材质组创建primitive
        for material_id in sorted(subset_groups.keys()):
            if material_count >= max_materials:
                print(f"达到最大材质数量限制{max_materials}，停止处理")
                break
                
            indices = subset_groups[material_id]
            print(f"处理材质{material_id}，包含{len(indices)}个索引")
            
            # 创建这个subset的索引缓冲区
            indices_data = bytearray()
            for idx in indices:
                indices_data.extend(struct.pack('<I', idx))
            
            # 添加到总缓冲区
            indices_offset = len(binary_data)
            binary_data.extend(indices_data)
            
            # 创建缓冲区视图
            buffer_views.append({
                "buffer": 0,
                "byteOffset": indices_offset,
                "byteLength": len(indices_data),
                "target": 34963  # ELEMENT_ARRAY_BUFFER
            })
            
            # 创建访问器
            indices_accessor_idx = len(accessors)
            accessors.append({
                "bufferView": len(buffer_views) - 1,
                "componentType": 5125,  # UNSIGNED_INT
                "count": len(indices),
                "type": "SCALAR"
            })
            
            # 创建primitive
            primitives.append({
                "attributes": {},
                "indices": indices_accessor_idx,
                "material": len(materials)  # 使用材质数组的索引而不是原始材质ID
            })
            
            # 创建材质
            materials.append({
                "pbrMetallicRoughness": {
                    "baseColorFactor": [1.0, 1.0, 1.0, 1.0],
                    "metallicFactor": 0.0,
                    "roughnessFactor": 1.0
                },
                "name": f"Material_{material_id}"
            })
            
            material_count += 1
        
        print(f"创建了{len(primitives)}个primitives和{len(materials)}个materials")
        return primitives, materials
    
    def _parse_skin_info(self, f, offset: int, version: int, blocks: Dict) -> bool:
        """解析骨骼蒙皮信息"""
        try:
            f.seek(offset)
            
            # 读取骨骼数量
            bone_count = struct.unpack('<I', f.read(4))[0]
            self.bone_count = bone_count
            
            print(f"解析骨骼数据，骨骼数量: {bone_count}")
            
            # 版本标志位检查
            has_px_pose = version & (0x80000000 >> 5)  # VERSION_BIT_ADDITIVE_PX_POSE
            
            # 根据BBoxBlock偏移判断是否有边界框数据
            has_bbox = blocks.get('BBoxBlock', 0) > 0
            print(f"版本标志位 - has_px_pose: {bool(has_px_pose)}, has_bbox: {has_bbox}")
            
            # 初始化顶点骨骼数据
            for i in range(self.vertex_count):
                self.vertices[i]['joints'] = [255, 255, 255, 255]
                self.vertices[i]['weights'] = [0.0, 0.0, 0.0, 0.0]
            
            self.bones = []
            
            for bone_idx in range(bone_count):
                if bone_idx % 10 == 0:
                    print(f"  处理骨骼: {bone_idx + 1}/{bone_count}")
                
                bone = {}
                
                # 读取骨骼名称 (60字节，前30字节是名称，后30字节跳过)
                bone_name = f.read(30).decode('ascii', errors='ignore').rstrip('\x00').lower()
                f.seek(30, 1)  # 跳过填充
                
                bone['name'] = bone_name
                
                # 读取子骨骼数量
                child_count = struct.unpack('<I', f.read(4))[0]
                print(f"    骨骼 {bone_name} 的子骨骼数量: {child_count}")
                
                bone['children'] = []
                
                # 读取子骨骼名称 (每个NAME_STRING是30字节)
                if child_count > 0:
                    if child_count > 100:  # 合理的上限检查
                        print(f"    警告：子骨骼数量异常: {child_count}，可能文件解析错误")
                        return False
                    
                    child_names_data = f.read(child_count * 30)
                    for i in range(child_count):
                        start_pos = i * 30
                        child_name = child_names_data[start_pos:start_pos + 30].decode('ascii', errors='ignore').rstrip('\x00').lower()
                        bone['children'].append(child_name)
                        print(f"      子骨骼: {child_name}")
                
                # 读取变换矩阵
                # mOffset (16 floats)
                offset_matrix = list(struct.unpack('<16f', f.read(64)))
                bone['offset_matrix'] = offset_matrix
                
                # mOffset2Parent (16 floats)
                parent_matrix = list(struct.unpack('<16f', f.read(64)))
                bone['parent_matrix'] = parent_matrix
                
                # mInvPxPose (如果版本支持)
                if has_px_pose:
                    inv_pose_matrix = list(struct.unpack('<16f', f.read(64)))
                    bone['inv_pose_matrix'] = inv_pose_matrix
                
                # 读取受影响的顶点数量
                ref_vertex_count = struct.unpack('<I', f.read(4))[0]
                
                # 读取顶点索引和权重
                if ref_vertex_count > 0:
                    # 批量读取顶点索引
                    indices_data = f.read(ref_vertex_count * 4)
                    vertex_indices = list(struct.unpack(f'<{ref_vertex_count}I', indices_data))
                    
                    # 批量读取权重
                    weights_data = f.read(ref_vertex_count * 4)
                    weights = list(struct.unpack(f'<{ref_vertex_count}f', weights_data))
                    
                    # 将权重信息应用到顶点
                    for i, vertex_idx in enumerate(vertex_indices):
                        if vertex_idx < self.vertex_count:
                            weight = weights[i]
                            vertex = self.vertices[vertex_idx]
                            
                            # 找到空的权重槽
                            for j in range(4):
                                if vertex['weights'][j] == 0.0:  # 找到空槽
                                    vertex['joints'][j] = bone_idx
                                    vertex['weights'][j] = weight
                                    break
                
                # 读取边界框 (如果有)
                if has_bbox:
                    f.seek(64, 1)  # 跳过变换矩阵
                    bbox = list(struct.unpack('<6f', f.read(24)))  # min_xyz, max_xyz
                    bone['bbox'] = bbox
                    f.seek(4, 1)  # 跳过BOOL
                
                self.bones.append(bone)
            
            # 读取Socket信息
            try:
                socket_count = struct.unpack('<I', f.read(4))[0]
                self.socket_count = socket_count
                
                self.sockets = []
                for _ in range(socket_count):
                    socket = {}
                    socket_name = f.read(30).decode('ascii', errors='ignore').rstrip('\x00').lower()
                    parent_name = f.read(30).decode('ascii', errors='ignore').rstrip('\x00').lower()
                    socket_matrix = list(struct.unpack('<16f', f.read(64)))
                    
                    socket['name'] = socket_name
                    socket['parent'] = parent_name
                    socket['matrix'] = socket_matrix
                    self.sockets.append(socket)
            except:
                self.socket_count = 0
                self.sockets = []
            
            self.has_skin_data = bone_count > 0
            print(f"解析蒙皮数据完成，骨骼数: {bone_count}, Socket数: {self.socket_count}")
            return True
            
        except Exception as e:
            import traceback
            print(f"解析蒙皮数据失败: {e}")
            print(f"详细错误信息:")
            traceback.print_exc()
            return False
    
    def _build_gltf_json(self) -> Optional[Dict]:
        """构建glTF JSON数据结构"""
        try:
            # 构建二进制数据和访问器
            binary_data, accessors, buffer_views = self._build_binary_data()
            
            # 基础glTF结构
            gltf = {
                "asset": {
                    "version": "2.0",
                    "generator": "Zongine Mesh to glTF Converter"
                },
                "scenes": [{"nodes": [0]}],
                "scene": 0,
                "nodes": [{"mesh": 0}],
                "meshes": [{}],  # 将在后面填充primitives
                "materials": [{
                    "pbrMetallicRoughness": {
                        "baseColorFactor": [1.0, 1.0, 1.0, 1.0],
                        "metallicFactor": 0.0,
                        "roughnessFactor": 1.0
                    },
                    "name": "DefaultMaterial"
                }],
                "accessors": accessors,
                "bufferViews": buffer_views,
                "buffers": [{
                    "byteLength": len(binary_data)
                }]
            }
            
            # 创建基于subset的primitives和materials
            primitives, materials = self._create_subset_primitives(binary_data, accessors, buffer_views)
            gltf["meshes"][0]["primitives"] = primitives
            gltf["materials"] = materials
            
            # 设置顶点属性到所有primitives
            accessor_index = 0
            
            for primitive in primitives:
                if self.has_positions:
                    primitive["attributes"]["POSITION"] = accessor_index
                
            vertex_accessor_index = accessor_index
            if self.has_positions:
                vertex_accessor_index += 1
            
            # 为所有primitive设置共享的顶点属性
            current_accessor = vertex_accessor_index
            for primitive in primitives:
                if self.has_normals:
                    primitive["attributes"]["NORMAL"] = current_accessor
            if self.has_normals:
                current_accessor += 1
            
            for primitive in primitives:
                if self.has_texcoords:
                    primitive["attributes"]["TEXCOORD_0"] = current_accessor
            if self.has_texcoords:
                current_accessor += 1
            
            for primitive in primitives:
                if self.has_colors:
                    primitive["attributes"]["COLOR_0"] = current_accessor
            if self.has_colors:
                current_accessor += 1
            
            if self.has_skin_data:
                for primitive in primitives:
                    primitive["attributes"]["JOINTS_0"] = current_accessor
                    primitive["attributes"]["WEIGHTS_0"] = current_accessor + 1
                
                # 添加蒙皮数据
                skin_data = self._build_skin_data(binary_data, accessors, buffer_views)
                gltf["skins"] = [skin_data["skin"]]
                gltf["nodes"][0]["skin"] = 0
                
                # 添加骨骼节点
                gltf["nodes"].extend(skin_data["bone_nodes"])
                
                # 添加Socket节点
                for socket in self.sockets:
                    socket_node = {
                        "name": f"Socket_{socket['name']}",  # 使用Socket_前缀便于识别
                        "matrix": socket['matrix'],
                        "extras": {
                            "parent": socket['parent']
                        }
                    }
                    gltf["nodes"].append(socket_node)
                    
                    # Socket应该作为独立节点添加到场景根部，便于查找
                    # 但不直接添加到scene nodes，而是让它们作为骨骼的子节点
                    # 根据parent_name找到对应的父骨骼并建立层次关系
                    parent_name = socket.get('parent', '').lower()
                    if parent_name:
                        # 找到父骨骼节点
                        parent_node_idx = None
                        for i, bone_node in enumerate(skin_data["bone_nodes"]):
                            if bone_node.get("name", "").lower() == parent_name:
                                parent_node_idx = len(gltf["nodes"]) - len(skin_data["bone_nodes"]) + i - 1
                                break
                        
                        # 将Socket作为父骨骼的子节点
                        if parent_node_idx is not None and parent_node_idx < len(gltf["nodes"]):
                            parent_node = gltf["nodes"][parent_node_idx]
                            if "children" not in parent_node:
                                parent_node["children"] = []
                            parent_node["children"].append(len(gltf["nodes"]) - 1)
                
                # 更新访问器和缓冲区视图
                gltf["accessors"] = accessors
                gltf["bufferViews"] = buffer_views
                gltf["buffers"][0]["byteLength"] = len(binary_data)
            
            return {"json": gltf, "binary": binary_data}
            
        except Exception as e:
            print(f"构建glTF JSON失败: {e}")
            return None
    
    def _build_binary_data(self) -> Tuple[bytearray, List[Dict], List[Dict]]:
        """构建二进制数据和访问器"""
        binary_data = bytearray()
        accessors = []
        buffer_views = []
        
        def add_buffer_data(data: bytes, target: Optional[int] = None) -> int:
            """添加缓冲区数据并返回BufferView索引"""
            offset = len(binary_data)
            binary_data.extend(data)
            
            buffer_view = {
                "buffer": 0,
                "byteOffset": offset,
                "byteLength": len(data)
            }
            if target is not None:
                buffer_view["target"] = target
            
            buffer_views.append(buffer_view)
            return len(buffer_views) - 1
        
        def add_accessor(buffer_view_idx: int, component_type: int, count: int, 
                        accessor_type: str, min_vals: List[float] = None, 
                        max_vals: List[float] = None) -> int:
            """添加访问器并返回索引"""
            accessor = {
                "bufferView": buffer_view_idx,
                "componentType": component_type,
                "count": count,
                "type": accessor_type
            }
            if min_vals is not None:
                accessor["min"] = min_vals
            if max_vals is not None:
                accessor["max"] = max_vals
            
            accessors.append(accessor)
            return len(accessors) - 1
        
        # 常量定义
        FLOAT = 5126
        UNSIGNED_INT = 5125
        UNSIGNED_SHORT = 5123
        ARRAY_BUFFER = 34962
        ELEMENT_ARRAY_BUFFER = 34963
        
        # 位置数据
        if self.has_positions:
            pos_data = bytearray()
            min_pos = [float('inf')] * 3
            max_pos = [float('-inf')] * 3
            
            for vertex in self.vertices:
                pos = vertex.get('position', [0.0, 0.0, 0.0])
                pos_data.extend(struct.pack('<fff', *pos))
                
                for i in range(3):
                    min_pos[i] = min(min_pos[i], pos[i])
                    max_pos[i] = max(max_pos[i], pos[i])
            
            bv_idx = add_buffer_data(bytes(pos_data), ARRAY_BUFFER)
            add_accessor(bv_idx, FLOAT, self.vertex_count, "VEC3", min_pos, max_pos)
        
        # 法线数据
        if self.has_normals:
            norm_data = bytearray()
            for vertex in self.vertices:
                normal = vertex.get('normal', [0.0, 1.0, 0.0])
                norm_data.extend(struct.pack('<fff', *normal))
            
            bv_idx = add_buffer_data(bytes(norm_data), ARRAY_BUFFER)
            add_accessor(bv_idx, FLOAT, self.vertex_count, "VEC3")
        
        # 纹理坐标
        if self.has_texcoords:
            tex_data = bytearray()
            for vertex in self.vertices:
                texcoord = vertex.get('texcoord', [0.0, 0.0])
                tex_data.extend(struct.pack('<ff', *texcoord))
            
            bv_idx = add_buffer_data(bytes(tex_data), ARRAY_BUFFER)
            add_accessor(bv_idx, FLOAT, self.vertex_count, "VEC2")
        
        # 颜色数据
        if self.has_colors:
            color_data = bytearray()
            for vertex in self.vertices:
                color = vertex.get('color', [1.0, 1.0, 1.0, 1.0])
                color_data.extend(struct.pack('<ffff', *color))
            
            bv_idx = add_buffer_data(bytes(color_data), ARRAY_BUFFER)
            add_accessor(bv_idx, FLOAT, self.vertex_count, "VEC4")
        
        # 骨骼关节和权重
        if self.has_skin_data:
            joints_data = bytearray()
            weights_data = bytearray()
            
            for vertex in self.vertices:
                joints = vertex.get('joints', [0, 0, 0, 0])
                weights = vertex.get('weights', [0.0, 0.0, 0.0, 0.0])
                
                # 归一化权重
                total_weight = sum(weights)
                if total_weight > 0:
                    weights = [w / total_weight for w in weights]
                
                joints_data.extend(struct.pack('<HHHH', *[min(j, 65535) for j in joints]))
                weights_data.extend(struct.pack('<ffff', *weights))
            
            joints_bv_idx = add_buffer_data(bytes(joints_data), ARRAY_BUFFER)
            weights_bv_idx = add_buffer_data(bytes(weights_data), ARRAY_BUFFER)
            
            add_accessor(joints_bv_idx, UNSIGNED_SHORT, self.vertex_count, "VEC4")
            add_accessor(weights_bv_idx, FLOAT, self.vertex_count, "VEC4")
        
        # 索引数据
        indices_data = bytearray()
        for index in self.indices:
            indices_data.extend(struct.pack('<I', index))
        
        indices_bv_idx = add_buffer_data(bytes(indices_data), ELEMENT_ARRAY_BUFFER)
        add_accessor(indices_bv_idx, UNSIGNED_INT, len(self.indices), "SCALAR")
        
        return binary_data, accessors, buffer_views
    
    def _build_skin_data(self, binary_data: bytearray, accessors: List[Dict], 
                        buffer_views: List[Dict]) -> Dict:
        """构建蒙皮数据"""
        if not self.bones:
            return {}
        
        # 创建骨骼节点
        bone_nodes = []
        for i, bone in enumerate(self.bones):
            bone_node = {
                "name": bone['name'],
                "matrix": bone.get('parent_matrix', [1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1])
            }
            bone_nodes.append(bone_node)
        
        # 构建骨骼层次结构
        for i, bone in enumerate(self.bones):
            children = []
            for child_name in bone.get('children', []):
                # 找到子骨骼的索引
                for j, child_bone in enumerate(self.bones):
                    if child_bone['name'] == child_name:
                        children.append(len(bone_nodes) + j + 1)  # 调整节点索引
                        break
            
            if children:
                bone_nodes[i]["children"] = children
        
        # 构建逆绑定矩阵数据
        inv_bind_data = bytearray()
        for bone in self.bones:
            matrix = bone.get('offset_matrix', [1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1])
            for val in matrix:
                inv_bind_data.extend(struct.pack('<f', val))
        
        # 添加逆绑定矩阵缓冲区
        offset = len(binary_data)
        binary_data.extend(inv_bind_data)
        
        buffer_views.append({
            "buffer": 0,
            "byteOffset": offset,
            "byteLength": len(inv_bind_data)
        })
        
        accessors.append({
            "bufferView": len(buffer_views) - 1,
            "componentType": 5126,  # FLOAT
            "count": len(self.bones),
            "type": "MAT4"
        })
        
        # 创建蒙皮对象
        joint_indices = list(range(1, len(self.bones) + 1))  # 骨骼节点从索引1开始
        
        skin = {
            "joints": joint_indices,
            "inverseBindMatrices": len(accessors) - 1,
            "name": "ArmatureSkin"
        }
        
        return {
            "skin": skin,
            "bone_nodes": bone_nodes
        }
    
    def _save_gltf(self, output_file: str, gltf_data: Dict) -> bool:
        """保存为glTF格式（JSON + bin文件）"""
        try:
            json_data = gltf_data["json"]
            binary_data = gltf_data["binary"]
            
            # 保存二进制文件
            bin_file = output_file.rsplit('.', 1)[0] + '.bin'
            json_data["buffers"][0]["uri"] = os.path.basename(bin_file)
            
            with open(bin_file, 'wb') as f:
                f.write(binary_data)
            
            # 保存JSON文件
            with open(output_file, 'w', encoding='utf-8') as f:
                json.dump(json_data, f, indent=2, ensure_ascii=False)
            
            print(f"转换成功: {output_file}")
            print(f"二进制数据: {bin_file}")
            return True
            
        except Exception as e:
            print(f"保存glTF文件失败: {e}")
            return False
    
    def _save_glb(self, output_file: str, gltf_data: Dict) -> bool:
        """保存为GLB格式（二进制glTF）"""
        try:
            json_data = gltf_data["json"]
            binary_data = gltf_data["binary"]
            
            # 构建GLB文件
            json_str = json.dumps(json_data, separators=(',', ':')).encode('utf-8')
            
            # JSON chunk需要4字节对齐
            json_padding = (4 - len(json_str) % 4) % 4
            json_str += b' ' * json_padding
            
            # 二进制数据需要4字节对齐
            bin_padding = (4 - len(binary_data) % 4) % 4
            binary_data += b'\x00' * bin_padding
            
            # GLB头部
            glb_header = struct.pack('<III', 
                0x46546C67,  # 'glTF' magic
                2,           # version
                12 + 8 + len(json_str) + 8 + len(binary_data)  # total length
            )
            
            # JSON chunk header
            json_chunk_header = struct.pack('<II', len(json_str), 0x4E4F534A)  # 'JSON'
            
            # BIN chunk header  
            bin_chunk_header = struct.pack('<II', len(binary_data), 0x004E4942)  # 'BIN\0'
            
            # 写入GLB文件
            with open(output_file, 'wb') as f:
                f.write(glb_header)
                f.write(json_chunk_header)
                f.write(json_str)
                f.write(bin_chunk_header)
                f.write(binary_data)
            
            print(f"转换成功: {output_file}")
            return True
            
        except Exception as e:
            print(f"保存GLB文件失败: {e}")
            return False


def main():
    """主函数"""
    if len(sys.argv) != 2:
        print("使用方法: python simple_mesh_to_gltf_converter.py <input.mesh>")
        print("支持输出格式:")
        print("  .gltf - 标准glTF格式 (JSON + .bin文件)")
        print("  .glb  - 二进制glTF格式")
        sys.exit(1)
    
    input_file = sys.argv[1]
    # 自动生成输出文件名，并放到parts文件夹
    input_dir = os.path.dirname(input_file)
    filename = os.path.splitext(os.path.basename(input_file))[0]
    # 将"部件"替换为"parts"
    output_dir = input_dir.replace("部件", "parts")
    # 确保输出目录存在
    os.makedirs(output_dir, exist_ok=True)
    output_file = os.path.join(output_dir, filename.lower() + ".gltf")
    
    if not os.path.exists(input_file):
        print(f"错误：输入文件不存在: {input_file}")
        sys.exit(1)
    
    converter = SimpleMeshToGLTFConverter()
    success = converter.convert_mesh_to_gltf(input_file, output_file)
    
    if success:
        print("\n转换完成！")
        
        # 输出统计信息
        print(f"\n统计信息:")
        print(f"顶点数: {converter.vertex_count}")
        print(f"面数: {converter.face_count}")
        print(f"索引数: {len(converter.indices)}")
        print(f"骨骼数: {converter.bone_count}")
        print(f"Socket数: {converter.socket_count}")
        print(f"包含位置: {converter.has_positions}")
        print(f"包含法线: {converter.has_normals}")
        print(f"包含切线: {converter.has_tangents}")
        print(f"包含颜色: {converter.has_colors}")
        print(f"包含纹理坐标: {converter.has_texcoords}")
        print(f"包含蒙皮数据: {converter.has_skin_data}")
        
    else:
        print("转换失败！")
        sys.exit(1)


if __name__ == "__main__":
    main()