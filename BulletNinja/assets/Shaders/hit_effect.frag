uniform sampler2D texture;
uniform float intensity;
uniform float time;
uniform vec4 hitColor; // Typically white (1,1,1,1)
uniform vec4 baseColor; // Typically black (0,0,0,1)

void main() {
    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
    
    if (pixel.a == 0.0) {
        discard;
    }
    
    // Create a pulsing effect (0-1-0)
    float pulse = abs(sin(time * 10.0));
    
    // Blend between hit color and base color
    vec4 effectColor = mix(baseColor, hitColor, pulse);
    
    // Mix with original texture based on intensity
    gl_FragColor = mix(pixel, effectColor, intensity);
}