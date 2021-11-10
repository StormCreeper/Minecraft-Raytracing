#version 430

layout (local_size_x = 16, local_size_y = 16, local_size_z = 4) in;

layout (r8ui, binding = 0) uniform uimage3D img_output;

uniform float iTime;

float tseed = 0;
uint rngState;

uint wang_hash(inout uint seed) {
    seed = uint(seed ^ uint(61)) ^ uint(seed >> uint(16));
    seed *= uint(9);
    seed = seed ^ (seed >> 4);
    seed *= uint(0x27d4eb2d);
    seed = seed ^ (seed >> 15);
    return seed;
} 
float RandomFloat01(inout uint state) {
    return float(wang_hash(state)) / 4294967296.0;
}

vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
vec4 taylorInvSqrt(vec4 r){return 1.79284291400159 - 0.85373472095314 * r;}

float snoise(vec3 v){ 
	const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
	const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

	// First corner
	vec3 i  = floor(v + dot(v, C.yyy) );
	vec3 x0 =   v - i + dot(i, C.xxx) ;

	// Other corners
	vec3 g = step(x0.yzx, x0.xyz);
	vec3 l = 1.0 - g;
	vec3 i1 = min( g.xyz, l.zxy );
	vec3 i2 = max( g.xyz, l.zxy );

	//  x0 = x0 - 0. + 0.0 * C 
	vec3 x1 = x0 - i1 + 1.0 * C.xxx;
	vec3 x2 = x0 - i2 + 2.0 * C.xxx;
	vec3 x3 = x0 - 1. + 3.0 * C.xxx;

	// Permutations
	i = mod(i, 289.0 ); 
	vec4 p = permute( permute( permute( 
	         i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
	       + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
	       + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

	// Gradients
	// ( N*N points uniformly over a square, mapped onto an octahedron.)
	float n_ = 1.0/7.0; // N=7
	vec3  ns = n_ * D.wyz - D.xzx;

	vec4 j = p - 49.0 * floor(p * ns.z *ns.z);  //  mod(p,N*N)

	vec4 x_ = floor(j * ns.z);
	vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

	vec4 x = x_ *ns.x + ns.yyyy;
	vec4 y = y_ *ns.x + ns.yyyy;
	vec4 h = 1.0 - abs(x) - abs(y);

	vec4 b0 = vec4( x.xy, y.xy );
	vec4 b1 = vec4( x.zw, y.zw );

	vec4 s0 = floor(b0)*2.0 + 1.0;
	vec4 s1 = floor(b1)*2.0 + 1.0;
	vec4 sh = -step(h, vec4(0.0));

	vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
	vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

	vec3 p0 = vec3(a0.xy,h.x);
	vec3 p1 = vec3(a0.zw,h.y);
	vec3 p2 = vec3(a1.xy,h.z);
	vec3 p3 = vec3(a1.zw,h.w);

	//Normalise gradients
	vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
	p0 *= norm.x;
	p1 *= norm.y;
	p2 *= norm.z;
	p3 *= norm.w;

	// Mix final noise value
	vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
	m = m * m;
	return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
                            dot(p2,x2), dot(p3,x3) ) );
}

float fbm(vec3 pos, int octaves)  {
    float noiseSum = 0.0, frequency = 1.0, amplitude = 1.0;
    float ampSum = 0.0;
    
    for(int i = 0; i < octaves; ++i) {
        noiseSum += snoise(pos * frequency + vec3(i * 100.02341, 121 + i * 200.0354310, 121 + i * 150.02451)) * amplitude;
        ampSum += amplitude;
        amplitude *= 0.5;
        frequency *= 2.0;
    }

    return noiseSum;
}

float getHeight01(vec2 pos, inout float h1, inout float flatness, inout float river) {
	float scale = 0.001;
	float waterlevel = 0.2;

	float noise = fbm(vec3(pos * scale, 250), 16);

	float mountain = pow(1 - abs(1-noise), 2);
	river = pow(1 - abs(noise), 4);

	float roughness = (fbm(vec3(pos * scale, 500), 1) * 0.5 + 0.5) * (1-mountain);

	float land = fbm(vec3(pos * scale, 0), 16) * 0.5 + 0.5;

	float mountainHeight = 0.7 + 0.3 * fbm(vec3(pos * scale, 750), 2);
	h1 = mountain;
	
	flatness = 1 - mountain;

	//river = (min(river + 0.1, 1) - 0.1) / (1 - 0.1);

	return mix(0.05 + roughness * land + mountain * mountainHeight, waterlevel, river) - pow(river, 3) * 0.1;
}


int getVoxel(ivec3 coords) {

	rngState = uint(uint(coords.x*16) * uint(201254) + uint(coords.y*16) * uint(19277)+ uint(coords.z*16) * uint(9277)) | uint(1);

	float h = 256;
	float h1;
	float f;
	float r;
	//if(coords.y < 128) return 11;
	//else return 0;
	float height = getHeight01(coords.xz + vec2(190, 300), h1, f, r) * h;

	if(coords.y > height && coords.y < height + 1) {
		//if(RandomFloat01(rngState) < 0.05) return 15;
	}
	if(coords.y > height) {
		float n = 1 - abs(fbm(coords * 0.1, 4));
		if(coords.x > 110 && coords.x < 130 && coords.z > 110 && coords.z < 130 && coords.y < 210) return n < 0.8 ? 7 : 8;
		if(coords.x > 70 && coords.x < 90 && coords.z > 100 && coords.z < 120 && coords.y < 210) return n < 0.8 ? 9 : 10;
		//if(coords.x > 80 && coords.x < 200 && coords.z > 130 && coords.z < 250 && coords.y < 100) return 11;
		
		if(coords.y < 0.2 * h) return 14;

		return 0;
	}
	
	if(coords.y > height-3) {
		//if(r > 0.6) return 14;
		float n = fbm(vec3(coords.xz, 0)*0.05, 4);
		if(h1*h > n * 8 + 0.90 * h) return 12;
		if(h1*h > n * 18 + 0.85 * h) return 3;
		if(h1*h > n * 18 + 0.80 * h) return 2;
		if(r >= 0.55) return 13;
		return 1;
	} 
	if(coords.y > height-7) return 2;

	float noise = fbm(coords.xyz*0.1, 3) * fbm(coords.xyz*0.01, 2);
	if(noise < -0.5) return 4;
	noise = fbm(coords.xyz*0.1 + vec3(10), 3) * fbm(coords.xyz*0.01 + vec3(10), 2);
	if(noise < -0.5) return 8;
	noise = fbm(coords.xyz*0.1 + vec3(20), 3) * fbm(coords.xyz*0.01 + vec3(20), 2);
	if(noise < -0.5) return 6;
	return 3;
}

void main() {
	ivec3 coords = ivec3(gl_GlobalInvocationID);
	
	uint rngState = uint(uint(coords.x) * uint(1973) + uint(coords.y) * uint(12573) + uint(coords.z) * uint(9277) + uint(tseed * 100) * uint(26699)) | uint(1);
	
	int pixel = getVoxel(coords);
	imageStore(img_output, coords, uvec4(pixel));
}