#version 460 core
#pragma : compute

layout(local_size_x = 16, local_size_y = 16) in;

layout(rgba8, set = _USER_SET_, binding = 0) writeonly uniform image2D i_output_image;
layout(set = _USER_SET_, binding = 1) uniform sampler2D i_input_sampler;

layout(push_constant) uniform PSettings
{
    float exposure;
    float gamma;
    float contrast;
    float grain_intensity;
    float chromatic_aberation_intensity;
    float time;
}
p_settings;

vec3 ACES(vec3 color, const float exposure, const float gamma)
{
    color.rgb *= exposure;
    color.rgb     = pow(color.rgb, vec3(1.0 / gamma));
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
};

vec3 filmicToneMapping(vec3 color, const float exposure, const float gamma)
{
    color.rgb *= exposure;
    color.rgb = pow(color.rgb, vec3(1.0 / gamma));
    color     = max(vec3(0.), color - vec3(0.004));
    color     = (color * (6.2 * color + .5)) / (color * (6.2 * color + 1.7) + 0.06);
    return color;
};

vec3 linearToneMapping(vec3 color, const float exposure, const float gamma)
{
    color = clamp(exposure * color, 0.0, 1.0);
    color = pow(color, vec3(1.0 / gamma));
    return color;
};

vec3 reinhardToneMapping(vec3 color, const float exposure, const float gamma)
{
    color = color / (color + vec3(1.0)) * exposure;
    color = pow(color, vec3(1.0 / gamma));
    return color;
};

vec3 lumaBasedReinhardToneMapping(vec3 color, const float exposure, const float gamma)
{
    color.rgb *= exposure;
    float luma           = dot(color, vec3(0.2126, 0.7152, 0.0722));
    float toneMappedLuma = luma / (1. + luma);
    color *= toneMappedLuma / luma;
    color = pow(color, vec3(1.0 / gamma));
    return color;
};

vec3 romBinDaHouseToneMapping(vec3 color, const float exposure, const float gamma)
{
    color.rgb *= exposure;
    color = exp(-1.0 / (2.72 * color + 0.15));
    color = pow(color, vec3(1.0 / gamma));
    return color;
};

vec3 uncharted2ToneMapping(vec3 color, const float exposure, const float gamma)
{
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    float W = 11.2;
    color *= exposure;
    color       = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
    float white = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;
    color /= white;
    color = pow(color, vec3(1.0 / gamma));
    return color;
};

vec3 hableToneMapping(vec3 color, float exposure, float gamma)
{
    color *= exposure;

    const float A = 0.22;
    const float B = 0.30;
    const float C = 0.10;
    const float D = 0.20;
    const float E = 0.01;
    const float F = 0.30;
    const float W = 11.2;

    color       = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
    float white = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;

    color /= white;
    return pow(clamp(color, 0.0, 1.0), vec3(1.0 / gamma));
};

vec3 hejlBurgessDawsonToneMapping(vec3 x, float exposure, float gamma)
{
    x *= exposure;
    x = max(vec3(0.0), x - 0.004);
    return pow((x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06), vec3(1.0 / gamma));
};

vec3 uchimuraToneMapping(vec3 x, float exposure, float gamma)
{
    const float P = 1.0;  // max brightness
    const float a = 1.0;  // contrast
    const float m = 0.22; // linear section start
    const float l = 0.4;  // linear section length
    const float c = 1.33; // black
    const float b = 0.0;  // pedestal

    x *= exposure;

    vec3 lc       = clamp((x - m + l) / l, 0.0, 1.0);
    vec3 shoulder = (1.0 - exp(-a * (x - m))) / (1.0 - exp(-a * (P - m)));
    vec3 linear   = m + l * lc;
    vec3 result   = mix(linear, P * shoulder, step(m, x));
    result        = mix(b + (1.0 - b) * result / P, result, step(0.0, x));
    return pow(clamp(result, 0.0, 1.0), vec3(1.0 / gamma));
};

vec3 exponentialToneMapping(vec3 color, float exposure, float gamma)
{
    color *= exposure;
    color = 1.0 - exp(-color);
    return pow(color, vec3(1.0 / gamma));
};

// -------------------------------------------------------------------

vec3 contrastCurveToneMapping(vec3 color, float exposure, float gamma)
{
    color *= exposure;
    const float k = 1.2;
    color         = pow(color, vec3(k));
    return pow(clamp(color, 0.0, 1.0), vec3(1.0 / gamma));
}

vec3 sCurveToneMapping(vec3 color, float exposure, float gamma)
{
    color *= exposure;
    color = color / (color + vec3(0.5));
    color = pow(color, vec3(1.0 / gamma));
    return color;
}

vec3 hardContrastBoost(vec3 color, float exposure, float gamma)
{
    color *= exposure;
    color = pow(color, vec3(0.8));
    color = clamp((color - 0.5) * 1.5 + 0.5, 0.0, 1.0);
    return pow(color, vec3(1.0 / gamma));
}

vec3 contrastAdjust(vec3 color, float contrast)
{
    return clamp(((color - 0.5) * contrast + 0.5), 0.0, 1.0);
}

vec4 chromaticAberration(const vec2 uv, const ivec2 image_size, const float aberration_strength_pixels, sampler2D texture_sampler)
{
    const float offset = 0.001 * aberration_strength_pixels;

    const vec4  center = texture(texture_sampler, uv);
    const float red    = texture(texture_sampler, uv + vec2(0, offset)).r;
    const float green  = texture(texture_sampler, uv + vec2(offset, -offset)).g;
    const float blue   = texture(texture_sampler, uv + vec2(-offset, -offset)).b;

    // return mix(center, vec4(red, green, blue, 1.0), 0.5);
    return center + vec4(red, green, blue, 1.0);
}

float rand(const vec2 uv, const float time)
{
    return fract(sin(dot(uv * vec2(time), vec2(12.9898f, 78.233f))) * 43758.5453123f);
}

vec3 filmGrain(const vec3 color, const vec2 uv, const float intensity, const float time)
{
    const float PI    = 3.14159265;
    const float theta = 2.0f * PI * rand(uv, time);

    return clamp(color + vec3(cos(theta), sin(theta), tan(theta)) * intensity, vec3(0.0), color);
}

void main()
{
    const ivec2 possition    = ivec2(gl_GlobalInvocationID.xy);
    const ivec2 image_size   = imageSize(i_output_image);
    const ivec2 texture_size = textureSize(i_input_sampler, 0);
    vec2        uv           = vec2(possition) / image_size;
    uv += (1.0 / image_size) * 0.5;

    vec4 color = texture(i_input_sampler, uv);

    // -------------------------------------------------------------------
    // We apply color correction only to the texel that has color value
    // (Avoid black pixels)
    // -------------------------------------------------------------------
    if (color.r > 0.0 || color.g > 0.0 || color.b > 0.0)
    {
        //color            = chromaticAberration(uv, image_size, p_settings.chromatic_aberation_intensity, i_input_sampler);
        const vec3 grain = filmGrain(color.rgb, uv, p_settings.grain_intensity, p_settings.time);

        const vec3 processed = contrastAdjust(linearToneMapping(grain, p_settings.exposure, p_settings.gamma), p_settings.contrast);

        imageStore(i_output_image, possition, vec4(processed, color.a));
    }
}