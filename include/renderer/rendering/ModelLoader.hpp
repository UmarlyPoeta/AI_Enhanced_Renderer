#pragma once

#include "renderer/rendering/Mesh.hpp"

#include <memory>
#include <string>

namespace renderer::rendering {

    class ModelLoader {
    public:
        static std::unique_ptr<Mesh> load(const std::string& path);
    };

}