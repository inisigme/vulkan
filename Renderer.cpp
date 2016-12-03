#include "Renderer.h"
#include "Shared.h"

#include <cstdlib>
#include <assert.h>
#include <vector>

#include <iostream>
#include <sstream>

#if defined( _WIN32 )
#include <windows.h>
#endif

Renderer::Renderer()
{
	_instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	_device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	_SetupDebug();
	glfwInit();
	if (GLFW_FALSE == glfwVulkanSupported()) {
		glfwTerminate();
		exit(3);
	}
	uint32_t instance_extension_count = 0;
	const char ** instance_extensions_buffer = glfwGetRequiredInstanceExtensions(&instance_extension_count);
	for (uint32_t i = 0; i < instance_extension_count; ++i) {
		// Push back required instance extensions as well
		_instance_extensions.push_back(instance_extensions_buffer[i]);
	}
	_InitInstance();
	_InitDebug();
	_InitDevice();
	int width = 800;
	int height = 600;
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);		// This tells GLFW to not create an OpenGL context with the window
	window = glfwCreateWindow(width, height, application_info.pApplicationName, nullptr, nullptr);

	// make sure we indeed get the surface size we want.
	glfwGetFramebufferSize(window, &width, &height);

	// Create window surface, looks a lot like a Vulkan function ( and not GLFW function )
	// This is a one function solution for all operating systems. No need to hassle with the OS specifics.
	// For windows this would be vkCreateWin32SurfaceKHR() or on linux XCB window library this would be vkCreateXcbSurfaceKHR()
	
	VkResult ret = glfwCreateWindowSurface(_instance, window, nullptr, &surface);


	VkBool32 supported = false;
	vkGetPhysicalDeviceSurfaceSupportKHR(_gpu, _graphics_family_index, surface, &supported);
	if (supported)
		std::cout << "AAAAAAAAAAAAAAAAAAAAAAa" << std::endl;
	else
		std::cout << "BBBBBBBBBBBBBBBBBBBBBBBB" << std::endl;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_gpu, surface, &_surface_capabilities);
	if (_surface_capabilities.currentExtent.width < UINT32_MAX) {
		_surface_size_x = _surface_capabilities.currentExtent.width;
		_surface_size_y = _surface_capabilities.currentExtent.height;
	}

	{
		uint32_t format_count = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(_gpu, surface, &format_count, nullptr);
		if (format_count == 0) {
			assert(0 && "Surface formats missing.");
			std::exit(-1);
		}
		std::vector<VkSurfaceFormatKHR> formats(format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(_gpu, surface, &format_count, formats.data());
		if (formats[0].format == VK_FORMAT_UNDEFINED) {
			_surface_format.format = VK_FORMAT_B8G8R8A8_UNORM;
			_surface_format.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		}
		else {
			_surface_format = formats[0];
		}
	}


	if (VK_SUCCESS != ret) {		
		// couldn't create surface, exit
		glfwTerminate();
		exit(5);
	}

	_InitSwapChain();
	_InitSwapchainImages();
}

Renderer::~Renderer()
{
	_DeInitSwapChainImages();
	_DeInitSwapChain();
	_DeInitDevice();
	_DeInitDebug();
	_DeInitInstance();
	glfwTerminate();
}

void Renderer::_InitInstance()
{

	application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_info.apiVersion = VK_MAKE_VERSION(1, 0, 2);			// 1.0.2 should work on all vulkan enabled drivers.
	application_info.applicationVersion = VK_MAKE_VERSION(0, 0, 4);
	application_info.pApplicationName = "Vulkan tutorial 4";

	VkInstanceCreateInfo instance_create_info{};
	instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_create_info.pApplicationInfo = &application_info;
	instance_create_info.enabledLayerCount = _instance_layers.size();
	instance_create_info.ppEnabledLayerNames = _instance_layers.data();
	instance_create_info.enabledExtensionCount = _instance_extensions.size();
	instance_create_info.ppEnabledExtensionNames = _instance_extensions.data();
	instance_create_info.pNext = &debug_callback_create_info;

	ErrorCheck(vkCreateInstance(&instance_create_info, nullptr, &_instance));
}

void Renderer::_DeInitInstance()
{
	vkDestroyInstance(_instance, nullptr);
	_instance = nullptr;
}

void Renderer::_InitDevice()
{
	{
		uint32_t gpu_count = 0;
		vkEnumeratePhysicalDevices(_instance, &gpu_count, nullptr);
		std::vector<VkPhysicalDevice> gpu_list(gpu_count);
		vkEnumeratePhysicalDevices(_instance, &gpu_count, gpu_list.data());
		_gpu = gpu_list[0];
		vkGetPhysicalDeviceProperties(_gpu, &_gpu_properties);
	}
	{
		uint32_t family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(_gpu, &family_count, nullptr);
		std::vector<VkQueueFamilyProperties> family_property_list(family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(_gpu, &family_count, family_property_list.data());

		bool found = false;
		for (uint32_t i = 0; i < family_count; ++i) {
			if (family_property_list[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				found = true;
				_graphics_family_index = i;
			}
		}
		if (!found) {
			assert(0 && "Vulkan ERROR: Queue family supporting graphics not found.");
			std::exit(-1);
		}
	}

	{
		uint32_t layer_count = 0;
		vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
		std::vector<VkLayerProperties> layer_property_list(layer_count);
		vkEnumerateInstanceLayerProperties(&layer_count, layer_property_list.data());
		std::cout << "Instance Layers: \n";
		for (auto &i : layer_property_list) {
			std::cout << "  " << i.layerName << "\t\t | " << i.description << std::endl;
		}
		std::cout << std::endl;
	}
	{
		uint32_t layer_count = 0;
		vkEnumerateDeviceLayerProperties(_gpu, &layer_count, nullptr);
		std::vector<VkLayerProperties> layer_property_list(layer_count);
		vkEnumerateDeviceLayerProperties(_gpu, &layer_count, layer_property_list.data());
		std::cout << "Device Layers: \n";
		for (auto &i : layer_property_list) {
			std::cout << "  " << i.layerName << "\t\t | " << i.description << std::endl;
		}
		std::cout << std::endl;
	}

	float queue_priorities[]{ 1.0f };
	VkDeviceQueueCreateInfo device_queue_create_info{};
	device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	device_queue_create_info.queueFamilyIndex = _graphics_family_index;
	device_queue_create_info.queueCount = 1;
	device_queue_create_info.pQueuePriorities = queue_priorities;

	VkDeviceCreateInfo device_create_info{};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.queueCreateInfoCount = 1;
	device_create_info.pQueueCreateInfos = &device_queue_create_info;
	device_create_info.enabledLayerCount		= _device_layers.size();				// depricated
	device_create_info.ppEnabledLayerNames		= _device_layers.data();				// depricated
	device_create_info.enabledExtensionCount = _device_extensions.size();
	device_create_info.ppEnabledExtensionNames = _device_extensions.data();

	ErrorCheck(vkCreateDevice(_gpu, &device_create_info, nullptr, &_device));

	vkGetDeviceQueue(_device, _graphics_family_index, 0, &_queue);
}

void Renderer::_DeInitDevice()
{
	vkDestroySurfaceKHR(_instance, surface, nullptr);

	// destroy window using GLFW function
	glfwDestroyWindow(window);
	vkDestroyDevice(_device, nullptr);
	_device = nullptr;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugCallback(
	VkDebugReportFlagsEXT		flags,
	VkDebugReportObjectTypeEXT	obj_type,
	uint64_t					src_obj,
	size_t						location,
	int32_t						msg_code,
	const char *				layer_prefix,
	const char *				msg,
	void *						user_data
)
{
	std::ostringstream stream;
	stream << "VKDBG: ";
	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
		stream << "INFO: ";
	}
	if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
		stream << "WARNING: ";
	}
	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
		stream << "PERFORMANCE: ";
	}
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		stream << "ERROR: ";
	}
	if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
		stream << "DEBUG: ";
	}
	stream << "@[" << layer_prefix << "]: ";
	stream << msg << std::endl;
	std::cout << stream.str();

#if defined( _WIN32 )
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		MessageBox(NULL, stream.str().c_str(), "Vulkan Error!", 0);
	}
#endif

	return false;
}

void Renderer::_SetupDebug()
{
	debug_callback_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	debug_callback_create_info.pfnCallback = VulkanDebugCallback;
	debug_callback_create_info.flags =
		//		VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_ERROR_BIT_EXT |
		//		VK_DEBUG_REPORT_DEBUG_BIT_EXT |
		0;

	_instance_layers.push_back("VK_LAYER_LUNARG_standard_validation");
	/*
	//	_instance_layers.push_back( "VK_LAYER_LUNARG_threading" );
	_instance_layers.push_back( "VK_LAYER_GOOGLE_threading" );
	_instance_layers.push_back( "VK_LAYER_LUNARG_draw_state" );
	_instance_layers.push_back( "VK_LAYER_LUNARG_image" );
	_instance_layers.push_back( "VK_LAYER_LUNARG_mem_tracker" );
	_instance_layers.push_back( "VK_LAYER_LUNARG_object_tracker" );
	_instance_layers.push_back( "VK_LAYER_LUNARG_param_checker" );
	*/
	_instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	_device_layers.push_back("VK_LAYER_LUNARG_standard_validation");			// depricated

																			// push back extensions and layers you need
																			// We'll need the swapchain for sure if we want to display anything
	_device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	//	_device_layers.push_back( "VK_LAYER_LUNARG_standard_validation" );				// depricated
	/*
	//	_device_layers.push_back( "VK_LAYER_LUNARG_threading" );
	_device_layers.push_back( "VK_LAYER_GOOGLE_threading" );
	_device_layers.push_back( "VK_LAYER_LUNARG_draw_state" );
	_device_layers.push_back( "VK_LAYER_LUNARG_image" );
	_device_layers.push_back( "VK_LAYER_LUNARG_mem_tracker" );
	_device_layers.push_back( "VK_LAYER_LUNARG_object_tracker" );
	_device_layers.push_back( "VK_LAYER_LUNARG_param_checker" );
	*/
}

PFN_vkCreateDebugReportCallbackEXT		fvkCreateDebugReportCallbackEXT = nullptr;
PFN_vkDestroyDebugReportCallbackEXT		fvkDestroyDebugReportCallbackEXT = nullptr;

void Renderer::_InitDebug()
{
	fvkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(_instance, "vkCreateDebugReportCallbackEXT");
	fvkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(_instance, "vkDestroyDebugReportCallbackEXT");
	if (nullptr == fvkCreateDebugReportCallbackEXT || nullptr == fvkDestroyDebugReportCallbackEXT) {
		assert(0 && "Vulkan ERROR: Can't fetch debug function pointers.");
		std::exit(-1);
	}

	fvkCreateDebugReportCallbackEXT(_instance, &debug_callback_create_info, nullptr, &_debug_report);

	//	vkCreateDebugReportCallbackEXT( _instance, nullptr, nullptr, nullptr );
}

void Renderer::_DeInitDebug()
{
	fvkDestroyDebugReportCallbackEXT(_instance, _debug_report, nullptr);
	_debug_report = VK_NULL_HANDLE;
}

void Renderer::_InitSwapChain()
{
	if (_swapchain_image_count > _surface_capabilities.maxImageCount) 
		_swapchain_image_count = _surface_capabilities.maxImageCount;
	if (_swapchain_image_count < _surface_capabilities.minImageCount + 1)
		_swapchain_image_count = _surface_capabilities.minImageCount + 1;
	
	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
	{
		uint32_t present_mode_count = 0;
		ErrorCheck( vkGetPhysicalDeviceSurfacePresentModesKHR(_gpu, surface, &present_mode_count, nullptr));
		std::vector<VkPresentModeKHR> present_mode_list(present_mode_count);
		ErrorCheck( vkGetPhysicalDeviceSurfacePresentModesKHR(_gpu, surface, &present_mode_count, present_mode_list.data()));

		for (auto m : present_mode_list) {
			if (m == VK_PRESENT_MODE_MAILBOX_KHR) present_mode = m;
		}
			
	}

	VkSwapchainCreateInfoKHR _swapchain_create_info{};
	_swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	_swapchain_create_info.surface = surface;
	_swapchain_create_info.minImageCount = _swapchain_image_count;
	_swapchain_create_info.imageFormat = _surface_format.format;
	_swapchain_create_info.imageColorSpace = _surface_format.colorSpace;
	_swapchain_create_info.imageExtent.width = _surface_size_x;
	_swapchain_create_info.imageExtent.height = _surface_size_y;
	_swapchain_create_info.imageArrayLayers = 1;
	_swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	_swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	_swapchain_create_info.queueFamilyIndexCount = 0;
	_swapchain_create_info.pQueueFamilyIndices = nullptr;
	_swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	_swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	_swapchain_create_info.presentMode = present_mode;
	_swapchain_create_info.clipped = VK_TRUE;
	_swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

	ErrorCheck( vkCreateSwapchainKHR(_device, &_swapchain_create_info , nullptr, &_swapchain));
	ErrorCheck( vkGetSwapchainImagesKHR(_device, _swapchain, &_swapchain_image_count, nullptr));
}


void Renderer::_DeInitSwapChain()
{
	vkDestroySwapchainKHR(_device, _swapchain, nullptr);
}

void Renderer::_InitSwapchainImages()
{
	_swapchain_images.resize(_swapchain_image_count);
	_swapchain_image_views.resize(_swapchain_image_count);

	ErrorCheck(vkGetSwapchainImagesKHR(_device, _swapchain, &_swapchain_image_count, _swapchain_images.data()));

	for (uint32_t i = 0; i < _swapchain_image_count; ++i) {
		VkImageViewCreateInfo image_view_create_info{};
		image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.image = _swapchain_images[i];
		image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format = _surface_format.format;
		image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_view_create_info.subresourceRange.baseMipLevel = 0;
		image_view_create_info.subresourceRange.levelCount = 1;
		image_view_create_info.subresourceRange.baseArrayLayer = 0;
		image_view_create_info.subresourceRange.layerCount = 1;

		ErrorCheck(vkCreateImageView(_device, &image_view_create_info, nullptr, &_swapchain_image_views[i]));
	}
}

void Renderer::_DeInitSwapChainImages()
{
	for (auto view : _swapchain_image_views) {
		vkDestroyImageView(_device, view, nullptr);
	}
}
