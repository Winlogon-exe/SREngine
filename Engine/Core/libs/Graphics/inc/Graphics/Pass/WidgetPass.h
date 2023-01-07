//
// Created by Monika on 14.07.2022.
//

#ifndef SRENGINE_WIDGETPASS_H
#define SRENGINE_WIDGETPASS_H

#include <Graphics/Pass/BasePass.h>

namespace SR_GRAPH_NS {
    class WidgetPass : public BasePass {
    public:
        explicit WidgetPass(RenderTechnique* pTechnique, BasePass* pParent);
        ~WidgetPass() override = default;

    public:
        bool Overlay() override;

    };
}


#endif //SRENGINE_WIDGETPASS_H