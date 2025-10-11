#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <vector>

namespace Zongine {
    using Microsoft::WRL::ComPtr;

    class RenderGraph;
    struct PassDependency;

    // RenderPass abstract base class
    class RenderPass {
    public:
        virtual ~RenderPass() = default;

        // Setup phase: declare input/output resources
        virtual void Setup(RenderGraph& graph) = 0;

        // Execute phase: perform rendering
        virtual void Execute(ComPtr<ID3D11DeviceContext> context, RenderGraph& graph) = 0;

        // Get input resources
        const std::vector<PassDependency>& GetInputs() const { return m_Inputs; }

        // Get output resources
        const std::vector<PassDependency>& GetOutputs() const { return m_Outputs; }

        // Enable/disable pass
        void SetEnabled(bool enabled) { m_bEnabled = enabled; }
        bool IsEnabled() const { return m_bEnabled; }

    protected:
        // Helper functions: add input/output
        void AddInput(const std::string& resourceName);
        void AddOutput(const std::string& resourceName);

        std::vector<PassDependency> m_Inputs;
        std::vector<PassDependency> m_Outputs;
        bool m_bEnabled{ true };
    };
}
