#pragma once
#include "Vars/Vars.h"

class RayGenKernel {
public:
    RayGenKernel(vars::Vars* vars);

private:
    vars::Vars* vars;
};