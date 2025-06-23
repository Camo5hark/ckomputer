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

void ck_vk_cmd_bfr_group_alloc(
    struct ck_vk_dev_logical_t *ck_vk_dev_logical,
    VkCommandPool vk_cmd_pool,
    const VkCommandBufferLevel vk_cmd_bfr_level,
    const uint32_t n_vk_cmd_bfrs,
    struct ck_vk_cmd_bfr_group_t *ck_vk_cmd_bfr_group
) {
    if (n_vk_cmd_bfrs > CK_MAX_N_VK_CMD_BFRS_PER_CK_VK_CMD_BFR_GROUP) {
        fprintf(
            stderr,
            "n_cmd_bfrs is greater than MAX_N_VK_CMD_BFRS_PER_VK_CMD_BFR_GROUP; "
            "n_cmd_bfrs = %u; "
            "MAX_N_VK_CMD_BFRS_PER_VK_CMD_BFR_GROUP = %u\n",
            n_vk_cmd_bfrs,
            CK_MAX_N_VK_CMD_BFRS_PER_CK_VK_CMD_BFR_GROUP
        );
        fatal_exit();
    }
    VkCommandBufferAllocateInfo cmd_bfr_alloc_info = {0};
    cmd_bfr_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_bfr_alloc_info.commandPool = vk_cmd_pool;
    cmd_bfr_alloc_info.level = vk_cmd_bfr_level;
    cmd_bfr_alloc_info.commandBufferCount = n_vk_cmd_bfrs;
    const VkResult vk_result = vkAllocateCommandBuffers(
        ck_vk_dev_logical->vk,
        &cmd_bfr_alloc_info,
        ck_vk_cmd_bfr_group->vk_cmd_bfrs
    );
    if (_VK_FAILURE) {
        fatal_exit_vk(vk_result, "vkAllocateCommandBuffers");
    }
    ck_vk_cmd_bfr_group->n_vk_cmd_bfrs = n_vk_cmd_bfrs;
    ck_vk_cmd_bfr_group->vk_cmd_pool = vk_cmd_pool;
    ck_vk_cmd_bfr_group->ck_vk_dev_logical = ck_vk_dev_logical;
}

void ck_vk_cmd_bfr_begin(
    const struct ck_vk_cmd_bfr_group_t *ck_vk_cmd_bfr_group,
    const uint32_t vk_cmd_bfr_index,
    const VkCommandBufferUsageFlags vk_cmd_bfr_usage
) {
    VkCommandBufferBeginInfo cmd_bfr_begin_info = {0};
    cmd_bfr_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmd_bfr_begin_info.flags = vk_cmd_bfr_usage;
    const VkResult vk_result = vkBeginCommandBuffer(ck_vk_cmd_bfr_group->vk_cmd_bfrs[vk_cmd_bfr_index], &cmd_bfr_begin_info);
    if (_VK_FAILURE) {
        fatal_exit_vk(vk_result, "vkBeginCommandBuffer");
    }
}

void ck_vk_cmd_bfr_end(const struct ck_vk_cmd_bfr_group_t *ck_vk_cmd_bfr_group, const uint32_t vk_cmd_bfr_index) {
    const VkResult vk_result = vkEndCommandBuffer(ck_vk_cmd_bfr_group->vk_cmd_bfrs[vk_cmd_bfr_index]);
    if (vk_result != VK_SUCCESS) {
        fprintf(
            stderr,
            "failed to end Vulkan command buffer; "
            "vk_result = %x; "
            "cmd_bfr_index = %u\n",
            vk_result,
            vk_cmd_bfr_index
        );
        fatal_exit();
    }
}

void ck_vk_cmd_bfr_group_submit(
    const struct ck_vk_cmd_bfr_group_t *ck_vk_cmd_bfr_group,
    VkQueue vk_q,
    VkFence vk_fence,
    const bool sync
) {
    VkSubmitInfo submit_info = {0};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = ck_vk_cmd_bfr_group->n_vk_cmd_bfrs;
    submit_info.pCommandBuffers = ck_vk_cmd_bfr_group->vk_cmd_bfrs;
    VkResult vk_result = vkQueueSubmit(vk_q, 1, &submit_info, vk_fence);
    if (_VK_FAILURE) {
        fatal_exit_vk(vk_result, "vkQueueSubmit");
    }
    if (sync) {
        vk_result = vkQueueWaitIdle(vk_q);
        if (_VK_FAILURE) {
            fatal_exit_vk(vk_result, "vkQueueWaitIdle");
        }
    }
}

void ck_vk_cmd_bfr_group_free(struct ck_vk_cmd_bfr_group_t *ck_vk_cmd_bfr_group) {
    vkFreeCommandBuffers(
        ck_vk_cmd_bfr_group->ck_vk_dev_logical->vk,
        ck_vk_cmd_bfr_group->vk_cmd_pool,
        ck_vk_cmd_bfr_group->n_vk_cmd_bfrs,
        ck_vk_cmd_bfr_group->vk_cmd_bfrs
    );
    memset(ck_vk_cmd_bfr_group, 0, sizeof(struct ck_vk_cmd_bfr_group_t));
}
