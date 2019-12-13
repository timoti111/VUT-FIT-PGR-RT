#include "Material.h"
#include <imgui.h>

static const char* names[] = { "Diffuse", "Ideal Reflective", "Ideal Refractive", "Emissive" };
static const std::vector<int> values = { DIFFUSE, IDEAL_REFLECTION, IDEAL_REFRACTION, EMISSIVE };

Material::Material()
{
    Kd = glm::vec4(0.5f);
    Ks = glm::vec4(0.5f);
    Ke = glm::vec4(0.5f);
    Ns = 1.0f;
    Ni = 1.52f;
    map_Kd = -1;
    map_Ks = -1;
    map_N = -1;
    type = DIFFUSE;
}

bool Material::drawGUI()
{
    bool changed = false;
    int index = valueToIndex(type);
    changed |= ImGui::Combo("Material type", &index, names, IM_ARRAYSIZE(names));
    type = indexToValue(index);
    if (index == 0)
        changed |= ImGui::ColorEdit3("Diffuse color", &Kd.x);
    if (index == 1)
        changed |= ImGui::ColorEdit3("Specular color", &Ks.x);
    if (index == 2)
    {
        changed |= ImGui::ColorEdit3("Specular color", &Ks.x);
        ImGui::Text("Refractive index");
        changed |= ImGui::SliderFloat("", &Ni, 1.0f, 2.65f);
    }
    if (index == 3)
    {
        changed |= ImGui::ColorEdit3("Emissive color", &Ke.x);
        ImGui::Text("Emission intensity");
        changed |= ImGui::SliderFloat("", &Ns, 0.0f, 100.0f);
    }
    return changed;
}

int Material::indexToValue(int index)
{
    return values[index];
}

int Material::valueToIndex(int value)
{
    for (int i = 0; i < values.size(); i++)
        if (values[i] == value)
            return i;
    return 0;
}

static std::vector<Material> materials;
Material* Material::generateNewMaterial(int& newIndex)
{
    newIndex = materials.size();
    materials.push_back(Material());
    return materials.data() + newIndex;
}

std::vector<Material>& Material::getMaterials()
{
    return materials;
}
