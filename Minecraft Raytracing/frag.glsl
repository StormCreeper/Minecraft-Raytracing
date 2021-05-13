#version 430

in vec2 fragPos;

uniform mat4 view;

uniform vec3 viewPos;

uniform vec2 uResolution;

uniform int renderDistance;

uniform vec3 mapSize;
uniform samplerCube skybox;
uniform usampler3D tex3D;

uniform float uTime;

float epsilon = 0.001;

struct WaterParameters {
	float intensity;
	vec2 speed;
	float diffuse;
	float reflection;
	float refraction;
	float ior;
};

uniform WaterParameters wt;

uniform float air_absorbance;
uniform float water_absorbance;

out vec4 FragColor;

#define PI 3.141592653589793238462643383279

float tseed = 0;
uint rngState = uint(uint(gl_FragCoord.x) * uint(1973) + uint(gl_FragCoord.y) * uint(9277) + uint(tseed * 100) * uint(26699)) | uint(1);

vec3 lightDir = normalize(vec3(0.2, 1, -0.6));
vec3 viewDir;

vec3 air_fog = vec3(0.8, 1, 1);
vec3 water_fog = vec3(0.1, 0.3, 0.5)*0.5;

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
float fbmOff(vec3 pos, vec3 off, int octaves)  {
    float noiseSum = 0.0, frequency = 1.0, amplitude = 1.0;
    
    for(int i = 0; i < octaves; ++i) 
    {
        noiseSum += snoise(pos * frequency + off) * amplitude;
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

uint testVoxel(int x, int y, int z) {
	if (x < 0 || y < 0 || z < 0 || x >= mapSize.x || y >= mapSize.y || z >= mapSize.z) return 0;
	return texture(tex3D, vec3(x, y, z) / mapSize).r;
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

float voxel_traversal(vec3 orig, vec3 direction, inout vec3 normal, inout uint blockType, inout vec3 throughput) {
	vec3 origin = orig;
	uint medium = testVoxel(int(origin.x), int(origin.y), int(origin.z));
	if(medium != 14) medium = 0;
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

	int step = 1;

	for (int i = 0; i < 6000; i++) {
		if ((mapX >= mapSize.x && stepX > 0) || (mapY >= mapSize.y && stepY > 0) || (mapZ >= mapSize.z && stepZ > 0)) break;
		if ((mapX < 0 && stepX < 0) || (mapY < 0 && stepY < 0) || (mapZ < 0 && stepZ < 0)) break;

		if (sideDistX < sideDistY && sideDistX < sideDistZ) {
			sideDistX += deltaDX;
			mapX += stepX * step;
			side = 0;
		} else if(sideDistY < sideDistX && sideDistY < sideDistZ){
			sideDistY += deltaDY;
			mapY += stepY * step;
			side = 1;
		} else {
			sideDistZ += deltaDZ;
			mapZ += stepZ * step;
			side = 2;
		}

		if(medium == 14) throughput.x *= 1 - pow(10, -water_absorbance);
		if(medium == 0) throughput.y *= 1 - pow(10, -air_absorbance);

		uint block = testVoxel(mapX, mapY, mapZ);
		if (block != medium) {
			hit = 1;
			if(block != 0) blockType = block;
			else blockType = medium;

			if (side == 0) {
				perpWallDist = (mapX - origin.x + (1 - stepX * step) / 2) / direction.x + t1;
				normal = vec3(1, 0, 0) * -stepX;
			}
			else if (side == 1) {
				perpWallDist = (mapY - origin.y + (1 - stepY * step) / 2) / direction.y + t1;
				normal = vec3(0, 1, 0) * -stepY;
			}
			else {
				perpWallDist = (mapZ - origin.z + (1 - stepZ * step) / 2) / direction.z + t1;
				normal = vec3(0, 0, 1) * -stepZ;
			}
			break;
		}
	}

	return perpWallDist;
}

struct Material {
	vec3 color;
	vec3 normal;
	float specular;
	float rmin;
	float rmax;
	vec3 tint;
	float transparent;
};
// 0 : random noise block (parameter : intensity);
// 1 : random noise pixel (parameter : intensity);
// 2 : ores (parameter : color)
// 3 : vines (parameter : color)
// 4 : vignette

void addDetails(int type, inout Material mat, inout uint rngb, inout uint rngp, vec3 pos, float val = 0, vec3 color = vec3(0)) {
	//if(length(pos - viewPos) > 100 && (type == 1 || type == 3)) return;
	ivec3 co = ivec3(pos * 16);
	ivec3 nco = ivec3(pos * 16) % 16;
	switch(type) {
	case 0:
		mat.color *= (RandomFloat01(rngb)*2-1) * val + 1;
		mat.normal = normalize(mat.normal + vec3(RandomFloat01(rngb)*2-1, RandomFloat01(rngb)*2-1, RandomFloat01(rngb)*2-1) * 0.04);
		break;
	case 1:
		mat.color *= (RandomFloat01(rngp)*2-1) * val + (1-val);
		mat.normal = normalize(mat.normal + vec3(RandomFloat01(rngp)*2-1, RandomFloat01(rngp)*2-1, RandomFloat01(rngp)*2-1) * val * 0.4);
		break;
	case 2:
		float n1 = fbm(co/8.0, 5);
		if(n1 < -0.5) {
			mat.color = color * pow(-n1, 0.5) * 1.5;
			mat.specular = 2;
		}
		break;
	case 3:
		float n = fbm(co * 0.1, 3);
		if(n > .4) mat.color = mix(mat.color, color*0.7 + vec3(0, RandomFloat01(rngp), 0)*val, (n - 0.5)*2);
		n = 1-abs(fbm(co * 0.1 + vec3(10), 3));
		if(n > .8) mat.color = mix(mat.color, color + vec3(0, RandomFloat01(rngp), 0)*val, pow((n - 0.5)*2, 4));
		break;
	case 4:
		mat.color *= pow(1/length(vec3(nco) - vec3(8, 8, 8)) * 8, val);
		break;
	default:
		break;
	}
}

float waterHeightFunction(vec3 pos, float scale, int depth) {
	vec3 time = uTime * vec3(wt.speed.x, 0, wt.speed.y);
	return fbmOff(pos * scale, time, depth) * 0.2 + 0.8;
}

void getMaterial(uint type, vec3 pos, inout Material mat) {

	uint rngb = uint(uint(pos.x) * uint(201254) + uint(pos.y) * uint(19277)+ uint(pos.z) * uint(9277) + uint(tseed * 100) * uint(26699)) | uint(1);
	uint rngp = uint(uint(pos.x*16) * uint(201254) + uint(pos.y*16) * uint(19277)+ uint(pos.z*16) * uint(9277) + uint(tseed * 100) * uint(26699)) | uint(1);

	ivec3 co = ivec3(pos * 16);
	ivec3 nco = ivec3(pos * 16) % 16;

	if(type == 7 || type == 8) {
		vec3 ba = vec3(0.9, 0.4, 0.3);
		vec3 bb = vec3(0.9, 0.8, 0.8);

		bool displaced = true;
		if(RandomFloat01(rngp) > 0.9) co.x ++;
		else if(RandomFloat01(rngp) > 0.9) co.y ++;
		else if(RandomFloat01(rngp) > 0.9) co.z ++;
		else displaced = false;

		if(abs(mat.normal.x) > 0.5) if(co.y % 16 > 8) co.z += 8;
		if(abs(mat.normal.y) > 0.5) if(co.x % 16 > 8) co.z += 8;
		if(abs(mat.normal.z) > 0.5) if(co.y % 16 > 8) co.x += 8;

		int vx = abs(8 - co.x % 16);
		int vy = abs(4 - co.y % 8)*2;
		int vz = abs(8 - co.z % 16);

		if(abs(mat.normal.y) > 0.5) vx = abs(4 - co.x % 8)*2;

		float vam;

		if(abs(mat.normal.x) > 0.5) vam = max(vy, vz);
		if(abs(mat.normal.y) > 0.5) vam = max(vx, vz);
		if(abs(mat.normal.z) > 0.5) vam = max(vx, vy);

		if(vam > 7) {
			mat.color = mix(bb, ba, displaced ? 0.5 : 0);
		} else {
			mat.color = (ba + vec3(RandomFloat01(rngp)) * 0.1) * (1.7-pow(float(vam + 5) / 13, 0.7));
		}

		if(type == 8) addDetails(3, mat, rngb, rngp, pos, 0.1, vec3(0.2, 0.5, 0.1));

		mat.specular = 0.1;

		return;
	}
	if(type == 9 || type == 10) {
		vec3 col1 = vec3(0.3);
		vec3 col2 = vec3(0.8);
		
		float noise = fbm(vec3(co) *0.1, 4)*0.5 + 0.5;

		mat.color = mix(col1, col2, noise + RandomFloat01(rngp)*0.3);
		mat.color *= pow(1/length(vec3(nco) - vec3(8, 8, 8)) * 9, 1);

		mat.normal = normalize(mat.normal + 0.1 * vec3(cos(noise*3), sin(noise*3), cos(noise*3 + 0.3)) + 0.05 * vec3(RandomFloat01(rngp)*2-1, RandomFloat01(rngp)*2-1, RandomFloat01(rngp)*2-1));

		if(type == 10) addDetails(3, mat, rngb, rngp, pos, 0.1, vec3(0.2, 0.5, 0.1));

		mat.specular = 0.1;
		return;
	}

	if(type == 11) {
		vec3 col = vec3(1, 1, 0.5);
		float d = 1/length(vec3(nco) - vec3(8, 8, 8)) * 8;
		col *= pow(d, 0.4);
		float noise = fbm(co * 0.1, 5) * 0.5 + 0.5;

		mat.color = col*1.5;
		mat.specular = 0;
		mat.rmax = 0.8;
		mat.rmin = 0;
		mat.tint = vec3(1, 1, 0);
		addDetails(1, mat, rngb, rngp, pos, 0.01);
		return;
	}
	if(type == 12) {
		vec3 col = vec3(1, 1, 1);
		float d = 1/length(vec3(nco) - vec3(8, 8, 8)) * 8;
		col += vec3(RandomFloat01(rngp)) * 0.1;
		float noise = fbm(co * 0.1, 5) * 0.5 + 0.5;

		mat.color = col;
		mat.specular = 0.5;
		mat.normal = normalize(mat.normal + 0.01*(vec3(RandomFloat01(rngp), RandomFloat01(rngp), RandomFloat01(rngp)) * 2 - 1));

		return;
	}
	if(type == 14) {
		int depth = 10;
		float scale = wt.intensity;
		float n11 = waterHeightFunction(pos, scale, depth);
		if(mat.normal.y > 0.5) {
			float n12 = waterHeightFunction(pos + vec3(1, 0, 0), scale, depth);
			float n21 = waterHeightFunction(pos + vec3(0, 0, 1), scale, depth);

			mat.normal = -normalize(cross(vec3(1, n12 - n11, 0),vec3(0, n21-n11, 1)));
		}

		vec3 col = mix(vec3(0.1, 0.5, 1.0), vec3(0.2, 0.8, 1.0), n11);

		mat.color = col;
		mat.specular = 0;
		mat.rmin = 0.5;
		mat.rmax = 1;
		mat.tint = vec3(0.8, 0.8, 1.0);
		mat.transparent = 1;
		return;
	}

	vec3 ore;
	switch(type) {
		case 1:
			mat.color = vec3(0.3, 1, 0.1); break;
		case 2:
			mat.color = vec3(0.8, 0.7, 0.2); break;
		case 3:
			mat.color = vec3(0.8, 0.8, 0.8); break;
		case 4:
			mat.color = vec3(0.8, 0.8, 0.8);
			ore = vec3(0.1, 0.1, 0.1); break;
		case 5:
			mat.color = vec3(0.8, 0.8, 0.8);
			ore = vec3(0.9, 0.8, 0.1); break;
		case 6:
			mat.color = vec3(0.8, 0.8, 0.8);
			ore = vec3(0.1, 0.8, 0.9); break;
		case 13:
			mat.color = vec3(0.8, 1, 0.4); break;
		default:
			mat.color = vec3(0, 1, 0); break;
	}

	
	mat.specular = 0.02;
	if(type == 3) mat.specular = 0.1;
	if(type==4 || type==5 || type==6) {
		mat.specular = 0.1;
		addDetails(2, mat, rngb, rngp, pos, 0, ore);
	}
	addDetails(0, mat, rngb, rngp, pos, 0.1);
	addDetails(1, mat, rngb, rngp, pos, 0.1);
	addDetails(4, mat, rngb, rngp, pos, 0.2);
	
		
	return;
}

struct HitObj {
	vec3 hitPoint;
	Material mat;
	bool hit;
};

void SendOneRay(vec3 origin, vec3 direction, inout HitObj obj, inout vec3 throughput) {
	obj.hit = false;

	uint blockType = 0;

	float t = voxel_traversal(origin, direction, obj.mat.normal, blockType, throughput);

	if (t >= 0) {
		obj.hitPoint = origin + direction * t;
		getMaterial(blockType, obj.hitPoint, obj.mat);
		obj.hit = true;
	}
}

float SendLightRay(vec3 origin, vec3 direction, inout vec3 throughput) {
	vec3 norm;
	uint bt;
	float t = voxel_traversal(origin, direction, norm, bt, throughput);

	return t>0 ? 0.5 : 1;
}

vec3 lerp(vec3 a, vec3 b, vec3 c) {
	return vec3(mix(a.x, b.x, c.x),
		mix(a.y, b.y, c.y),
		mix(a.z, b.z, c.z));
}

vec3 getSkyColor(vec3 dir) {
	//return texture(skybox, dir).xyz;
	vec3 color = vec3(0.2, 0.3, 0.8);
	color += pow(max(dot(lightDir, dir), 0), 256) * vec3(1, 0.6, 0.8) * 6;
	float density = fbm(dir*0.6 + vec3(uTime*0.05, 0, 0) + vec3(10), 1) * 0.5 + 0.5;
	float cloud = fbm(dir + vec3(uTime*0.07, 0, 0), 10) * 0.5 + 0.5;
	float heightMod = 1/(1+exp(-4*dir.y));
	color += density * cloud * heightMod;
	return color;
}

vec3 getInfoFrom(vec3 origin, vec3 direction, inout HitObj obj) {
	vec3 endColor = vec3(1);

	vec3 throughput = vec3(1);
	vec3 throughputl = vec3(1);

	SendOneRay(origin, direction, obj, throughput);
	if(obj.hit) {
		float illum = 1.0;
		illum = max(dot(lightDir, obj.mat.normal), 0.3);
		illum += obj.mat.specular * 5 *max(pow(dot(reflect(lightDir,obj.mat.normal), normalize(direction)), 128), 0);

		illum *= SendLightRay(obj.hitPoint + obj.mat.normal * epsilon, lightDir, throughputl);
		
		endColor = obj.mat.color * illum;

	} else {
		vec3 sky = texture(skybox, direction).xyz;
		endColor *= sky;
	}

	endColor = mix(endColor, water_fog, 1-throughput.x);
	endColor = mix(endColor, air_fog, 1-throughput.y);

	return endColor;
}
void fresnel(vec3 I, vec3 N, float ior, inout float kr)  {
    float cosi = clamp(-1, 1, dot(I, N)); 
    float etai = 1, etat = ior; 
    if (cosi > 0) {
    	float tmp = etai;
    	etai = etat;
    	etat = tmp;
   	} 
    // Compute sini using Snell's law
    float sint = etai / etat * sqrt(max(0.f, 1 - cosi * cosi)); 
    // Total internal reflection
    if (sint >= 1) { 
        kr = 1; 
    } 
    else { 
        float cost = sqrt(max(0.f, 1 - sint * sint)); 
        cosi = abs(cosi); 
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost)); 
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost)); 
        kr = (Rs * Rs + Rp * Rp) / 2; 
    } 
    // As a consequence of the conservation of energy, transmittance is given by:
    // kt = 1 - kr;
}
vec3 RayTrace(vec3 origin, vec3 direction) {
	direction = normalize(direction);
	HitObj obj;
	vec3 endColor = vec3(1);

	vec3 throughput = vec3(1);
	vec3 throughputl = vec3(1);
	
	SendOneRay(origin, direction, obj, throughput);
	if(obj.hit) {
		float illum = 1.0;
		illum = max(dot(lightDir, obj.mat.normal), 0.3);
		illum += obj.mat.specular * 5 *max(pow(dot(reflect(lightDir,obj.mat.normal), normalize(direction)), 128), 0);

		illum *= SendLightRay(obj.hitPoint + obj.mat.normal * epsilon, lightDir, throughputl);
		
		if(obj.mat.rmax == 0 && obj.mat.transparent == 0) {
			endColor = obj.mat.color * illum;
		}
		if(obj.mat.rmax > 0) {
			HitObj obj2;
			vec3 newDir = reflect(direction, obj.mat.normal);
			vec3 throughput1 = throughput;
			SendOneRay(obj.hitPoint + newDir * epsilon, newDir, obj2, throughput1);
			vec3 ncol;
			if(obj2.hit) {
				ncol = obj2.mat.color;
				float illum = max(dot(lightDir, obj2.mat.normal), 0.3);
				illum *= SendLightRay(obj2.hitPoint + obj2.mat.normal * epsilon, lightDir, throughputl);
				ncol *= illum;
			} else {
				ncol = getSkyColor(newDir);
			}
			ncol = mix(ncol, water_fog, 1-throughput1.x);
			ncol = mix(ncol, air_fog, 1-throughput1.y);

			float amount = wt.reflection;// * FresnelReflectAmount(1, 1.6, obj.mat.normal, direction, obj.mat.rmin, obj.mat.rmax);

			endColor = mix(obj.mat.color * illum, ncol * obj.mat.tint, amount);
		}
		if(obj.mat.transparent > 0) {
			HitObj obj2;
			vec3 newDir = normalize(refract(direction, obj.mat.normal, 1./wt.ior));
			vec3 throughput2 = throughput;
			SendOneRay(obj.hitPoint + newDir * epsilon, newDir, obj2, throughput2);
			vec3 ncol;
			if(obj2.hit) {
				ncol = obj2.mat.color;
				float illum = max(dot(lightDir, obj2.mat.normal), 0.3);
				//illum *= SendLightRay(obj2.hitPoint + obj2.mat.normal * epsilon, lightDir, throughputl);
				ncol *= illum;
			} else {
				vec3 sky = texture(skybox, newDir).xyz;
				ncol = sky;
			}
			ncol = mix(ncol, water_fog, 1-throughput2.x);
			ncol = mix(ncol, air_fog, 1-throughput2.y);
			float amount = wt.refraction;// * FresnelReflectAmount(1, 1.6, obj.mat.normal, direction, obj.mat.rmin, obj.mat.rmax);
			endColor = mix(endColor * obj.mat.tint, ncol * obj.mat.tint, amount);
		}
	} else {
		endColor *= getSkyColor(direction);
	}

	endColor = mix(endColor, water_fog, 1-throughput.x);
	endColor = mix(endColor, air_fog, 1-throughput.y);
	return endColor;
}

void main() {
	vec3 origin = viewPos;
	vec2 uv;
	uv = (gl_FragCoord.xy - .5 * uResolution.xy) / uResolution.y;
	vec3 dir = normalize(vec3(uv.x, uv.y, 0.4));
    vec3 direction = normalize((vec4(dir, 0) * view).xyz);

    viewDir = (vec4(0, 1, 0, 0) * view).xyz;
	FragColor.xyz += RayTrace(origin, direction).xyz;

	return;
}