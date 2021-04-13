#include "vf3dMat44.h"

ostream& operator<< (ostream& os, v3 vec) {
	os << vec.x << ", " << vec.y << ", " << vec.z << std::endl;

	return os;
}