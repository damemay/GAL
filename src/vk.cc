#include <algorithm>
#include <vk.hh>
#include <string.h>
#include <util.hh>
#include <vulkan/vulkan_core.h>
#include <shaderc/shaderc.h>
#include <stdexcept>
#include <set>

namespace cg {
    namespace vulkan {
        bool queue_family_indices::complete() {
            return graphics_family.has_value() && present_family.has_value();
        }

        bool instance::check_validation_layer_support() {
            uint32_t layer_count;
            vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
            VkLayerProperties available_layers[layer_count];
            vkEnumerateInstanceLayerProperties(&layer_count, available_layers);
        
            for(const auto& layer_name: validation_layers) {
                bool layer_found = false;
                for(const auto& layer_properties: available_layers) {
                    if(strcmp(layer_name, layer_properties.layerName) == 0) {
                        layer_found = true;
                        break;
                    }
                }
        
                if(!layer_found) return false;
            }
        
            return true;
        }
        
        void instance::create_instance() {
            if(enable_validation_layers && !check_validation_layer_support())
                throw std::runtime_error("Enabled validation layers are not supported");
        
            VkApplicationInfo app_info = {
                .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .pApplicationName = app_name.c_str(),
                .applicationVersion = VK_MAKE_VERSION(app_version.x, app_version.y, app_version.z),
                .pEngineName = engine_name.c_str(),
                .engineVersion = VK_MAKE_VERSION(engine_version.x, engine_version.y, engine_version.z),
                .apiVersion = VK_API_VERSION_1_0,
            };
        
            uint32_t glfw_extension_count = 0;
            const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
            std::vector<const char*> extensions(glfw_extensions, glfw_extensions+glfw_extension_count);
            extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        
            VkInstanceCreateInfo create_info = {
                .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
                .pApplicationInfo = &app_info,
                .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
                .ppEnabledExtensionNames = extensions.data(),
            };
        
            if(enable_validation_layers) {
                create_info.enabledLayerCount = validation_layers.size();
                create_info.ppEnabledLayerNames = validation_layers.data();
            } else {
                create_info.enabledLayerCount = 0;
            }
        
            if(vkCreateInstance(&create_info, nullptr, &vk_instance) != VK_SUCCESS)
                throw std::runtime_error("Failed to create Vulkan instance");
        }
        
        static bool queue_family_indices_complete(queue_family_indices* indices) {
            return indices->graphics_family != -1 && indices->present_family != -1;
        }
        
        queue_family_indices instance::find_queue_families(VkPhysicalDevice device) {
            queue_family_indices indices;
            uint32_t queue_family_count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
            VkQueueFamilyProperties queue_families[queue_family_count];
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);
        
            for(size_t i=0; i<queue_family_count; i++) {
                VkBool32 present_support = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
                if(present_support) indices.present_family = i;
                if(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphics_family = i;
                if(indices.complete()) break;
            }

            return indices;
        }
        
        bool instance::check_device_extension_support(VkPhysicalDevice device) {
            uint32_t extension_count = 0;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
            VkExtensionProperties available_extensions[extension_count];
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions);
        
            for(size_t i=0; i<device_extensions.size(); i++) {
                bool extension_found = false;
                for(size_t j=0; j<extension_count; j++) {
                    if(strcmp(device_extensions[i], available_extensions[j].extensionName) == 0) {
                        extension_found = true;
                        break;
                    }
                }
        
                if(!extension_found) return false;
            }
        
            return true;
        }
        
        void instance::query_swapchain_support(swapchain_support* details, VkPhysicalDevice device) {
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details->capabilities);
        
            uint32_t format_count = 0;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
            if(format_count != 0) {
                details->formats.resize(format_count);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details->formats.data());
            }
        
            uint32_t present_mode_count = 0;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);
            if(present_mode_count != 0) {
                details->present_modes.resize(present_mode_count);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details->present_modes.data());
            }
        }
        
        bool instance::is_device_suitable(VkPhysicalDevice device) {
            queue_family_indices indices = find_queue_families(device);
            bool extensions_support = check_device_extension_support(device);
            bool swapchain_adequate = false;
            if(extensions_support) {
                swapchain_support details;
                query_swapchain_support(&details, device);
                swapchain_adequate = details.formats.size() > 0 && details.present_modes.size() > 0;
            }
            return queue_family_indices_complete(&indices) && extensions_support && swapchain_adequate;
        }
        
        void instance::pick_physical_device() {
            physical_device = VK_NULL_HANDLE;
        
            uint32_t device_count = 0;
            vkEnumeratePhysicalDevices(vk_instance, &device_count, nullptr);
            if(device_count == 0)
                throw std::runtime_error("Vulkan is not supported on this device");
        
            VkPhysicalDevice devices[device_count];
            vkEnumeratePhysicalDevices(vk_instance, &device_count, devices);
        
            for(const auto& device: devices) {
                if(is_device_suitable(device)) {
                    physical_device = device;
                    break;
                }
            }
        
            if(physical_device == VK_NULL_HANDLE)
                throw std::runtime_error("Failed to find suitable device");
        }
        
        void instance::create_logical_device() {
            queue_family_indices indices = find_queue_families(physical_device);

            std::set<uint32_t> unique_queue_families = {
                indices.graphics_family.value(),
                indices.present_family.value()
            };
        
            uint32_t queue_families_count = unique_queue_families.size();
            VkDeviceQueueCreateInfo queue_create_infos[queue_families_count];
            float queue_priority = 1.0f;
            for(const auto& queue_family: unique_queue_families) {
                queue_create_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queue_create_infos[0].queueFamilyIndex = queue_family;
                queue_create_infos[0].queueCount = 1;
                queue_create_infos[0].pQueuePriorities = &queue_priority;
                queue_create_infos[0].pNext = nullptr;
                queue_create_infos[0].flags = 0;
            }
        
            VkPhysicalDeviceFeatures device_features = {0};
            VkDeviceCreateInfo create_info = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                .queueCreateInfoCount = queue_families_count,
                .pQueueCreateInfos = queue_create_infos,
                .enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
                .ppEnabledExtensionNames = device_extensions.data(),
                .pEnabledFeatures = &device_features,
            };
        
            if(enable_validation_layers) {
                create_info.enabledLayerCount = validation_layers.size();
                create_info.ppEnabledLayerNames = validation_layers.data();
            } else {
                create_info.enabledLayerCount = 0;
            }
        
            if(vkCreateDevice(physical_device, &create_info, nullptr, &device) != VK_SUCCESS)
                throw std::runtime_error("Failed to create Vulkan device");
        
            vkGetDeviceQueue(device, indices.graphics_family.value(), 0, &graphics_queue);
            vkGetDeviceQueue(device, indices.present_family.value(), 0, &present_queue);
        }
        
        void instance::create_surface(GLFWwindow* glfw) {
            if(glfwCreateWindowSurface(vk_instance, glfw, nullptr, &surface) != VK_SUCCESS)
                throw std::runtime_error("Failed to create Vulkan surface");
        }
        
        static void choose_swap_surface_format(VkSurfaceFormatKHR* format, swapchain_support* details) {
            for(size_t i=0; i<details->formats.size(); i++) {
                if(details->formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && details->formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    *format = details->formats[i];
                    return;
                }
            }
        
            *format = details->formats[0];
        }
        
        static void choose_swap_present_mode(VkPresentModeKHR* present_mode, swapchain_support* details) {
            for(size_t i=0; i<details->present_modes.size(); i++) {
                if(details->present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
                    *present_mode = details->present_modes[i];
                    return;
                }
            }
        
            *present_mode = VK_PRESENT_MODE_FIFO_KHR;
        }
        
        static void choose_swap_extent(GLFWwindow* glfw, VkExtent2D* extent, swapchain_support* details) {
            if(details->capabilities.currentExtent.width != UINT32_MAX) {
                *extent = details->capabilities.currentExtent;
                return;
            } else {
                int width, height;
                glfwGetFramebufferSize(glfw, &width, &height);
                VkExtent2D actual_extent = {
                    (uint32_t)width,
                    (uint32_t)height,
                };
                actual_extent.width = std::clamp(actual_extent.width, details->capabilities.minImageExtent.width, details->capabilities.maxImageExtent.width);
                actual_extent.height = std::clamp(actual_extent.height, details->capabilities.minImageExtent.height, details->capabilities.maxImageExtent.height);
                *extent = actual_extent;
            }
        }
        
        void instance::create_swapchain(GLFWwindow* glfw) {
            swapchain_support details;
            query_swapchain_support(&details, physical_device);
        
            VkSurfaceFormatKHR surface_format;
            choose_swap_surface_format(&surface_format, &details);
        
            VkPresentModeKHR present_mode;
            choose_swap_present_mode(&present_mode, &details);
        
            VkExtent2D extent;
            choose_swap_extent(glfw, &extent, &details);
        
            uint32_t image_count = details.capabilities.minImageCount + 1;
            if(details.capabilities.maxImageCount > 0 && image_count > details.capabilities.maxImageCount)
                image_count = details.capabilities.maxImageCount;
        
            VkSwapchainCreateInfoKHR create_info = {
                .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                .surface = surface,
                .minImageCount = image_count,
                .imageFormat = surface_format.format,
                .imageColorSpace = surface_format.colorSpace,
                .imageExtent = extent,
                .imageArrayLayers = 1,
                .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                .preTransform = details.capabilities.currentTransform,
                .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                .presentMode = present_mode,
                .clipped = VK_TRUE,
                .oldSwapchain = VK_NULL_HANDLE,
            };
        
            queue_family_indices indices = find_queue_families(physical_device);
            uint32_t queue_family_indices[] = {
                indices.graphics_family.value(),
                indices.present_family.value(),
            };
        
            if(indices.graphics_family != indices.present_family) {
                create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                create_info.queueFamilyIndexCount = 2;
                create_info.pQueueFamilyIndices = queue_family_indices;
            } else {
                create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                create_info.queueFamilyIndexCount = 0;
                create_info.pQueueFamilyIndices = nullptr;
            }
        
            if(vkCreateSwapchainKHR(device, &create_info, nullptr, &swapchain) != VK_SUCCESS)
                throw std::runtime_error("Failed to create swapchain");
        
            swapchain_image_format = surface_format.format;
            swapchain_extent = extent;
        
            uint32_t swapchain_images_count;
            vkGetSwapchainImagesKHR(device, swapchain, &swapchain_images_count, nullptr);
            swapchain_images.resize(swapchain_images_count);
            vkGetSwapchainImagesKHR(device, swapchain, &swapchain_images_count, swapchain_images.data());
        }
        
        void instance::create_image_views() {
            swapchain_image_views.resize(swapchain_images.size());
            for(size_t i=0; i<swapchain_image_views.size(); i++) {
                VkImageViewCreateInfo create_info = {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .image = swapchain_images[i],
                    .viewType = VK_IMAGE_VIEW_TYPE_2D,
                    .format = swapchain_image_format,
                    .components = {
                        VK_COMPONENT_SWIZZLE_IDENTITY,
                        VK_COMPONENT_SWIZZLE_IDENTITY,
                        VK_COMPONENT_SWIZZLE_IDENTITY,
                        VK_COMPONENT_SWIZZLE_IDENTITY,
                    },
                    .subresourceRange = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                    },
                };
        
                if(vkCreateImageView(device, &create_info, nullptr, &swapchain_image_views[i]) != VK_SUCCESS)
                    throw std::runtime_error("Failed to create swapchain image view");
            }
        }
        
        void instance::create_shader_module(VkShaderModule* shader, const char* code, const size_t size) {
            VkShaderModuleCreateInfo create_info = {
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .codeSize = size,
                .pCode = reinterpret_cast<const uint32_t*>(code),
            };
        
            if(vkCreateShaderModule(device, &create_info, nullptr, shader) != VK_SUCCESS)
                throw std::runtime_error("Failed to create shader module");
        }
        
        void instance::create_graphics_pipeline() {
            auto vert_code = util::read_file("../res/vert.spv");
            auto frag_code = util::read_file("../res/frag.spv");
        
            VkShaderModule vert_module, frag_module;
            create_shader_module(&vert_module, vert_code.c_str(), vert_code.size());
            create_shader_module(&frag_module, frag_code.c_str(), frag_code.size());
        
            VkPipelineShaderStageCreateInfo vert_stage_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_VERTEX_BIT,
                .module = vert_module,
                .pName = "main",
            };
        
            VkPipelineShaderStageCreateInfo frag_stage_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                .module = frag_module,
                .pName = "main",
            };
        
            VkPipelineShaderStageCreateInfo shader_stages[] = {
                vert_stage_info,
                frag_stage_info,
            };
        
            VkDynamicState dynamic_states[] = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR,
            };
        
            VkPipelineDynamicStateCreateInfo dynamic_state = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                .dynamicStateCount = 2,
                .pDynamicStates = dynamic_states,
            };
        
            VkPipelineVertexInputStateCreateInfo vert_input_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .vertexBindingDescriptionCount = 0,
                .pVertexBindingDescriptions = nullptr,
                .vertexAttributeDescriptionCount = 0,
                .pVertexAttributeDescriptions = nullptr,
            };
        
            VkPipelineInputAssemblyStateCreateInfo input_assembly = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .primitiveRestartEnable = VK_FALSE,
            };
        
            VkViewport viewport = {
                .x = 0.0f,
                .y = 0.0f,
                .width = (float)swapchain_extent.width,
                .height = (float)swapchain_extent.height,
                .minDepth = 0.0f,
                .maxDepth = 1.0f,
            };
        
            VkRect2D scissor = {
                .offset = {0},
                .extent = swapchain_extent,
            };
        
            VkPipelineViewportStateCreateInfo viewport_state = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .viewportCount = 1,
                .scissorCount = 1,
            };
        
            VkPipelineRasterizationStateCreateInfo rasterizer = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                .depthClampEnable = VK_FALSE,
                .rasterizerDiscardEnable = VK_FALSE,
                .polygonMode = VK_POLYGON_MODE_FILL,
                .cullMode = VK_CULL_MODE_BACK_BIT,
                .frontFace = VK_FRONT_FACE_CLOCKWISE,
                .depthBiasEnable = VK_FALSE,
                .lineWidth = 1.0f,
            };
        
            VkPipelineMultisampleStateCreateInfo multisampling = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
                .sampleShadingEnable = VK_FALSE,
            };
        
            VkPipelineColorBlendAttachmentState color_blend_attachment = {
                .blendEnable = VK_FALSE,
                .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            };
        
            VkPipelineColorBlendStateCreateInfo color_blending = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                .logicOpEnable = VK_FALSE,
                .attachmentCount = 1,
                .pAttachments = &color_blend_attachment,
            };
        
            VkPipelineLayoutCreateInfo pipeline_layout_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            };
        
            if(vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS)
                throw std::runtime_error("Failed to create pipeline layout");
        
            VkGraphicsPipelineCreateInfo pipeline_info = {
                .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                .stageCount = 2,
                .pStages = shader_stages,
        
                .pVertexInputState = &vert_input_info,
                .pInputAssemblyState = &input_assembly,
                .pViewportState = &viewport_state,
                .pRasterizationState = &rasterizer,
                .pMultisampleState = &multisampling,
                .pColorBlendState = &color_blending,
                .pDynamicState = &dynamic_state,
        
                .layout = pipeline_layout,
                .renderPass = render_pass,
                .subpass = 0,
            };
        
            if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline) != VK_SUCCESS)
                throw std::runtime_error("Failed to create graphics pipeline");
        
            vkDestroyShaderModule(device, vert_module, nullptr);
            vkDestroyShaderModule(device, frag_module, nullptr);
        }
        
        void instance::create_renderpass() {
            VkAttachmentDescription color_attachment = {
                .format = swapchain_image_format,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            };
        
            VkAttachmentReference color_attachment_ref = {
                .attachment = 0,
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            };
        
            VkSubpassDescription subpass = {
                .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                .colorAttachmentCount = 1,
                .pColorAttachments = &color_attachment_ref,
            };
        
            VkSubpassDependency dependency = {
                .srcSubpass = VK_SUBPASS_EXTERNAL,
                .dstSubpass = 0,
                .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask = 0,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            };
        
            VkRenderPassCreateInfo render_pass_info = {
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                .attachmentCount = 1,
                .pAttachments = &color_attachment,
                .subpassCount = 1,
                .pSubpasses = &subpass,
                .dependencyCount = 1,
                .pDependencies = &dependency,
            };
        
            if(vkCreateRenderPass(device, &render_pass_info, nullptr, &render_pass) != VK_SUCCESS)
                throw std::runtime_error("Failed to create render pass");
        }
        
        void instance::create_framebuffers() {
            swapchain_framebuffers.resize(swapchain_image_views.size());
        
            for(size_t i=0; i<swapchain_framebuffers.size(); i++) {
                VkImageView attachments[] = {
                    swapchain_image_views[i]
                };
        
                VkFramebufferCreateInfo framebuffer_info = {
                    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                    .renderPass = render_pass,
                    .attachmentCount = 1,
                    .pAttachments = attachments,
                    .width = swapchain_extent.width,
                    .height = swapchain_extent.height,
                    .layers = 1,
                };
        
                if(vkCreateFramebuffer(device, &framebuffer_info, nullptr, &swapchain_framebuffers[i]) != VK_SUCCESS)
                    throw std::runtime_error("Failed to create framebuffer");
            }
        }
        
        void instance::create_command_pool() {
            queue_family_indices indices = find_queue_families(physical_device);
        
            VkCommandPoolCreateInfo pool_info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                .queueFamilyIndex = indices.graphics_family.value(),
            };
        
            if(vkCreateCommandPool(device, &pool_info, nullptr, &command_pool) != VK_SUCCESS)
                throw std::runtime_error("Failed to create command pool");
        }
        
        void instance::create_command_buffer() {
            command_buffers.resize(max_frames_in_flight);
            VkCommandBufferAllocateInfo alloc_info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = command_pool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = static_cast<uint32_t>(command_buffers.size()),
            };
        
            if(vkAllocateCommandBuffers(device, &alloc_info, command_buffers.data()) != VK_SUCCESS)
                throw std::runtime_error("Failed to create command buffer");
        }
        
        void instance::record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index) {
            VkCommandBufferBeginInfo begin_info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            };
        
            if(vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS)
                throw std::runtime_error("Failed to begin recording command buffer");
        
            VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
            VkRenderPassBeginInfo renderpass_info = {
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                .renderPass = render_pass,
                .framebuffer = swapchain_framebuffers[image_index],
                .renderArea = {
                    {0, 0},
                    swapchain_extent
                },
                .clearValueCount = 1,
                .pClearValues = &clear_color,
            };
        
            vkCmdBeginRenderPass(command_buffer, &renderpass_info, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);
        
            VkViewport viewport = {
                .x = 0.0f,
                .y = 0.0f,
                .width = (float)swapchain_extent.width,
                .height = (float)swapchain_extent.height,
                .minDepth = 0.0f,
                .maxDepth = 1.0f,
            };
        
            vkCmdSetViewport(command_buffer, 0, 1, &viewport);
        
            VkRect2D scissor = {
                .offset = {0, 0},
                .extent = swapchain_extent,
            };
        
            vkCmdSetScissor(command_buffer, 0, 1, &scissor);
        
            vkCmdDraw(command_buffer, 3, 1, 0, 0);
        
            vkCmdEndRenderPass(command_buffer);
            if(vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
                throw std::runtime_error("Failed to end recording command buffer");
        }
        
        void instance::create_sync_objects() {
            image_available_semaphores.resize(max_frames_in_flight);
            render_finished_semaphores.resize(max_frames_in_flight);
            in_flight_fences.resize(max_frames_in_flight);

            VkSemaphoreCreateInfo semaphore_info = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            };
        
            VkFenceCreateInfo fence_info = {
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .flags = VK_FENCE_CREATE_SIGNALED_BIT,
            };
        
            for(size_t i=0; i<max_frames_in_flight; i++) {
                if(vkCreateSemaphore(device, &semaphore_info, nullptr, &image_available_semaphores[i]) != VK_SUCCESS
                    || vkCreateSemaphore(device, &semaphore_info, nullptr, &render_finished_semaphores[i]) != VK_SUCCESS
                    || vkCreateFence(device, &fence_info, nullptr, &in_flight_fences[i]) != VK_SUCCESS)
                    throw std::runtime_error("Failed to create semaphores");
            }
        }
        
        instance::instance(const std::string& name, GLFWwindow* glfw) : app_name{name} {
            create_instance();
            create_surface(glfw);
            pick_physical_device();
            create_logical_device();
            create_swapchain(glfw);
            create_image_views();
            create_renderpass();
            create_graphics_pipeline();
            create_framebuffers();
            create_command_pool();
            create_command_buffer();
            create_sync_objects();
        }
        
        instance::~instance() {
            for(size_t i=0; i<max_frames_in_flight; i++) {
                vkDestroySemaphore(device, image_available_semaphores[i], nullptr);
                vkDestroySemaphore(device, render_finished_semaphores[i], nullptr);
                vkDestroyFence(device, in_flight_fences[i], nullptr);
            }
            vkDestroyCommandPool(device, command_pool, nullptr);
            for(const auto& framebuffer: swapchain_framebuffers)
                vkDestroyFramebuffer(device, framebuffer, nullptr);
            vkDestroyPipeline(device, graphics_pipeline, nullptr);
            vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
            vkDestroyRenderPass(device, render_pass, nullptr);
            for(const auto& image_view: swapchain_image_views)
                vkDestroyImageView(device, image_view, nullptr);
            vkDestroySwapchainKHR(device, swapchain, nullptr);
            vkDestroyDevice(device, nullptr);
            vkDestroySurfaceKHR(vk_instance, surface, nullptr);
            vkDestroyInstance(vk_instance, nullptr);
        }
        
        void instance::draw() {
            vkWaitForFences(device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);
            vkResetFences(device, 1, &in_flight_fences[current_frame]);
        
            uint32_t image_index;
            vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, image_available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);
        
            vkResetCommandBuffer(command_buffers[current_frame], 0);
            record_command_buffer(command_buffers[current_frame], image_index);
        
            VkSemaphore wait_semaphores[] = {image_available_semaphores[current_frame]};
            VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
            VkSemaphore signal_semaphores[] = {render_finished_semaphores[current_frame]};
            VkSubmitInfo submit_info = {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = wait_semaphores,
                .pWaitDstStageMask = wait_stages,
                .commandBufferCount = 1,
                .pCommandBuffers = &command_buffers[current_frame],
                .signalSemaphoreCount = 1,
                .pSignalSemaphores = signal_semaphores,
            };
        
            if(vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fences[current_frame]) != VK_SUCCESS)
                throw std::runtime_error("Failed to submit queue");
        
            VkSwapchainKHR swapchains[] = {swapchain};
            VkPresentInfoKHR present_info = {
                .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = signal_semaphores,
                .swapchainCount = 1,
                .pSwapchains = swapchains,
                .pImageIndices = &image_index
            };
        
            vkQueuePresentKHR(present_queue, &present_info);

            current_frame = (current_frame+1) % max_frames_in_flight;
        }

    }
}
