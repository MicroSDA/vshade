//------------------------------------------------------------------------
// Depth test and depth attachment is required !
//------------------------------------------------------------------------

#version 460 core
#pragma : vertex

#include "common/common.glsl"

layout(location = 0) in vec3 a_position;

layout(location = 0) out vec3 out_near;
layout(location = 1) out vec3 out_far;

layout(std140, set = _GLOBAL_SET_, binding = _CAMERA_BUFFER_BINDING_INDEX_) uniform UCamera
{
    Camera u_camera;
};

layout(std430, set = _PER_INSTANCE_SET_, binding = _TRANSFORM_BUFFER_BINDING_INDEX_) restrict readonly buffer STransform
{
    mat4 s_transforms[];
};

// Function that returns the unprojected point given specific coordinates and view projection matrix
vec3 unprojectPoint(const float x, const float y, const float z, const mat4 inverse_view_projection)
{
    vec4 unprojected_point = inverse_view_projection * vec4(x, y, z, 1.0);
    return unprojected_point.xyz / unprojected_point.w;
}

void main()
{
    gl_Position = s_transforms[gl_InstanceIndex] * vec4(a_position, 1.0);

    // Calculate the near and far unprojected points for the current vertex
    const mat4 innvers_projection = inverse(u_camera.view_projection_matrix);
    out_near                      = unprojectPoint(a_position.x, a_position.y, 0.0, innvers_projection).xyz;
    out_far                       = unprojectPoint(a_position.x, a_position.y, 1.0, innvers_projection).xyz;
}

#version 460 core
#pragma : fragment

#include "common/common.glsl"

#define _DEPTH_RANGE_NEAR_ 0.1
#define _DEPTH_RANGE_FAR_ 100.0

layout(location = 0) in vec3 in_near;
layout(location = 1) in vec3 in_far;

layout(std140, set = _GLOBAL_SET_, binding = _CAMERA_BUFFER_BINDING_INDEX_) uniform UCamera
{
    Camera u_camera;
};

// Function that creates a grid given specific parameters
vec4 createGrid(const vec3 grid_color, const vec3 frag_pos_3D, const float scale, const float line_width, const bool draw_axis)
{
    // Calculate the coordinates of the current fragment in the 2D plane
    const vec2 coord      = frag_pos_3D.xz * scale;
    const vec2 derivative = fwidth(coord) + vec2(line_width);
    const vec2 grid       = abs(fract(coord - 0.5) - 0.5) / derivative;
    // Calculate the line value based on the coordinate values
    const float line  = min(grid.x, grid.y);
    const float min_z = min(derivative.y, 0.1);
    const float min_x = min(derivative.x, 0.1);
    // Set the color value based on the line value and the draw_axis parameter
    const float alpha = 1.0 - min(line, 1.0);

    // Center point in XZ plane
    if (abs(frag_pos_3D.x) < 5.0 * min_x + line_width &&
        abs(frag_pos_3D.z) < 5.0 * min_z + line_width)
    {
        if (draw_axis)
            return vec4(0.2, 0.3, 0.7, alpha); 
    }
    // Z axis
    if (frag_pos_3D.x > -2.0 * min_x - line_width && frag_pos_3D.x < 2.0 * min_x + line_width)
    {
        if (draw_axis)
            return vec4(0.2, 0.7, 0.3, alpha);
    }
    // X axis
    if (frag_pos_3D.z > -2.0 * min_z - line_width && frag_pos_3D.z < 2.0 * min_z + line_width)
    {
        if (draw_axis)
            return vec4(0.7, 0.2, 0.3, alpha);
    }

    return vec4(grid_color, alpha);
}

// This function calculates the depth of a given 3D position in the scene, based on the current camera and projection settings.
float computeDepth(const vec3 position)
{
    // Transform the position into clip space using the current camera view and projection matrices.
    const vec4 clip_space_position = u_camera.view_projection_matrix * vec4(position.xyz, 1.0);

    // Calculate the depth value in clip space by dividing the z component by the w component of the position vector.
    const float clip_space_depth = (clip_space_position.z / clip_space_position.w);

    // Calculate the final depth value by transforming the clip_space_depth back into world space, and averaging the near and far clip distances.
    // return (((_DEPTH_RANGE_FAR_ - _DEPTH_RANGE_NEAR_) * clip_space_depth) + _DEPTH_RANGE_NEAR_ + _DEPTH_RANGE_FAR_) / 2.0;

    return clip_space_depth;
}

// Calculates the linearized depth of a given position in view space
float computeLinearDepth(const vec3 position)
{
    // Multiply the position by the view and projection matrices to obtain the clip space position
    const vec4 clip_space_position = u_camera.view_projection_matrix * vec4(position.xyz, 1.0);

    // Calculate the clip space depth
    const float clip_space_depth = (clip_space_position.z / clip_space_position.w) * 2.0 - 1.0;

    // Calculate linear depth using the near and far clip planes of the current view frustum
    const float linear_depth = (2.0 * _DEPTH_RANGE_NEAR_ * _DEPTH_RANGE_FAR_) /
                               (_DEPTH_RANGE_FAR_ + _DEPTH_RANGE_NEAR_ - clip_space_depth * (_DEPTH_RANGE_FAR_ - _DEPTH_RANGE_NEAR_));

    // Scale the result down for improved precision and return the linear depth value
    return linear_depth / _DEPTH_RANGE_FAR_;
}

#define _GRID_SCALE_ 1.0
#define _LINE_WIDTH_ 0.005

layout(location = 0) out vec4 out_fragment_color;

void main()
{
    const float t                  = -in_near.y / (in_far.y - in_near.y); // Calculate t based on near and far plane coordinates
    const vec3  frag_pos_3D        = in_near + t * (in_far - in_near);    // Calculate fragment position in 3D space
    const float linear_depth       = computeLinearDepth(frag_pos_3D);
    const float fading             = max(0.0, (1.0 - linear_depth));          //  Calculate fading based on linear depth
    const float distance_to_camera = length(u_camera.position - frag_pos_3D); // Compute linear depth based on fragment position
    // const float fading             = smoothstep(u_camera.far, u_camera.near, distance_to_camera); // Calculate fading based on camera distance
    vec3 grid_color = vec3(0.1); // Set grid color
    vec4 color1     = createGrid(grid_color * 2.0, frag_pos_3D, _GRID_SCALE_ * 0.2, _LINE_WIDTH_, true);
    vec4 color2     = createGrid(grid_color, frag_pos_3D, _GRID_SCALE_, _LINE_WIDTH_, false);
    vec4 color      = mix(color2, color1, color1.a) * float(t > 0);

    color.a *= fading;
    color.rgb *= fading;
    // Discard fragments with low alpha
    if (color.a <= 0.1)
        discard;

    gl_FragDepth       = computeDepth(frag_pos_3D); // Compute depth based on fragment position
    out_fragment_color = color;
}