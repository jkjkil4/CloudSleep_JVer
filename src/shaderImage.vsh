
attribute vec3 in_Position;
attribute vec2 in_TexCoord;

varying vec2 v_vTexCoord;

uniform vec2 u_vWindowSize;
uniform vec2 u_vTexPos;
uniform vec2 u_vTexOrig;
uniform vec2 u_vTexSize;
uniform vec2 u_vTexScale;
uniform float u_fRotation;

void main(){
    float winX = (in_Position[0] * u_vTexSize[0] - u_vTexOrig[0]) * u_vTexScale[0];
    float winY = (in_Position[1] * u_vTexSize[1] - u_vTexOrig[1]) * u_vTexScale[1];
    gl_Position.x = u_vTexPos[0] + (winX * cos(u_fRotation) - winY * sin(u_fRotation)) / u_vWindowSize[0];
    gl_Position.y = u_vTexPos[1] + (winX * sin(u_fRotation) + winY * cos(u_fRotation)) / u_vWindowSize[1];
    gl_Position.z = 0.0;
    gl_Position.w = 1.0;

    v_vTexCoord = in_TexCoord;
}
