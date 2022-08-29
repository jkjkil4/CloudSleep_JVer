
attribute vec3 in_Position;
attribute vec3 in_Color;
attribute vec2 in_TexCoord;

varying vec2 v_vTexCoord;

uniform vec2 u_vWindowSize;
uniform vec2 u_vTexPos;
uniform vec2 u_vTexSize;
uniform float u_fRotation;

void main(){
    float x = u_vTexSize[0] * in_Position[0];
    float y = u_vTexSize[1] * in_Position[1];
    gl_Position.x = (u_vTexPos[0] + x * cos(u_fRotation) - y * sin(u_fRotation));
    gl_Position.y = (u_vTexPos[1] + x * sin(u_fRotation) + y * cos(u_fRotation));
    gl_Position.z = 0.0;
    gl_Position.w = 1.0;

    v_vTexCoord = in_TexCoord;
}
