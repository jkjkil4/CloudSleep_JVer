#ifdef GL_ES
precision mediump float;
#endif

varying vec2 v_vTexCoord;

uniform sampler2D u_sTexImg;

void main() {
    gl_FragColor = texture2D(u_sTexImg, v_vTexCoord);
}
