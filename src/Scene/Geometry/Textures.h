#pragma once
#include <geGL/geGL.h>
#include <memory>
class Textures
{
public:
    static unsigned addTexture(std::shared_ptr<ge::gl::Texture> texture);
    static void bindTextures(int arrayBinding);
};