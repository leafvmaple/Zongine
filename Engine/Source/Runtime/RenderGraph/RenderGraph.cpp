#include "RenderGraph.h"
#include "RenderPass.h"
#include "../Managers/DeviceManager.h"

#include <algorithm>
#include <queue>
#include <set>

namespace Zongine {

    RenderGraph::RenderGraph() {
        m_Device = DeviceManager::GetInstance().GetDevice();
    }

    RenderGraph::~RenderGraph() {
        Clear();
    }

    void RenderGraph::CreateResource(const ResourceDesc& desc) {
        if (m_Resources.find(desc.Name) != m_Resources.end()) {
            return; // Resource already exists
        }

        ResourceNode node;
        node.Desc = desc;
        m_Resources[desc.Name] = node;
        m_bCompiled = false;
    }

    void RenderGraph::ImportResource(const std::string& name, ComPtr<ID3D11Resource> resource) {
        ResourceNode node;
        node.Desc.Name = name;
        node.Desc.IsImported = true;
        node.Resource = resource;
        m_Resources[name] = node;
        m_bCompiled = false;
    }

    void RenderGraph::ImportRenderTarget(const std::string& name, ComPtr<ID3D11RenderTargetView> rtv) {
        ResourceNode node;
        node.Desc.Name = name;
        node.Desc.Type = ResourceType::RenderTarget;
        node.Desc.IsImported = true;
        node.RTV = rtv;
        
        // Get underlying resource from RTV
        ComPtr<ID3D11Resource> resource;
        rtv->GetResource(resource.GetAddressOf());
        node.Resource = resource;
        
        m_Resources[name] = node;
        m_bCompiled = false;
    }

    void RenderGraph::ImportDepthStencil(const std::string& name, ComPtr<ID3D11DepthStencilView> dsv) {
        ResourceNode node;
        node.Desc.Name = name;
        node.Desc.Type = ResourceType::DepthStencil;
        node.Desc.IsImported = true;
        node.DSV = dsv;
        
        // Get underlying resource from DSV
        ComPtr<ID3D11Resource> resource;
        dsv->GetResource(resource.GetAddressOf());
        node.Resource = resource;
        
        m_Resources[name] = node;
        m_bCompiled = false;
    }

    void RenderGraph::Compile() {
        if (m_bCompiled) {
            return;
        }

        // 1. Allocate resources
        _AllocateResources();

        // 2. Build dependency graph
        _BuildDependencyGraph();

        // 3. Topological sort to determine execution order
        _TopologicalSort();

        m_bCompiled = true;
    }

    void RenderGraph::Execute(ComPtr<ID3D11DeviceContext> context) {
        if (!m_bCompiled) {
            Compile();
        }

        // Execute all passes in order
        for (auto* pass : m_ExecutionOrder) {
            if (pass) {
                pass->Execute(context, *this);
            }
        }
    }

    void RenderGraph::Clear() {
        m_Passes.clear();
        m_Resources.clear();
        m_ExecutionOrder.clear();
        m_bCompiled = false;
    }

    ComPtr<ID3D11Resource> RenderGraph::GetResource(const std::string& name) const {
        auto it = m_Resources.find(name);
        if (it != m_Resources.end()) {
            return it->second.Resource;
        }
        return nullptr;
    }

    ComPtr<ID3D11RenderTargetView> RenderGraph::GetRenderTarget(const std::string& name) const {
        auto it = m_Resources.find(name);
        if (it != m_Resources.end()) {
            return it->second.RTV;
        }
        return nullptr;
    }

    ComPtr<ID3D11DepthStencilView> RenderGraph::GetDepthStencil(const std::string& name) const {
        auto it = m_Resources.find(name);
        if (it != m_Resources.end()) {
            return it->second.DSV;
        }
        return nullptr;
    }

    ComPtr<ID3D11ShaderResourceView> RenderGraph::GetShaderResource(const std::string& name) const {
        auto it = m_Resources.find(name);
        if (it != m_Resources.end()) {
            return it->second.SRV;
        }
        return nullptr;
    }

    void RenderGraph::_BuildDependencyGraph() {
        // Setup dependencies for each pass
        for (auto& passNode : m_Passes) {
            if (passNode.Pass) {
                passNode.Pass->Setup(*this);
                passNode.Inputs = passNode.Pass->GetInputs();
                passNode.Outputs = passNode.Pass->GetOutputs();
            }
        }
    }

    void RenderGraph::_TopologicalSort() {
        m_ExecutionOrder.clear();

        // Calculate in-degree for each pass
        std::unordered_map<RenderPass*, int> inDegree;
        std::unordered_map<RenderPass*, std::vector<RenderPass*>> adjacencyList;

        for (auto& passNode : m_Passes) {
            inDegree[passNode.Pass.get()] = 0;
        }

        // Build adjacency list
        for (auto& passNode : m_Passes) {
            for (const auto& output : passNode.Outputs) {
                // Find other passes that use this output
                for (auto& otherNode : m_Passes) {
                    if (otherNode.Pass.get() == passNode.Pass.get())
                        continue;

                    for (const auto& input : otherNode.Inputs) {
                        if (input.ResourceName == output.ResourceName) {
                            adjacencyList[passNode.Pass.get()].push_back(otherNode.Pass.get());
                            inDegree[otherNode.Pass.get()]++;
                        }
                    }
                }
            }
        }

        // Kahn's algorithm for topological sort
        std::queue<RenderPass*> queue;
        for (auto& [pass, degree] : inDegree) {
            if (degree == 0) {
                queue.push(pass);
            }
        }

        while (!queue.empty()) {
            RenderPass* current = queue.front();
            queue.pop();
            m_ExecutionOrder.push_back(current);

            for (RenderPass* neighbor : adjacencyList[current]) {
                inDegree[neighbor]--;
                if (inDegree[neighbor] == 0) {
                    queue.push(neighbor);
                }
            }
        }

        // Check for cycles
        if (m_ExecutionOrder.size() != m_Passes.size()) {
            // Cycle detected, throw exception or log error
            // Here we simply clear the execution order
            m_ExecutionOrder.clear();
        }
    }

    void RenderGraph::_AllocateResources() {
        for (auto& [name, node] : m_Resources) {
            if (node.Desc.IsImported) {
                // Create views for imported resources
                _CreateResourceViews(node);
                continue;
            }

            // Create resources based on type
            switch (node.Desc.Type) {
            case ResourceType::Texture2D:
            case ResourceType::RenderTarget: {
                D3D11_TEXTURE2D_DESC texDesc = {};
                texDesc.Width = node.Desc.Width;
                texDesc.Height = node.Desc.Height;
                texDesc.MipLevels = 1;
                texDesc.ArraySize = 1;
                texDesc.Format = node.Desc.Format;
                texDesc.SampleDesc.Count = 1;
                texDesc.SampleDesc.Quality = 0;
                texDesc.Usage = D3D11_USAGE_DEFAULT;
                texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
                texDesc.CPUAccessFlags = 0;
                texDesc.MiscFlags = 0;

                ComPtr<ID3D11Texture2D> texture;
                HRESULT hr = m_Device->CreateTexture2D(&texDesc, nullptr, texture.GetAddressOf());
                if (SUCCEEDED(hr)) {
                    node.Resource = texture;
                    _CreateResourceViews(node);
                }
                break;
            }
            case ResourceType::DepthStencil: {
                D3D11_TEXTURE2D_DESC texDesc = {};
                texDesc.Width = node.Desc.Width;
                texDesc.Height = node.Desc.Height;
                texDesc.MipLevels = 1;
                texDesc.ArraySize = 1;
                texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
                texDesc.SampleDesc.Count = 1;
                texDesc.SampleDesc.Quality = 0;
                texDesc.Usage = D3D11_USAGE_DEFAULT;
                texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
                texDesc.CPUAccessFlags = 0;
                texDesc.MiscFlags = 0;

                ComPtr<ID3D11Texture2D> texture;
                HRESULT hr = m_Device->CreateTexture2D(&texDesc, nullptr, texture.GetAddressOf());
                if (SUCCEEDED(hr)) {
                    node.Resource = texture;
                    _CreateResourceViews(node);
                }
                break;
            }
            case ResourceType::Buffer: {
                // Buffer creation logic
                break;
            }
            }
        }
    }

    void RenderGraph::_CreateResourceViews(ResourceNode& node) {
        if (!node.Resource) {
            return;
        }

        switch (node.Desc.Type) {
        case ResourceType::RenderTarget: {
            // Create RTV if it doesn't exist (imported resources may already have one)
            if (!node.RTV) {
                D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
                rtvDesc.Format = node.Desc.Format;
                rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                rtvDesc.Texture2D.MipSlice = 0;

                m_Device->CreateRenderTargetView(node.Resource.Get(), &rtvDesc, node.RTV.GetAddressOf());
            }

            // Create SRV if it doesn't exist (imported resources usually need SRV creation)
            if (!node.SRV) {
                // Get texture description to determine format
                ComPtr<ID3D11Texture2D> texture;
                node.Resource.As(&texture);
                
                D3D11_TEXTURE2D_DESC texDesc;
                texture->GetDesc(&texDesc);
                
                D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                srvDesc.Format = texDesc.Format;
                srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MostDetailedMip = 0;
                srvDesc.Texture2D.MipLevels = 1;

                m_Device->CreateShaderResourceView(node.Resource.Get(), &srvDesc, node.SRV.GetAddressOf());
            }
            break;
        }
        case ResourceType::DepthStencil: {
            // Create DSV if it doesn't exist
            if (!node.DSV) {
                D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
                dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
                dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
                dsvDesc.Texture2D.MipSlice = 0;

                m_Device->CreateDepthStencilView(node.Resource.Get(), &dsvDesc, node.DSV.GetAddressOf());
            }

            // Create SRV if it doesn't exist
            if (!node.SRV) {
                D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
                srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MostDetailedMip = 0;
                srvDesc.Texture2D.MipLevels = 1;

                m_Device->CreateShaderResourceView(node.Resource.Get(), &srvDesc, node.SRV.GetAddressOf());
            }
            break;
        }
        case ResourceType::Texture2D: {
            if (!node.SRV) {
                // Get texture description to determine format
                ComPtr<ID3D11Texture2D> texture;
                node.Resource.As(&texture);
                
                D3D11_TEXTURE2D_DESC texDesc;
                texture->GetDesc(&texDesc);
                
                D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                srvDesc.Format = texDesc.Format;
                srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MostDetailedMip = 0;
                srvDesc.Texture2D.MipLevels = 1;

                m_Device->CreateShaderResourceView(node.Resource.Get(), &srvDesc, node.SRV.GetAddressOf());
            }
            break;
        }
        }
    }
}
