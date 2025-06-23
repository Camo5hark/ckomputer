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
#include "vk.h"

uint32_t n_ck_vk_dev_phys = 0;
struct ck_vk_dev_phys_t *m_ck_vk_dev_phys = NULL;

static void init_qfi_compute(void) {
    for (uint32_t i = 0; i < n_ck_vk_dev_phys; ++i) {
        uint32_t n_qfs = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_ck_vk_dev_phys[i].vk, &n_qfs, NULL);
        if (n_qfs == 0) {
            fprintf(
                stderr,
                "failed to find Vulkan-supported physical device queue families; "
                "physical device name = \"%s\"\n",
                m_ck_vk_dev_phys[i].vk_props.deviceName
            );
            fatal_exit();
        }
        VkQueueFamilyProperties *m_qf_props = malloc(n_qfs * sizeof(VkQueueFamilyProperties));
        if (m_qf_props == NULL) {
            fatal_exit_alloc("malloc", "m_qf_props");
        }
        vkGetPhysicalDeviceQueueFamilyProperties(m_ck_vk_dev_phys[i].vk, &n_qfs, m_qf_props);
        uint64_t qfi_compute = UINT64_MAX;
        for (uint32_t j = 0; j < n_qfs; ++j) {
            if (m_qf_props[j].queueFlags & VK_QUEUE_COMPUTE_BIT) {
                qfi_compute = j;
                break;
            }
        }
        free(m_qf_props);
        if (qfi_compute == UINT64_MAX) {
            fprintf(
                stderr,
                "failed to find Vulkan-supported physical device compute queue family; "
                "physical device name = \"%s\"\n",
                m_ck_vk_dev_phys[i].vk_props.deviceName
            );
            fatal_exit();
        }
        m_ck_vk_dev_phys[i].qfi_compute = qfi_compute;
    }
}

void vk_dev_phys_init(void) {
    vkEnumeratePhysicalDevices(vk_inst, &n_ck_vk_dev_phys, NULL);
    if (n_ck_vk_dev_phys == 0) {
        fprintf(stderr, "failed to find Vulkan-supported physical devices\n");
        fatal_exit();
    }
#ifdef DEBUG
    printf("DEBUG: found %u Vulkan-supported physical devices\n", n_ck_vk_dev_phys);
#endif
    m_ck_vk_dev_phys = malloc(n_ck_vk_dev_phys * sizeof(struct ck_vk_dev_phys_t));
    if (m_ck_vk_dev_phys == NULL) {
        fatal_exit_alloc("malloc", "m_dev_phys");
    }
    VkPhysicalDevice *m_vk_dev_phys_handles = malloc(n_ck_vk_dev_phys * sizeof(VkPhysicalDevice));
    if (m_vk_dev_phys_handles == NULL) {
        fatal_exit_alloc("malloc", "m_vk_dev_phys_handles");
    }
    vkEnumeratePhysicalDevices(vk_inst, &n_ck_vk_dev_phys, m_vk_dev_phys_handles);
    for (uint32_t i = 0; i < n_ck_vk_dev_phys; ++i) {
        VkPhysicalDevice handle = m_vk_dev_phys_handles[i];
        m_ck_vk_dev_phys[i].vk = handle;
        vkGetPhysicalDeviceProperties(handle, &m_ck_vk_dev_phys[i].vk_props);
        vkGetPhysicalDeviceFeatures(handle, &m_ck_vk_dev_phys[i].vk_feats);
    }
    free(m_vk_dev_phys_handles);
    init_qfi_compute();
#ifdef DEBUG
    ck_vk_dev_phys_print_all(true);
    putchar('\n');
#endif
}

void vk_dev_phys_cleanup(void) {
    if (m_ck_vk_dev_phys != NULL) {
        free(m_ck_vk_dev_phys);
    }
}

void ck_vk_dev_phys_print(const struct ck_vk_dev_phys_t *vk_dev_phys, const bool limits) {
    printf(
        "--- VULKAN PHYSICAL DEVICE INFO ---\n"
        "  - name: \"%s\"\n"
        "  - vendor ID: %u\n"
        "  - ID: %u\n"
        "  - driver version: %u\n",
        vk_dev_phys->vk_props.deviceName,
        vk_dev_phys->vk_props.vendorID,
        vk_dev_phys->vk_props.deviceID,
        vk_dev_phys->vk_props.driverVersion
    );
    if (limits) {
        printf(
            "  --- COMPUTE LIMITS ---\n"
            "    - max shared memory size: %u\n"
            "    - max work group count: %u %u %u\n"
            "    - max work group invocations: %u\n"
            "    - max work group size: %u %u %u\n"
            "  --- END COMPUTE LIMITS ---\n",
            vk_dev_phys->vk_props.limits.maxComputeSharedMemorySize,
            vk_dev_phys->vk_props.limits.maxComputeWorkGroupCount[0],
            vk_dev_phys->vk_props.limits.maxComputeWorkGroupCount[1],
            vk_dev_phys->vk_props.limits.maxComputeWorkGroupCount[2],
            vk_dev_phys->vk_props.limits.maxComputeWorkGroupInvocations,
            vk_dev_phys->vk_props.limits.maxComputeWorkGroupSize[0],
            vk_dev_phys->vk_props.limits.maxComputeWorkGroupSize[1],
            vk_dev_phys->vk_props.limits.maxComputeWorkGroupSize[2]
        );
    }
    puts("--- END VULKAN PHYSICAL DEVICE INFO ---");
}

void ck_vk_dev_phys_print_all(const bool limits) {
    for (uint32_t i = 0; i < n_ck_vk_dev_phys; ++i) {
        printf("\n--- VULKAN PHYSICAL DEVICE INDEX: [%u]\n", i);
        ck_vk_dev_phys_print(&m_ck_vk_dev_phys[i], limits);
    }
}
