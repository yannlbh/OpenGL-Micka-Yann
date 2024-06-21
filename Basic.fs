#version 150

varying vec4 v_color;
varying vec3 v_position;

uniform vec3 u_Id;
uniform vec3 u_L;
varying vec3 v_N;
varying vec2 v_texcoords_d;
uniform sampler2D u_sampler;

uniform vec3 E;

uniform float u_shininess;
uniform vec3 u_Is;
uniform vec3 u_Ks;

vec3 diffuse(vec3 N, vec3 L, vec3 K) {
    N = normalize(N);

    float NdotL = dot(N, L);

    return u_Id * max(0.0, NdotL) * K;
}


vec3 specular(vec3 N, vec3 I, vec3 P, vec3 E, vec3 K)
{
    N = normalize(N);
    I = normalize(I);

    if (dot(N, -I) <= 0.0) {
        return vec3(0.0);
    }

    vec3 V = normalize(E - P);
//    vec3 R = reflect(I, N);
    vec3 H = normalize(-I + V);
//    return pow(max(dot(R, V), 0.0), u_shininess) * u_Is * u_Ks;
    return pow(max(dot(N, H), 0.0), u_shininess) * u_Is * u_Ks;
}



void main(void) {
    vec4 texcolor_d = texture2D(u_sampler, v_texcoords_d);
    vec3 diff = diffuse(v_N, -u_L, texcolor_d.xyz);
    vec3 spec = specular(v_N, u_L, v_position, E, texcolor_d.xyz);
    gl_FragColor = v_color * vec4(diff, 1.0) + vec4(spec, 1.0);
}