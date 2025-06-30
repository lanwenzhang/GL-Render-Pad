#version 460 core

layout(location = 0) in vec3 aPos;

uniform mat4 lightMatrix;
uniform mat4 modelMatrix;

out vec3 worldPosition;

void main(){
	
	worldPosition = (modelMatrix * vec4(aPos, 1.0)).xyz;

	gl_Position = lightMatrix * modelMatrix * vec4(aPos, 1.0);

}