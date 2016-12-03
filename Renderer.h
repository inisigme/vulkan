#pragma once
#include <vulkan/vulkan.h>

// You tell GLFW to use vulkan by defining GLFW_INCLUDE_VULKAN, GLFW automatically looks for
// vulkan header in <vulkan/vulkan.h> if not included already.
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

class Renderer
{
public:
	Renderer();
	~Renderer();

	//private:
	void _InitInstance();
	void _DeInitInstance();

	void _InitDevice();
	void _DeInitDevice();

	void _SetupDebug();
	void _InitDebug();
	void _DeInitDebug();

	void _InitSwapChain();
	void _DeInitSwapChain();

	void _InitSwapchainImages();
	void _DeInitSwapChainImages();


	uint32_t					_surface_size_x			= 512;
	uint32_t					_surface_size_y			= 512;
	GLFWwindow *				window					= nullptr;
	VkSurfaceKHR				surface					= VK_NULL_HANDLE;
	VkApplicationInfo			application_info		{};
	VkInstance					_instance				= VK_NULL_HANDLE;
	VkPhysicalDevice			_gpu					= VK_NULL_HANDLE;
	VkDevice					_device					= VK_NULL_HANDLE;
	VkQueue						_queue					= VK_NULL_HANDLE;
	VkPhysicalDeviceProperties	_gpu_properties			= {};
	VkSurfaceCapabilitiesKHR	_surface_capabilities	= {};
	uint32_t					_graphics_family_index	= 0;
	VkSurfaceFormatKHR			_surface_format			= {};
	VkSwapchainKHR				_swapchain				= VK_NULL_HANDLE;
	uint32_t					_swapchain_image_count	= 2;
	std::vector<const char*>	_instance_layers;
	std::vector<const char*>	_instance_extensions;
	std::vector<const char*>	_device_layers;				// depricated
	std::vector<const char*>	_device_extensions;
	std::vector<VkImage>		_swapchain_images;
	std::vector<VkImageView>	_swapchain_image_views;
	VkDebugReportCallbackEXT				_debug_report				= VK_NULL_HANDLE;
	VkDebugReportCallbackCreateInfoEXT		debug_callback_create_info	= {};
};