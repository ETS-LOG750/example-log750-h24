#version 430 core
// Texture
layout(location = 0) uniform sampler2D iChannel0;
// If we want to do filtering or not
layout(location = 1) uniform bool useFilter;
// UV coordinates
in vec2 fUV;
// Out color
out vec4 fColor;

// From: https://www.shadertoy.com/view/Xdf3Rf
float intensity(in vec4 color){
	return sqrt((color.x*color.x)+(color.y*color.y)+(color.z*color.z));
}
vec3 sobel(float stepx, float stepy, vec2 center){
	// get samples around pixel
    float tleft = intensity(texture(iChannel0,center + vec2(-stepx,stepy)));
    float left = intensity(texture(iChannel0,center + vec2(-stepx,0)));
    float bleft = intensity(texture(iChannel0,center + vec2(-stepx,-stepy)));
    float top = intensity(texture(iChannel0,center + vec2(0,stepy)));
    float bottom = intensity(texture(iChannel0,center + vec2(0,-stepy)));
    float tright = intensity(texture(iChannel0,center + vec2(stepx,stepy)));
    float right = intensity(texture(iChannel0,center + vec2(stepx,0)));
    float bright = intensity(texture(iChannel0,center + vec2(stepx,-stepy)));
 
	// Sobel masks (see http://en.wikipedia.org/wiki/Sobel_operator)
	//        1 0 -1     -1 -2 -1
	//    X = 2 0 -2  Y = 0  0  0
	//        1 0 -1      1  2  1
	
	// You could also use Scharr operator:
	//        3 0 -3        3 10   3
	//    X = 10 0 -10  Y = 0  0   0
	//        3 0 -3        -3 -10 -3
 
    float x = tleft + 2.0*left + bleft - tright - 2.0*right - bright;
    float y = -tleft - 2.0*top - tright + bleft + 2.0 * bottom + bright;
    float color = sqrt((x*x) + (y*y));
    return vec3(color,color,color);
 }

void main()
{
    if(useFilter) {
        // Step for computing the sobol
        float step = 0.001;
        fColor = vec4(sobel(step, step, fUV), 1.0);
    } else {
        // We use absolut value to better display the position
        fColor = abs(texture(iChannel0, fUV));
    }
}