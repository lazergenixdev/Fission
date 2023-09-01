#version 450 core

layout (location = 0) in vec2 uv;
layout (location = 0) out vec4 fragColor;

layout (set = 0, binding = 0) uniform sampler2D color;
layout (set = 0, binding = 1) uniform A {
	vec3 resolution_strength;
} i;

#define iResolution (i.resolution_strength.xy)
#define iStrength   (i.resolution_strength.z)

//=================================================================================================
//
//  Baking Lab
//  by MJP and David Neubelt
//  http://mynameismjp.wordpress.com/
//
//  All code licensed under the MIT license
//
// The code in this file was originally written by Stephen Hill (@self_shadow), who deserves all
// credit for coming up with this fit and implementing it. Buy him a beer next time you see him. :)
const mat3x3 ACESInputMat =
{
    {0.59719, 0.35458, 0.04823},
    {0.07600, 0.90834, 0.01566},
    {0.02840, 0.13383, 0.83777}
};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
const mat3x3 ACESOutputMat =
{
    { 1.60475, -0.53108, -0.07367},
    {-0.10208,  1.10813, -0.00605},
    {-0.00327, -0.07276,  1.07602}
};

vec3 RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}

vec4 ACESFilm(vec4 color)
{
    color.rgb = ACESInputMat * color.rgb;

    // Apply RRT and ODT
    color.rgb = RRTAndODTFit(color.rgb);

    color.rgb = ACESOutputMat * color.rgb;

    // Clamp to [0, 1]
    //color = saturate(color);

    return color;
}
//=================================================================================================

#if 0
#define pow2(x) (x * x)
const float pi = atan(1.0) * 4.0;
const int samples = 16;
const float sigma = float(samples) * 0.25;

float gaussian(vec2 i) {
    return 1.0 / (2.0 * pi * pow2(sigma)) * exp(-(dot(i,i) / (2.0 * pow2(sigma))));
}

vec3 blur(sampler2D sp, vec2 uv, vec2 scale) {
    vec3 col = vec3(0.0);
    float accum = 0.0;
    float weight;
    vec2 offset;
    
    for (int x = -samples / 2; x < samples / 2; ++x) {
        for (int y = -samples / 2; y < samples / 2; ++y) {
            offset = vec2(x, y);
            weight = gaussian(offset);
            col += texture(sp, uv + scale * offset).rgb * weight;
            accum += weight;
        }
    }
    
    return col / accum;
}
#else
float normpdf(in float x, in float sigma) {
	return 0.39894*exp(-0.5*x*x/(sigma*sigma))/sigma;
}

vec3 blur(sampler2D s, vec2 uv, vec2 scale) {
	vec3 c = texture(s, uv).rgb;

	//declare stuff
	const int mSize = 13;
	const int kSize = (mSize-1)/2;
	float kernel[mSize];
	vec3 final_colour = vec3(0.0);
	
	//create the 1-D kernel
	float sigma = 3.0;
	float Z = 0.0;
	for (int j = 0; j <= kSize; ++j)
	{
		kernel[kSize+j] = kernel[kSize-j] = normpdf(float(j), sigma);
	}
	
	//get the normalization factor (as the gaussian has been clamped)
	for (int j = 0; j < mSize; ++j)
	{
		Z += kernel[j];
	}
	
	//read out the texels
	for (int i=-kSize; i <= kSize; ++i)
	{
		for (int j=-kSize; j <= kSize; ++j)
		{
			final_colour += kernel[kSize+j]*kernel[kSize+i]*texture(s, uv+vec2(float(i),float(j))*scale).rgb;
		}
	}
	
	return final_colour/(Z*Z);
}
#endif

void main() {
	vec4 Color = vec4(blur(color, uv, vec2(1.0)/iResolution), 0.5);
    fragColor = ACESFilm(Color * iStrength);
}
