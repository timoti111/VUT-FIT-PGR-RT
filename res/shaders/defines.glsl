#define FLT_MAX 3.402823466e+38
#define FLT_MIN 1.175494351e-38
#define EPSILON 1e-8
#define TRIANGLE 1
#define SPHERE 2
#define CYLINDER 3
#define PI 3.14159265f
#define INVPI 0.31830988f
#define NUM_PATHS 2 << 19
#define UINTMAXINV 0.0000000002328306436538696
#define BXDF_DIFFUSE 1 << 1
#define BXDF_IDEAL_REFLECTION 1 << 2
#define BXDF_IDEAL_DIELECTRIC 1 << 3
#define BXDF_EMISSIVE 1 << 4
#define BXDF_IS_SINGULAR(t) ((t & (BXDF_IDEAL_REFLECTION | BXDF_IDEAL_DIELECTRIC)) != 0)

#define swap(val1, val2, type) { type tmp = val1; val1 = val2; val2 = tmp; }
