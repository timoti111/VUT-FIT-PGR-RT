#pragma once
#include <glm/vec4.hpp>

struct alignas(16) Material
{
    Material();
    glm::vec4 Kd;     // diffuse reflectivity
    glm::vec4 Ks;     // specular reflectivity 
    glm::vec4 Ke;     // emission
    float Ns;   // specular exponent (shininess), normally in [0, 1000]
    float Ni;   // index of refraction
    int map_Kd; // diffuse texture descriptor idx
    int map_Ks; // specular texture descriptor idx
    int map_N;  // normal texture descriptor idx
    int type;   // BXDF type, defined in bxdf.cl

    bool drawGUI();
    int indexToValue(int index);
    int valueToIndex(int value);
};

enum MaterialType
{
    DIFFUSE = 1 << 1,
    IDEAL_REFLECTION = 1 << 2,
    IDEAL_REFRACTION = 1 << 3,
    EMISSIVE = 1 << 4
};

