#version 330 core

in vec3 vFragPositionVS;
in vec3 vFragNormalVS;
in vec2 vFragTex;

out vec3 fFragColor;

uniform sampler2D uKdSampler;

uniform vec3 uDirectionalLightDir;
uniform vec3 uDirectionalLightIntensity;
uniform vec3 uPointLightPosition;
uniform vec3 uPointLightIntensity;
uniform vec3 uKd;

void main(){

    float distToPointLight = length(uPointLightPosition -vFragPositionVS);
	vec3 dirToPointLight = (uPointLightPosition - vFragPositionVS) / distToPointLight;

	vec3 tmp = texture(uKdSampler, vFragTex).rgb * uKd;

	fFragColor = tmp * (uDirectionalLightIntensity * max(0.0, dot(vFragNormalVS, uDirectionalLightDir)) + uPointLightIntensity * max(0.0, dot(vFragNormalVS, dirToPointLight)) / (distToPointLight * distToPointLight));

}
