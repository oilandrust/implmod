uniform vec3 cameraPos;
attribute mat4 boneWTrans;


varying vec3 worldPos;
varying mat4 bmat;//Used to debug


void main(){
	vec4 pos = gl_Vertex;//

	worldPos = pos.xyz;
	bmat = boneWTrans;

	gl_Position = ftransform();
}
