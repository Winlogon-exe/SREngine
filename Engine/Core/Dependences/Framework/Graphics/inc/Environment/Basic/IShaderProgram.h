//
// Created by Nikita on 29.03.2021.
//

#ifndef GAMEENGINE_ISHADERPROGRAM_H
#define GAMEENGINE_ISHADERPROGRAM_H

#include <FileSystem/FileSystem.h>
#include <Utils/StringUtils.h>
#include <Utils/Enumerations.h>

namespace Framework::Graphics {
    SR_ENUM_CLASS(ShaderType, Unknown, Vertex, Fragment, Tesselation)
    SR_ENUM_CLASS(LayoutBinding, Unknown = 0, Uniform = 1, Sampler2D = 2)
    SR_ENUM_CLASS(PolygonMode, Unknown, Fill, Line, Point)
    SR_ENUM_CLASS(CullMode, Unknown, None, Front, Back, FrontAndBack)
    SR_ENUM_CLASS(PrimitiveTopology,
            Unknown,
            PointList,
            LineList,
            LineStrip,
            TriangleList,
            TriangleStrip,
            TriangleFan,
            LineListWithAdjacency,
            LineStripWithAdjacency,
            TriangleListWithAdjacency,
            TriangleStripWithAdjacency,
            PathList)

    SR_ENUM_CLASS(DepthCompare,
        Unknown,
        Never,
        Less,
        Equal,
        LessOrEqual,
        Greater,
        NotEqual,
        GreaterOrEqual,
        Always)

    struct SRShaderCreateInfo {
        PolygonMode       polygonMode       = PolygonMode::Unknown;
        CullMode          cullMode          = CullMode::Unknown;
        DepthCompare      depthCompare      = DepthCompare::Unknown;
        PrimitiveTopology primitiveTopology = PrimitiveTopology::Unknown;

        bool blendEnabled = false;
        bool depthWrite   = false;
        bool depthTest    = false;

        [[nodiscard]] bool Validate() const {
            return polygonMode       != PolygonMode::Unknown
                   && cullMode          != CullMode::Unknown
                   && depthCompare      != DepthCompare::Unknown
                   && primitiveTopology != PrimitiveTopology::Unknown;
        }
    };

    static LayoutBinding GetBindingType(const std::string& line) {
        //! first check sampler, after that check uniform

        if (Helper::StringUtils::Contains(line, "sampler2D"))
            return LayoutBinding::Sampler2D;

        if (Helper::StringUtils::Contains(line, "samplerCube"))
            return LayoutBinding::Sampler2D;

        if (Helper::StringUtils::Contains(line, "uniform"))
            return LayoutBinding::Uniform;

        return LayoutBinding::Unknown;
    }

    struct SourceShader {
        std::string m_name;
        std::string m_path;
        ShaderType  m_type;

        SourceShader(const std::string& name, const std::string& path, ShaderType type) {
            m_name = name;
            m_path = path;
            m_type = type;
        }
    };

    static std::vector<std::pair<LayoutBinding, ShaderType>> AnalyseShader(
            const std::vector<SourceShader>& modules, bool* errors)
    {
        if (!errors) {
            Helper::Debug::Error("Graphics::AnalyseShader() : errors flag pointer is nullptr! You are stupid!");
            return { };
        } else
            *errors = false;

        uint32_t count = 0;

        auto bindings = std::vector<std::pair<LayoutBinding, ShaderType>>();

        std::vector<std::string> lines = { };
        for (const auto& module : modules) {
            lines = Helper::FileSystem::ReadAllLines(module.m_path);
            if (lines.empty()) {
                Helper::Debug::Error("Graphics::AnalyseShader() : failed read module! \n\tName: " + module.m_name);
                *errors = true;
                return { };
            }

            for (const std::string& line : lines) {
                if (Helper::StringUtils::Contains(line, "binding")) {
                    int32_t index = Helper::StringUtils::IndexOf(line, '=');
                    if (index <= 0) {
                        Helper::Debug::Error("Graphics::AnalyseShader() : incorrect binding location!");
                        *errors = true;
                        return { };
                    }

                    std::string location = Helper::StringUtils::ReadFrom(line, ')', index + 2);
                    if (location.empty()) {
                        Helper::Debug::Error("Graphics::AnalyseShader() : failed match location!");
                        *errors = true;
                        return { };
                    }

                    uint32_t loc = std::atoll(location.c_str());
                    if (loc + 1 > bindings.size())
                        bindings.resize(loc + 1);

                    bindings[loc] = std::pair(GetBindingType(line), module.m_type);
                    if (bindings[loc].first == LayoutBinding::Unknown) {
                        Helper::Debug::Error("Graphics::AnalyseShader() : unknown location! \n\tLine: " + line);
                        *errors = true;
                        return { };
                    }
                    count++;
                }
            }
        }

        // error correction
        for (uint32_t i = 0; i < static_cast<uint32_t>(bindings.size()); ++i)
            if (bindings[i].second == ShaderType::Unknown || bindings[i].first == LayoutBinding::Unknown) {
                Helper::Debug::Error("IShaderProgram::AnalyseShader() : incorrect bindings! Missing " + std::to_string(i) + " binding!");
                *errors = true;
                return { };
            }

        return bindings;
    }
}

#endif //GAMEENGINE_ISHADERPROGRAM_H