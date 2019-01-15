#version 330 core

in vec3 vFragPositionVS;
in vec3 vFragNormalVS;
in vec2 vFragTex;

out vec3 fFragNormalVS;

uniform sampler2D uTexture;

void main(){
    //vec4 tmp = texture(uTexture, vFragTex);
    //fFragNormalVS = tmp.xyz;
    fFragNormalVS = vFragNormalVS;
}
