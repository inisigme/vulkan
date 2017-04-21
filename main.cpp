#include <vulkan/vulkan.h>

// You tell GLFW to use vulkan by defining GLFW_INCLUDE_VULKAN, GLFW automatically looks for
// vulkan header in <vulkan/vulkan.h> if not included already.
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <Windows.h>
#include "Renderer.h"

int main()
{	
	Renderer *kon = new Renderer();





	Sleep(10000);
	delete kon;
	return 0;
}
