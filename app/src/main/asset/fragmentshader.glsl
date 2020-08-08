#version 120
precision highp float;
varying highp vec2 v_texcoord;
uniform sampler2D texSampler;
void main() {
    gl_FragColor = vec4(0.5,0.5,0.5,1.0);//texture2D(texSampler, v_texcoord);
}
