#include "VertexUtils.h"

#include <unordered_map>

namespace Zongine {
    struct SpatialHash {
        SpatialHash(float size, XMFLOAT3 offset) : cellSize(size), offset(offset) {}

        uint64_t getHashKey(const XMFLOAT4& pos) {
            int x = (int)((pos.x - offset.x) / cellSize);
            int y = (int)((pos.y - offset.y) / cellSize);
            int z = (int)((pos.z - offset.z) / cellSize);

            assert(x >= 0 && y >= 0 && z >= 0);

            return ((uint64_t)x << 42) | ((uint64_t)y << 21) | (uint64_t)z;
        }

        void insert(int vertexIndex, const XMFLOAT4& pos) {
            cells[getHashKey(pos)].push_back(vertexIndex);
        }

        std::vector<int> getNeighbors(const XMFLOAT4& pos, float radius) {
            std::vector<int> neighbors;

            int radiusCells = (int)ceil(radius / cellSize);

            for (int dx = -radiusCells; dx <= radiusCells; dx++) {
                for (int dy = -radiusCells; dy <= radiusCells; dy++) {
                    for (int dz = -radiusCells; dz <= radiusCells; dz++) {
                        XMFLOAT4 offsetPos(pos.x + dx * cellSize,
                            pos.y + dy * cellSize,
                            pos.z + dz * cellSize, 0);
                        uint64_t key = getHashKey(offsetPos);

                        if (cells.find(key) != cells.end()) {
                            neighbors.insert(neighbors.end(),
                                cells[key].begin(),
                                cells[key].end());
                        }
                    }
                }
            }
            return neighbors;
        }

        const std::vector<int>& getNeighbors(const XMFLOAT4& pos) {
            return cells[getHashKey(pos)];
        }

        std::unordered_map<uint64_t, std::vector<int>> cells;
        float cellSize{};
        XMFLOAT3 offset{};
    };

    void MergeVertex(std::vector<int>& ParticleVertexMap,
        std::vector<int>& VertiesParticlesMap,
        const std::vector<XMFLOAT4>& Verties,
        float cellSize, XMFLOAT3 offset,
        std::function<bool(int vertex1, int vertex2)> func) {

        SpatialHash spatialHash(cellSize, offset);

        for (int i = 0; i < Verties.size(); i++) {
            spatialHash.insert(i, Verties[i]);
        }

        VertiesParticlesMap.resize(Verties.size(), -1);
        for (int i = 0; i < Verties.size(); i++) {
            if (VertiesParticlesMap[i] != -1)
                continue;

            VertiesParticlesMap[i] = (int)ParticleVertexMap.size();
            ParticleVertexMap.push_back(i);

            auto neighbors = spatialHash.getNeighbors(Verties[i]);

            for (auto neighbor : neighbors) {
                if (VertiesParticlesMap[neighbor] != -1)
                    continue;
                if (func(i, neighbor))
                    VertiesParticlesMap[neighbor] = VertiesParticlesMap[i];
            }
        }
    }

}