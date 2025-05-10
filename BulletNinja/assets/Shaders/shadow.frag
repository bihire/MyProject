uniform sampler2D texture;

void main() {
    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
    
    // Only draw shadow where the original sprite is visible
    if (pixel.a > 0.1) {
        // Dark shadow with 60% opacity
        gl_FragColor = vec4(0.0, 0.0, 0.0, pixel.a * 0.6);
    } else {
        discard; // Completely discard transparent pixels
    }
}