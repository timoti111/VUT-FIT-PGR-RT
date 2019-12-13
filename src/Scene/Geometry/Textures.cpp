#include "Textures.h"
#include <vector>
static std::vector<std::shared_ptr<ge::gl::Texture>> textures;

unsigned Textures::addTexture(std::shared_ptr<ge::gl::Texture> texture)
{
    textures.push_back(texture);
    return textures.size() - 1;
}

void Textures::bindTextures(int arrayBinding)
{
    for (int i = 0; i < textures.size(); i++)
        textures[i]->bind(arrayBinding + i);
}
