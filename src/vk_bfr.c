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

static ll_List *all_m_vk_bfrs = NULL;

static void create_vma_bfr(
    const struct ck_vk_dev_logical_t *vk_dev_logical,
    const VkDeviceSize size,
    const VkBufferUsageFlags usage_bfr,
    const VmaMemoryUsage usage_mem,
    struct ck_vma_bfr_t *vma_bfr
) {
    VkBufferCreateInfo bfr_create_info = {0};
    bfr_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bfr_create_info.size = size;
    bfr_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage_bfr;
    VmaAllocationCreateInfo alloc_create_info = {0};
    alloc_create_info.usage = usage_mem;
    const VkResult vk_result = vmaCreateBuffer(
        vk_dev_logical->vma_allocator,
        &bfr_create_info,
        &alloc_create_info,
        &vma_bfr->vk,
        &vma_bfr->vma_allocation,
        &vma_bfr->vma_allocation_info
    );
    if (_VK_FAILURE) {
        fatal_exit_vk(vk_result, "vmaCreateBuffer");
    }
}

static void *vma_bfr_shared_map(const struct ck_vk_bfr_t *vk_bfr) {
    void *mapping;
    const VkResult vk_result = vmaMapMemory(vk_bfr->ck_vk_dev_logical->vma_allocator, vk_bfr->ck_vma_bfr_shared.vma_allocation, &mapping);
    if (_VK_FAILURE) {
        fatal_exit_vk(vk_result, "vmaMapMemory");
    }
    return mapping;
}

static void vma_bfr_shared_write(const struct ck_vk_bfr_t *vk_bfr, const size_t data_size, const void *data) {
    void *mapping = vma_bfr_shared_map(vk_bfr);
    memcpy(mapping, data, data_size);
    vmaUnmapMemory(vk_bfr->ck_vk_dev_logical->vma_allocator, vk_bfr->ck_vma_bfr_shared.vma_allocation);
}

static void vma_bfr_shared_read(const struct ck_vk_bfr_t *vk_bfr, const size_t data_size, void *data) {
    const void *mapping = vma_bfr_shared_map(vk_bfr);
    memcpy(data, mapping, data_size);
    vmaUnmapMemory(vk_bfr->ck_vk_dev_logical->vma_allocator, vk_bfr->ck_vma_bfr_shared.vma_allocation);
}

static void vma_bfr_transfer(const struct ck_vk_bfr_t *vk_bfr, const struct ck_vma_bfr_t *vma_bfr_src, const struct ck_vma_bfr_t *vma_bfr_dst, const VkDeviceSize size) {
    struct ck_vk_cmd_bfr_group_t cmd_bfr_group;
    ck_vk_cmd_bfr_group_alloc(vk_bfr->ck_vk_dev_logical, vk_bfr->ck_vk_dev_logical->vk_cmd_pool_compute, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, &cmd_bfr_group);
    ck_vk_cmd_bfr_begin(&cmd_bfr_group, 0, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VkBufferCopy copy_region = {0};
    copy_region.srcOffset = 0;
    copy_region.dstOffset = 0;
    copy_region.size = size;
    vkCmdCopyBuffer(cmd_bfr_group.vk_cmd_bfrs[0], vma_bfr_src->vk, vma_bfr_dst->vk, 1, &copy_region);
    ck_vk_cmd_bfr_end(&cmd_bfr_group, 0);
    ck_vk_cmd_bfr_group_submit(&cmd_bfr_group, vk_bfr->ck_vk_dev_logical->m_vk_q_computes[0], VK_NULL_HANDLE, true);
    ck_vk_cmd_bfr_group_free(&cmd_bfr_group);
}

static void cleanup(struct ck_vk_bfr_t *m_vk_bfr) {
    vmaDestroyBuffer(m_vk_bfr->ck_vk_dev_logical->vma_allocator, m_vk_bfr->ck_vma_bfr_dev.vk, m_vk_bfr->ck_vma_bfr_dev.vma_allocation);
    vmaDestroyBuffer(m_vk_bfr->ck_vk_dev_logical->vma_allocator, m_vk_bfr->ck_vma_bfr_shared.vk, m_vk_bfr->ck_vma_bfr_shared.vma_allocation);
    free(m_vk_bfr);
}

struct ck_vk_bfr_t *ck_vk_bfr_create(
    struct ck_vk_dev_logical_t *ck_vk_dev_logical,
    const VkDeviceSize vk_dev_size,
    const VkBufferUsageFlags vk_bfr_usage,
    const VkDescriptorType vk_desc_type
) {
    struct ck_vk_bfr_t *m_vk_bfr = calloc(1, sizeof(struct ck_vk_bfr_t));
    if (m_vk_bfr == NULL) {
        fatal_exit_alloc("calloc", "Vulkan buffer");
    }
    if (all_m_vk_bfrs == NULL) {
        all_m_vk_bfrs = ll_makelist();
    }
    ll_add((uint64_t)m_vk_bfr, all_m_vk_bfrs);
    create_vma_bfr(ck_vk_dev_logical, vk_dev_size, vk_bfr_usage, VMA_MEMORY_USAGE_CPU_ONLY, &m_vk_bfr->ck_vma_bfr_shared);
    create_vma_bfr(ck_vk_dev_logical, vk_dev_size, vk_bfr_usage, VMA_MEMORY_USAGE_GPU_ONLY, &m_vk_bfr->ck_vma_bfr_dev);
    m_vk_bfr->ck_vk_dev_logical = ck_vk_dev_logical;
    m_vk_bfr->vk_bfr_usage = vk_bfr_usage;
    m_vk_bfr->vk_desc_type = vk_desc_type;
    return m_vk_bfr;
}

void vk_bfr_cleanup(void) {
    if (all_m_vk_bfrs == NULL) {
        return;
    }
    const ll_Node *vk_bfr_node = all_m_vk_bfrs->head;
    while (vk_bfr_node != NULL) {
        cleanup((struct ck_vk_bfr_t *)vk_bfr_node->data);
        vk_bfr_node = vk_bfr_node->next;
    }
    ll_destroy(all_m_vk_bfrs);
}

void ck_vk_bfr_write(const struct ck_vk_bfr_t *ck_vk_bfr, const size_t data_size, const void *data) {
    vma_bfr_shared_write(ck_vk_bfr, data_size, data);
    vma_bfr_transfer(ck_vk_bfr, &ck_vk_bfr->ck_vma_bfr_shared, &ck_vk_bfr->ck_vma_bfr_dev, data_size);
}

void ck_vk_bfr_read(const struct ck_vk_bfr_t *ck_vk_bfr, const size_t data_size, void *data) {
    vma_bfr_transfer(ck_vk_bfr, &ck_vk_bfr->ck_vma_bfr_dev, &ck_vk_bfr->ck_vma_bfr_shared, data_size);
    vma_bfr_shared_read(ck_vk_bfr, data_size, data);
}
