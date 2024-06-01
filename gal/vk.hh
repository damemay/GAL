#ifndef CG_VK_H
#define CG_VK_H

#ifdef CG_VULKAN

#include <optional>
#include <cstdint>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include <win.hh>

namespace cg {
    namespace vulkan {
#ifdef CG_DEBUG
        constexpr bool enable_validation_layers = true;
#else
        constexpr bool enable_validation_layers = false;
#endif

        constexpr uint8_t max_frames_in_flight = 2;

        struct queue_family_indices {
            std::optional<uint32_t> graphics_family;
            std::optional<uint32_t> present_family;
            bool complete();
        };

        struct swapchain_support {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> present_modes;
        };

        struct instance {
            std::string app_name;
            glm::vec3 app_version {1, 0, 0};
        
            std::string engine_name {"cg"};
            glm::vec3 engine_version {0, 1, 0};
        
            std::vector<const char*> validation_layers = {
                "VK_LAYER_KHRONOS_validation"
            };
        
            std::vector<const char*> device_extensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
            };
        
            VkSurfaceKHR surface;
            VkSwapchainKHR swapchain;
            std::vector<VkImage> swapchain_images;
            std::vector<VkImageView> swapchain_image_views;
            VkFormat swapchain_image_format;
            VkExtent2D swapchain_extent;
            std::vector<VkFramebuffer> swapchain_framebuffers;
        
            VkRenderPass render_pass;
            VkPipelineLayout pipeline_layout;
            VkPipeline graphics_pipeline;
        
            VkCommandPool command_pool;
            std::vector<VkCommandBuffer> command_buffers;
        
            std::vector<VkSemaphore> image_available_semaphores;
            std::vector<VkSemaphore> render_finished_semaphores;
            std::vector<VkFence> in_flight_fences;

            uint32_t current_frame {0};
        
            VkInstance vk_instance;
            VkPhysicalDevice physical_device;
            VkDevice device;
            VkQueue graphics_queue;
            VkQueue present_queue;

            instance(const std::string& name, GLFWwindow* glfw);
            ~instance();
            void draw();
            
            private:
                bool check_validation_layer_support();
                void create_instance();
                queue_family_indices find_queue_families(VkPhysicalDevice device);
                bool check_device_extension_support(VkPhysicalDevice device);
                void query_swapchain_support(swapchain_support* details, VkPhysicalDevice device);
                bool is_device_suitable(VkPhysicalDevice device);
                void pick_physical_device();
                void create_logical_device();
                void create_surface(GLFWwindow* glfw);
                void create_swapchain(GLFWwindow* glfw);
                void create_image_views();
                void create_shader_module(VkShaderModule* shader, const char* code, const size_t size);
                void create_graphics_pipeline();
                void create_renderpass();
                void create_framebuffers();
                void create_command_pool();
                void create_command_buffer();
                void record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index);
                void create_sync_objects();
        };
    }
}

#endif
#endif
