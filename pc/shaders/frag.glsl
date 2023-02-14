#version 150

in vec4 v_color;
in vec2 v_uv;
out vec4 o_color;
uniform sampler2D tex;

// Color Combiner http://n64devkit.square7.ch/pro-man/pro12/12-06.htm
// http://fgfc.ddns.net/PerfectGold/SetCombine.htm

// Formula: (A-B)*C+D
uniform float cc_1[8];
uniform float cc_2[8];
uniform vec4 rdpPrimColor;

#define CC_PARAM_A    0
#define CC_PARAM_B    1
#define CC_PARAM_C    2
#define CC_PARAM_D    3

/* Color combiner constants: */
#define G_CCMUX_COMBINED	0
#define G_CCMUX_TEXEL0		1
#define G_CCMUX_TEXEL1		2
#define G_CCMUX_PRIMITIVE	3
#define G_CCMUX_SHADE		4
#define G_CCMUX_ENVIRONMENT	5
#define G_CCMUX_CENTER		6
#define G_CCMUX_SCALE		6
#define G_CCMUX_COMBINED_ALPHA	7
#define G_CCMUX_TEXEL0_ALPHA	8
#define G_CCMUX_TEXEL1_ALPHA	9
#define G_CCMUX_PRIMITIVE_ALPHA	10
#define G_CCMUX_SHADE_ALPHA	11
#define G_CCMUX_ENV_ALPHA	12
#define G_CCMUX_LOD_FRACTION	13
#define G_CCMUX_PRIM_LOD_FRAC	14
#define G_CCMUX_NOISE		7
#define G_CCMUX_K4		7
#define G_CCMUX_K5		15
#define G_CCMUX_1		6
#define G_CCMUX_0		15

/* Alpha combiner constants: */
#define G_ACMUX_COMBINED	0
#define G_ACMUX_TEXEL0		1
#define G_ACMUX_TEXEL1		2
#define G_ACMUX_PRIMITIVE	3
#define G_ACMUX_SHADE		4
#define G_ACMUX_ENVIRONMENT	5
#define G_ACMUX_LOD_FRACTION	0
#define G_ACMUX_PRIM_LOD_FRAC	6
#define G_ACMUX_1		6
#define G_ACMUX_0		7

vec3 getColorCombinerInput(int i, vec4 texel)
{
    float s = cc_1[i];
    if (s == G_CCMUX_0) {
        return vec3(0.0, 0.0, 0.0);
    } else if (s == G_CCMUX_1) {
        return vec3(1.0, 1.0, 1.0);
    } else if (s == G_CCMUX_TEXEL0) {
        return vec3(texel.r, texel.g, texel.b);
     } else if (s == G_CCMUX_TEXEL1) {
        return vec3(texel.r, texel.g, texel.b);
    } else if (s == G_CCMUX_PRIMITIVE) {
        return vec3(rdpPrimColor.r, rdpPrimColor.g, rdpPrimColor.b);
    } else if (s == G_CCMUX_SHADE) {
        return vec3(v_color.r, v_color.g, v_color.g);
    } else if (s == G_CCMUX_PRIMITIVE_ALPHA) {
        return vec3(rdpPrimColor.a, rdpPrimColor.a, rdpPrimColor.a);
    }

    return vec3(0.0, 0.0, 0.0);
}

float getAlphaCombinerInput(int i, vec4 texel)
{
    float s = cc_1[i];
    if (s == G_ACMUX_0) {
        return 0.0;
    } else if (s == G_ACMUX_1) {
        return 1.0;
    } else if (s == G_ACMUX_TEXEL0) {
        return texel.a;
    } else if (s == G_ACMUX_TEXEL1) {
        return texel.a;
    } else if (s == G_ACMUX_PRIMITIVE) {
        return rdpPrimColor.a;
    } else if (s == G_ACMUX_SHADE) {
        return v_color.a;
    }

    return 0.0;
}

void main() {
    vec4 texel = texture(tex, v_uv);

    // Color Combiner
    vec3 cc_col_input_a = getColorCombinerInput(CC_PARAM_A, texel);
    vec3 cc_col_input_b = getColorCombinerInput(CC_PARAM_B, texel);
    vec3 cc_col_input_c = getColorCombinerInput(CC_PARAM_C, texel);
    vec3 cc_col_input_d = getColorCombinerInput(CC_PARAM_D, texel);

    vec3 cc_col_output = (cc_col_input_a - cc_col_input_b) * cc_col_input_c + cc_col_input_d;

    float cc_alpha_input_a = getAlphaCombinerInput(CC_PARAM_A + 4, texel);
    float cc_alpha_input_b = getAlphaCombinerInput(CC_PARAM_B + 4, texel);
    float cc_alpha_input_c = getAlphaCombinerInput(CC_PARAM_C + 4, texel);
    float cc_alpha_input_d = getAlphaCombinerInput(CC_PARAM_D + 4, texel);

    float cc_alpha_output = (cc_alpha_input_a - cc_alpha_input_b) * cc_alpha_input_c + cc_alpha_input_d;

    vec4 cc_output = vec4(cc_col_output.r, cc_col_output.g, cc_col_output.b, cc_alpha_output);

    o_color = cc_output;
    //o_color = vec4(1.0, 0.0, 0.0, 1.0);
    //o_color = vec4(v_color.r, v_color.g, v_color.b, 1.0);
}
