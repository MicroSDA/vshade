#version 460 core
#pragma : vertex

#include "common/common.glsl"

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec3 a_tangent;
layout(location = 3) in vec3 a_bi_tangent;
layout(location = 4) in vec2 a_uv_coordinates;

layout(std140, set = _GLOBAL_SET_, binding = _CAMERA_BUFFER_BINDING_INDEX_) uniform UCamera
{
    Camera u_camera;
};

layout(std430, set = _PER_INSTANCE_SET_, binding = _TRANSFORM_BUFFER_BINDING_INDEX_) restrict readonly buffer STransforms
{
    mat4 s_transforms[];
};

layout(location = 0) flat out int out_instance_id;
layout(location = 1) out vec2 out_uv_coordinates;
layout(location = 2) out vec3 out_position;

void main()
{
    gl_Position        =  s_transforms[gl_InstanceIndex] * vec4(a_position.xy, 0.0, 1.0);
    out_instance_id    = gl_InstanceIndex;
    out_uv_coordinates =  vec2((a_position.x + 1.0) / 2.0, 1 + (a_position.y + 1.0) / 2.0); //a_uv_coordinates;
    out_position       = a_position;
}

#version 460 core
#pragma : fragment

#extension GL_EXT_nonuniform_qualifier : enable

#include "common/common.glsl"

layout(std430, set = _PER_INSTANCE_SET_, binding = _MATERIAL_BUFFER_BINDING_INDEX_) restrict readonly buffer SMaterial
{
    Material s_material[];
};
layout(set = _PER_INSTANCE_SET_, binding = _MATERIAL_TEXTURES_BINDING_INDEX_) uniform sampler2D MaterialTextures[_MAX_SAMPLED_IMAGES_];

layout(location = 0) flat in int in_instance_id;
layout(location = 1) in vec2 in_uv_coordinates;
layout(location = 2) in vec3 in_position;

layout(location = 0) out vec4 out_fragment_color;

layout(push_constant) uniform PDrawIndex
{
    int index;
}
p_draw_index;

void main()
{
    const int albedo_texture_index    = (in_instance_id * 4 + p_draw_index.index * 4) + _ALBEDO_TEXTURE_INDEX_;
    const int diffuse_texture_index   = (in_instance_id * 4 + p_draw_index.index * 4) + _DIFFUSE_TEXTURE_INDEX_;
    const int specular_texture_index  = (in_instance_id * 4 + p_draw_index.index * 4) + _SPECULAR_TEXTURE_INDEX_;
    const int norma_map_texture_index = (in_instance_id * 4 + p_draw_index.index * 4) + _NORMAL_MAP_TEXTURE_INDEX_;

    out_fragment_color = texture(MaterialTextures[diffuse_texture_index], in_uv_coordinates).rgba;
}
#version 460 core
#pragma : vertex

#include "common/common.glsl"

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec3 a_tangent;
layout(location = 3) in vec3 a_bi_tangent;
layout(location = 4) in vec2 a_uv_coordinates;

layout(std140, set = _GLOBAL_SET_, binding = _CAMERA_BUFFER_BINDING_INDEX_) uniform UCamera
{
    Camera u_camera;
};

layout(std430, set = _PER_INSTANCE_SET_, binding = _TRANSFORM_BUFFER_BINDING_INDEX_) restrict readonly buffer STransforms
{
    mat4 s_transforms[];
};

layout(location = 0) flat out int out_instance_id;
layout(location = 1) out vec2 out_uv_coordinates;
layout(location = 2) out vec3 out_position;

void main()
{
    gl_Position        = s_transforms[gl_InstanceIndex] * vec4(a_position.xy, 0.0, 1.0);
    out_instance_id    = gl_InstanceIndex;
    out_uv_coordinates = vec2((a_position.x + 1.0) / 2.0, 1 + (a_position.y + 1.0) / 2.0); // a_uv_coordinates;
    out_position       = a_position;
}

#version 460 core
#pragma : fragment

#extension GL_EXT_nonuniform_qualifier : enable

#include "common/common.glsl"

layout(std430, set = _PER_INSTANCE_SET_, binding = _MATERIAL_BUFFER_BINDING_INDEX_) restrict readonly buffer SMaterial
{
    Material s_material[];
};
layout(set = _PER_INSTANCE_SET_, binding = _MATERIAL_TEXTURES_BINDING_INDEX_) uniform sampler2D MaterialTextures[_MAX_SAMPLED_IMAGES_];

layout(location = 0) flat in int in_instance_id;
layout(location = 1) in vec2 in_uv_coordinates;
layout(location = 2) in vec3 in_position;

layout(location = 0) out vec4 out_fragment_color;

layout(push_constant) uniform PDrawIndex
{
    int index;
}
p_draw_index;

void main()
{
    const int albedo_texture_index    = (in_instance_id * 4 + p_draw_index.index * 4) + _ALBEDO_TEXTURE_INDEX_;
    const int diffuse_texture_index   = (in_instance_id * 4 + p_draw_index.index * 4) + _DIFFUSE_TEXTURE_INDEX_;
    const int specular_texture_index  = (in_instance_id * 4 + p_draw_index.index * 4) + _SPECULAR_TEXTURE_INDEX_;
    const int norma_map_texture_index = (in_instance_id * 4 + p_draw_index.index * 4) + _NORMAL_MAP_TEXTURE_INDEX_;

    out_fragment_color = texture(MaterialTextures[diffuse_texture_index], in_uv_coordinates).rgba;
}