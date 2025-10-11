#include "RenderPass.h"
#include "RenderGraph.h"

namespace Zongine {

    void RenderPass::AddInput(const std::string& resourceName) {
        PassDependency dep;
        dep.Pass = this;
        dep.ResourceName = resourceName;
        dep.Access = ResourceAccess::Read;
        m_Inputs.push_back(dep);
    }

    void RenderPass::AddOutput(const std::string& resourceName) {
        PassDependency dep;
        dep.Pass = this;
        dep.ResourceName = resourceName;
        dep.Access = ResourceAccess::Write;
        m_Outputs.push_back(dep);
    }
}
