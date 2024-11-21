#include "Renderer.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <Windows.h>

int main() {
	Renderer render_engine(800, 800);
	render_engine.start();

	return 0;
}