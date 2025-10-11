#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

namespace Zongine {
    using Microsoft::WRL::ComPtr;

    class RenderPass;
    class RenderResource;

    // Resource type
    enum class ResourceType {
        Texture2D,
        RenderTarget,
        DepthStencil,
        Buffer
    };

    // Resource access mode
    enum class ResourceAccess {
        Read,
        Write,
        ReadWrite
    };

    // Resource descriptor
    struct ResourceDesc {
        std::string Name;
        ResourceType Type;
        UINT Width{ 0 };
        UINT Height{ 0 };
        DXGI_FORMAT Format{ DXGI_FORMAT_R8G8B8A8_UNORM };
        bool IsImported{ false };  // Whether this is an externally imported resource
    };

    // Pass dependency
    struct PassDependency {
        RenderPass* Pass{ nullptr };
        std::string ResourceName;
        ResourceAccess Access{ ResourceAccess::Read };
    };

    // RenderGraph class
    class RenderGraph {
    public:
        RenderGraph();
        ~RenderGraph();

        // Add pass
        template<typename T, typename... Args>
        T* AddPass(const std::string& name, Args&&... args);

        // Create resource
        void CreateResource(const ResourceDesc& desc);

        // Import external resources
        void ImportResource(const std::string& name, ComPtr<ID3D11Resource> resource);
        void ImportRenderTarget(const std::string& name, ComPtr<ID3D11RenderTargetView> rtv);
        void ImportDepthStencil(const std::string& name, ComPtr<ID3D11DepthStencilView> dsv);

        // Compile render graph
        void Compile();

        // Execute render graph
        void Execute(ComPtr<ID3D11DeviceContext> context);

        // Clear
        void Clear();

        // Get resources
        ComPtr<ID3D11Resource> GetResource(const std::string& name) const;
        ComPtr<ID3D11RenderTargetView> GetRenderTarget(const std::string& name) const;
        ComPtr<ID3D11DepthStencilView> GetDepthStencil(const std::string& name) const;
        ComPtr<ID3D11ShaderResourceView> GetShaderResource(const std::string& name) const;

    private:
        struct ResourceNode {
            ResourceDesc Desc;
            ComPtr<ID3D11Resource> Resource;
            ComPtr<ID3D11RenderTargetView> RTV;
            ComPtr<ID3D11DepthStencilView> DSV;
            ComPtr<ID3D11ShaderResourceView> SRV;
            ComPtr<ID3D11UnorderedAccessView> UAV;
        };

        struct PassNode {
            std::unique_ptr<RenderPass> Pass;
            std::string Name;
            std::vector<PassDependency> Inputs;
            std::vector<PassDependency> Outputs;
            int ExecutionOrder{ -1 };
        };

        void _BuildDependencyGraph();
        void _TopologicalSort();
        void _AllocateResources();
        void _CreateResourceViews(ResourceNode& node);

        std::vector<PassNode> m_Passes;
        std::unordered_map<std::string, ResourceNode> m_Resources;
        std::vector<RenderPass*> m_ExecutionOrder;
        bool m_bCompiled{ false };

        ComPtr<ID3D11Device> m_Device;
    };

    // Template implementation
    template<typename T, typename... Args>
    T* RenderGraph::AddPass(const std::string& name, Args&&... args) {
        static_assert(std::is_base_of<RenderPass, T>::value, "T must derive from RenderPass");
        
        PassNode node;
        node.Name = name;
        node.Pass = std::make_unique<T>(std::forward<Args>(args)...);
        
        T* pass = static_cast<T*>(node.Pass.get());
        m_Passes.push_back(std::move(node));
        
        m_bCompiled = false;
        return pass;
    }
}
