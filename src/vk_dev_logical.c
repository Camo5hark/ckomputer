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
#define VMA_VULKAN_VERSION 100000
#include <vma/vk_mem_alloc.h>

static ll_List *all_m_dev_logicals = NULL;

static void init_cmd_pool_compute(struct ck_vk_dev_logical_t *dev_logical) {
    VkCommandPoolCreateInfo cmd_pool_create_info = {0};
    cmd_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmd_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmd_pool_create_info.queueFamilyIndex = dev_logical->ck_vk_dev_phys->qfi_compute;
    const VkResult vk_result = vkCreateCommandPool(
        dev_logical->vk,
        &cmd_pool_create_info,
        NULL,
        &dev_logical->vk_cmd_pool_compute
    );
    if (_VK_FAILURE) {
        fatal_exit_vk(vk_result, "vkCreateCommandPool");
    }
}

static void init_allocator(struct ck_vk_dev_logical_t *dev_logical) {
    VmaAllocatorCreateInfo allocator_create_info = {0};
    allocator_create_info.instance = vk_inst;
    allocator_create_info.physicalDevice = dev_logical->ck_vk_dev_phys->vk;
    allocator_create_info.device = dev_logical->vk;
    const VkResult vk_result = vmaCreateAllocator(&allocator_create_info, &dev_logical->vma_allocator);
    if (_VK_FAILURE) {
        fatal_exit_vk(vk_result, "vmaCreateAllocator");
    }
}

static void init_desc_pool(
    struct ck_vk_dev_logical_t *dev_logical,
    const uint32_t max_n_desc_uniform,
    const uint32_t max_n_desc_storage,
    const uint32_t max_n_desc_sets
) {
    const VkDescriptorPoolSize desc_pool_sizes[] = {
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount =
                max_n_desc_uniform == 0 ?
                CK_DEFAULT_MAX_N_VK_DESC_UNIFORM_PER_CK_VK_DEV_LOGICAL_DESC_POOL :
                max_n_desc_uniform
        },
        {
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount =
                max_n_desc_storage == 0 ?
                CK_DEFAULT_MAX_N_VK_DESC_STORAGE_PER_CK_VK_DEV_LOGICAL_DESC_POOL :
                max_n_desc_storage
        }
    };
    VkDescriptorPoolCreateInfo desc_pool_create_info = {0};
    desc_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    desc_pool_create_info.poolSizeCount = 2;
    desc_pool_create_info.pPoolSizes = desc_pool_sizes;
    desc_pool_create_info.maxSets =
        max_n_desc_sets == 0 ?
        CK_DEFAULT_MAX_N_VK_DESC_SETS_PER_CK_VK_DEV_LOGICAL_DESC_POOL :
        max_n_desc_sets;
    const VkResult vk_result = vkCreateDescriptorPool(
        dev_logical->vk,
        &desc_pool_create_info,
        NULL,
        &dev_logical->vk_desc_pool
    );
    if (_VK_FAILURE) {
        fatal_exit_vk(vk_result, "vkCreateDescriptorPool");
    }
}

static void cleanup(struct ck_vk_dev_logical_t *m_dev_logical) {
    if (m_dev_logical->vk_desc_pool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(m_dev_logical->vk, m_dev_logical->vk_desc_pool, NULL);
    }
    if (m_dev_logical->vma_allocator != NULL) {
        vmaDestroyAllocator(m_dev_logical->vma_allocator);
    }
    if (m_dev_logical->vk_cmd_pool_compute != VK_NULL_HANDLE) {
        vkDestroyCommandPool(m_dev_logical->vk, m_dev_logical->vk_cmd_pool_compute, NULL);
    }
    if (m_dev_logical->m_vk_q_computes != NULL) {
        free(m_dev_logical->m_vk_q_computes);
    }
    if (m_dev_logical->vk != VK_NULL_HANDLE) {
        vkDestroyDevice(m_dev_logical->vk, NULL);
    }
    free(m_dev_logical);
}

struct ck_vk_dev_logical_t *ck_vk_dev_logical_create(
    const uint32_t n_q_computes,
    const float *q_compute_prios,
    struct ck_vk_dev_phys_t *ck_vk_dev_phys,
    const uint32_t max_n_vk_desc_uniforms,
    const uint32_t max_n_vk_desc_storages,
    const uint32_t max_n_vk_desc_sets
) {
    struct ck_vk_dev_logical_t *m_dev_logical = calloc(1, sizeof(struct ck_vk_dev_logical_t));
    if (m_dev_logical == NULL) {
        fatal_exit_alloc("calloc", "logical device");
    }
    if (all_m_dev_logicals == NULL) {
        all_m_dev_logicals = ll_makelist();
    }
    ll_add((uint64_t)m_dev_logical, all_m_dev_logicals);
    VkDeviceQueueCreateInfo queue_create_info = {0};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueCount = n_q_computes;
    queue_create_info.queueFamilyIndex = ck_vk_dev_phys->qfi_compute;
    float *m_queue_compute_prios = NULL;
    uint32_t i;
    if (q_compute_prios != NULL) {
        queue_create_info.pQueuePriorities = q_compute_prios;
    } else {
        m_queue_compute_prios = malloc(n_q_computes * sizeof(float));
        for (i = 0; i < n_q_computes; ++i) {
            m_queue_compute_prios[i] = 1.0F;
        }
        queue_create_info.pQueuePriorities = m_queue_compute_prios;
    }
    VkDeviceCreateInfo dev_logical_create_info = {0};
    dev_logical_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dev_logical_create_info.queueCreateInfoCount = 1;
    dev_logical_create_info.pQueueCreateInfos = &queue_create_info;
    dev_logical_create_info.pEnabledFeatures = &ck_vk_dev_phys->vk_feats;
    const VkResult vk_result = vkCreateDevice(ck_vk_dev_phys->vk, &dev_logical_create_info, NULL, &m_dev_logical->vk);
    if (m_queue_compute_prios != NULL) {
        free(m_queue_compute_prios);
    }
    if (_VK_FAILURE) {
        fatal_exit_vk(vk_result, "vkCreateDevice");
    }
    m_dev_logical->n_vk_q_computes = n_q_computes;
    m_dev_logical->m_vk_q_computes = malloc(n_q_computes * sizeof(VkQueue));
    for (i = 0; i < n_q_computes; ++i) {
        vkGetDeviceQueue(
            m_dev_logical->vk,
            ck_vk_dev_phys->qfi_compute,
            i,
            &m_dev_logical->m_vk_q_computes[i]
        );
    }
    m_dev_logical->ck_vk_dev_phys = ck_vk_dev_phys;
    init_cmd_pool_compute(m_dev_logical);
    init_allocator(m_dev_logical);
    init_desc_pool(m_dev_logical, max_n_vk_desc_uniforms, max_n_vk_desc_storages, max_n_vk_desc_sets);
    return m_dev_logical;
}

void vk_dev_logical_cleanup(void) {
    if (all_m_dev_logicals == NULL) {
        return;
    }
    const ll_Node *dev_logical_node = all_m_dev_logicals->head;
    while (dev_logical_node != NULL) {
        cleanup((struct ck_vk_dev_logical_t *)dev_logical_node->data);
        dev_logical_node = dev_logical_node->next;
    }
    ll_destroy(all_m_dev_logicals);
}
