struct Camera
{
    mat4  view_projection_matrix;
    mat4  view_matrix;
    mat4  projection_matrix;
    vec3  position;
    vec3  forward_direction;
    float near;
    float far;
};

struct Material
{
    vec3  ambient_color;
    vec3  diffuse_color;
    vec3  albedo_color;
    vec3  specular_color;
    vec3  transparent_color;
    float emissive;
    float opacity;
    float shininess;
    float shininess_strength;
    float refractive_index;
    int   shading_model;
    bool  normal_map_enabled;
    bool  bump_map_enabled;
};