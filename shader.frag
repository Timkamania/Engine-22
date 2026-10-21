#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragWorldPos;

layout(location = 0) out vec4 outFragColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionViewMatrix;
    vec4 ambientLightColor;
    vec4 lightPosition;
    vec4 lightColor;
} ubo;

void main() {

    // Вектор от фрагмента к источнику света (в мировом пространстве)
    vec3 directionToLight = ubo.lightPosition.xyz - fragWorldPos;
    float distanceSquared = dot(directionToLight, directionToLight);

   //  +1.0 защищает от деления на ноль, если свет ровно в вершине
  float attenuation = 1.0 / (distanceSquared + 1.0);

  vec3 lightIntensity = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
  vec3 ambientLight   = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;

    // Диффузное освещение Ламберта (по пикселям!)
  float cosAngIncidence = max(dot(normalize(fragNormal), normalize(directionToLight)), 0.0);
  vec3 diffuseLight = lightIntensity * cosAngIncidence;

   outFragColor = vec4((diffuseLight + ambientLight) * fragColor, 1.0);
 
 
}