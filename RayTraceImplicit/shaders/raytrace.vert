uniform vec3 cameraPos;
varying vec3 worldPos;

void main(){
	vec4 pos = gl_Vertex;//
	worldPos = pos.xyz;
	gl_Position = ftransform();
}
