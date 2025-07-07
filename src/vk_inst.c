/*
ckomputer - Vulkan C wrapper library that provides compute functionality
Copyright (C) 2025  Andrew Hall

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

License location: <project-root>/COPYING
Contact e-mail: camo5hark10@gmail.com
Git repo: https://github.com/Camo5hark/ckomputer
 */

#include <ckomputer/ckomputer.h>
#include "lib.h"
#include "vk.h"

VkInstance vk_inst = VK_NULL_HANDLE;

void vk_inst_init(const bool enable_vk_val_layer) {
    VkApplicationInfo app_info = {0};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "ckomputer";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "N/A";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;
    VkInstanceCreateInfo inst_create_info = {0};
    inst_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    inst_create_info.pApplicationInfo = &app_info;
    if (enable_vk_val_layer) {
        inst_create_info.enabledLayerCount = 1;
        static const char *val_layer_name = "VK_LAYER_KHRONOS_validation";
        inst_create_info.ppEnabledLayerNames = &val_layer_name;
    }
    const VkResult vk_result = vkCreateInstance(&inst_create_info, VK_NULL_HANDLE, &vk_inst);
    if (_VK_FAILURE) {
        fatal_exit_vk(vk_result, "vkCreateInstance");
    }
}

void vk_inst_cleanup(void) {
    if (vk_inst != VK_NULL_HANDLE) {
        vkDestroyInstance(vk_inst, VK_NULL_HANDLE);
        vk_inst = VK_NULL_HANDLE;
    }
}
