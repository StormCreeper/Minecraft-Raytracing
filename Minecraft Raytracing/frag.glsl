#version 430

in vec2 fragPos;

uniform mat4 view;

uniform vec3 viewPos;

uniform vec2 uResolution;

uniform int renderDistance;

uniform vec3 mapSize;
uniform samplerCube skybox;
uniform sampler3D tex3D;

uniform float uTime;

float epsilon = 0.001;

out vec4 FragColor;

#define PI 3.1415926535

float tseed = 0;
uint rngState = uint(uint(gl_FragCoord.x) * uint(1973) + uint(gl_FragCoord.y) * uint(9277) + uint(tseed * 100) * uint(26699)) | uint(1);

vec3 lightDir = normalize(vec3(0.4, 1, -0.5));
vec3 viewDir;

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
vec3 RandomUnitVector(inout uint state) {
    float z = RandomFloat01(state) * 2.0f - 1.0f;
    float a = RandomFloat01(state) * 2 * PI;
    float r = sqrt(1.0f - z * z);
    float x = r * cos(a);
    float y = r * sin(a);
    return vec3(x, y, z);
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
    
    for(int i = 0; i < octaves; ++i) 
    {
        noiseSum += snoise(pos * frequency + vec3(i * 100.02341, 121 + i * 200.0354310, 121 + i * 150.02451)) * amplitude;
        amplitude *= 0.5;
        frequency *= 2.0;
    }

    return noiseSum;
}


float RayPlaneIntersection(vec3 origin, vec3 direction, vec3 planeOrigin, vec3 planeNormal) {
	float denom = dot(-planeNormal, direction); 
    if (denom > 1e-6) { 
        vec3 p0l0 = planeOrigin - origin; 
        float t = dot(p0l0, -planeNormal) / denom; 
        return t;
    } 
 
    return -1; 
}

struct HitObj {
	vec3 hitPoint;
	vec3 hitNormal;
	vec3 hitColor;
	bool hit;
};

vec4 testVoxel(int x, int y, int z) {
	if (x < 0 || y < 0 || z < 0 || x >= mapSize.x || y >= mapSize.y || z >= mapSize.z) return vec4(1, 0, 0, 0);
	return texture(tex3D, vec3(x, y, z) / mapSize).rrrr;
}

float projectToCube(vec3 ro, vec3 rd) {
	
	float tx1 = (0 - ro.x) / rd.x;
	float tx2 = (mapSize.x - ro.x) / rd.x;

	float ty1 = (0 - ro.y) / rd.y;
	float ty2 = (mapSize.y - ro.y) / rd.y;

	float tz1 = (0 - ro.z) / rd.z;
	float tz2 = (mapSize.z - ro.z) / rd.z;

	float tx = max(min(tx1, tx2), 0);
	float ty = max(min(ty1, ty2), 0);
	float tz = max(min(tz1, tz2), 0);

	float t = max(tx, max(ty, tz));
	
	return t;
}

float voxel_traversal(vec3 orig, vec3 direction, inout vec3 normal, inout vec3 color) {
	vec3 origin = orig;
	float t1 = max(projectToCube(origin, direction) - epsilon, 0);
	origin += t1 * direction;

	int mapX = int(floor(origin.x));
	int mapY = int(floor(origin.y));
	int mapZ = int(floor(origin.z));

	float sideDistX;
	float sideDistY;
	float sideDistZ;

	float deltaDX = abs(1 / direction.x);
	float deltaDY = abs(1 / direction.y);
	float deltaDZ = abs(1 / direction.z);
	float perpWallDist = -1;

	int stepX;
	int stepY;
	int stepZ;

	int hit = 0;
	int side;

	if (direction.x < 0) {
		stepX = -1;
		sideDistX = (origin.x - mapX) * deltaDX;
	} else {
		stepX = 1;
		sideDistX = (mapX + 1.0 - origin.x) * deltaDX;
	}
	if (direction.y < 0) {
		stepY = -1;
		sideDistY = (origin.y - mapY) * deltaDY;
	} else {
		stepY = 1;
		sideDistY = (mapY + 1.0 - origin.y) * deltaDY;
	}
	if (direction.z < 0) {
		stepZ = -1;
		sideDistZ = (origin.z - mapZ) * deltaDZ;
	} else {
		stepZ = 1;
		sideDistZ = (mapZ + 1.0 - origin.z) * deltaDZ;
	}

	for (int i = 0; i < 1000; i++) {
		if ((mapX >= mapSize.x && stepX > 0) || (mapY >= mapSize.y && stepY > 0) || (mapZ >= mapSize.z && stepZ > 0)) break;
		if ((mapX < 0 && stepX < 0) || (mapY < 0 && stepY < 0) || (mapZ < 0 && stepZ < 0)) break;

		if (sideDistX < sideDistY && sideDistX < sideDistZ) {
			sideDistX += deltaDX;
			mapX += stepX;
			side = 0;
		} else if(sideDistY < sideDistX && sideDistY < sideDistZ){
			sideDistY += deltaDY;
			mapY += stepY;
			side = 1;
		} else {
			sideDistZ += deltaDZ;
			mapZ += stepZ;
			side = 2;
		}
		vec4 vox = testVoxel(mapX, mapY, mapZ);
		if (length(vox.xyz) > 0) {
			hit = 1;
			color = vox.xyz;

			if (side == 0) {
				perpWallDist = (mapX - origin.x + (1 - stepX) / 2) / direction.x + t1;
				normal = vec3(1, 0, 0) * -stepX;
			}
			else if (side == 1) {
				perpWallDist = (mapY - origin.y + (1 - stepY) / 2) / direction.y + t1;
				normal = vec3(0, 1, 0) * -stepY;
			}
			else {
				perpWallDist = (mapZ - origin.z + (1 - stepZ) / 2) / direction.z + t1;
				normal = vec3(0, 0, 1) * -stepZ;
			}
			break;
		}
	}

	return perpWallDist;
}


void SendOneRay(vec3 origin, vec3 direction, inout HitObj obj) {

	obj.hit = false;
	float td = 1;
	float t = voxel_traversal(origin, direction, obj.hitNormal, obj.hitColor);
	if (t >= 0) {
		obj.hitPoint = origin + direction * t;
		obj.hit = true;
	}
	int noiseScale = 16;
	rngState = uint(uint(obj.hitPoint.x*noiseScale) * uint(201254) + uint(obj.hitPoint.y*noiseScale) * uint(19277)+ uint(obj.hitPoint.z*noiseScale) * uint(9277) + uint(tseed * 100) * uint(26699)) | uint(1);
	//obj.hitNormal = normalize(obj.hitNormal + vec3(RandomFloat01(rngState), RandomFloat01(rngState), RandomFloat01(rngState)) * 0.04);
	//obj.hitColor *= RandomFloat01(rngState)*0.2 + 0.8;
	float n = fbm(vec3(ivec3(obj.hitPoint*16))/16.0*2, 5);
	if(n < -0.5) obj.hitColor = vec3(0.9, 1, 0.1) * pow(-n, 0.5) * 1.5;
	

}

float SendLightRay(vec3 origin, vec3 direction) {
	vec3 norm;
	vec3 col;
	float t = voxel_traversal(origin, direction, norm, col);

	return t>0 ? 0.5 : 1;

}
vec3 fogColor = vec3(1, 1, 1);

vec3 RayTrace(vec3 origin, vec3 direction) {
	HitObj obj;
	vec3 endColor = vec3(1);
	
	SendOneRay(origin, direction, obj);
	if(obj.hit) {
		float illum = 1.0;
		illum = max(dot(lightDir, obj.hitNormal), 0.3);
		illum += 3*max(pow(dot(reflect(lightDir,obj.hitNormal), normalize(direction)), 128), 0);

		illum *= SendLightRay(obj.hitPoint + obj.hitNormal * epsilon, lightDir);
		
		return obj.hitColor.xyz * illum;
	} else {
		vec3 sky = texture(skybox, direction).xyz;
		endColor *= sky;
		return endColor;
	}

	return endColor;
}

void main() {
	vec3 origin = viewPos;
	vec2 uv;
	uv = (gl_FragCoord.xy - .5 * uResolution.xy) / uResolution.y;
	vec3 dir = vec3(uv.x, uv.y, 0.5);
    vec3 direction = (vec4(dir, 0) * view).xyz;
    viewDir = (vec4(0, 1, 0, 0) * view).xyz;
	FragColor.xyz += RayTrace(origin, direction).xyz;
}