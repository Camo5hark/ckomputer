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

#ifndef CKOMPUTER_H
#define CKOMPUTER_H

#include <vulkan/vulkan.h>
#include <stdint.h>
#define VMA_VULKAN_VERSION 100000
#include <vma/vk_mem_alloc.h>
#include <stdbool.h>
#include <stdio.h>

#define CK_MAX_N_VK_CMD_BFRS_PER_CK_VK_CMD_BFR_GROUP 32

#define CK_DEFAULT_MAX_N_VK_DESC_UNIFORM_PER_CK_VK_DEV_LOGICAL_DESC_POOL 4096
#define CK_DEFAULT_MAX_N_VK_DESC_STORAGE_PER_CK_VK_DEV_LOGICAL_DESC_POOL 4096
#define CK_DEFAULT_MAX_N_VK_DESC_SETS_PER_CK_VK_DEV_LOGICAL_DESC_POOL 4096

#define CK_MAX_N_VK_WRITE_DESC_SETS_PER_CK_VK_COMP_WDS_GROUP 16

#define CK_INIT_FLAG_ENABLE_VK_VAL_LAYER 0x1

struct ck_vk_dev_phys_t {
    VkPhysicalDevice vk;
    VkPhysicalDeviceProperties vk_props;
    VkPhysicalDeviceFeatures vk_feats;
    uint32_t qfi_compute;
};

struct ck_vk_dev_logical_t {
    VkDevice vk;
    uint32_t n_vk_q_computes;
    VkQueue *m_vk_q_computes;
    struct ck_vk_dev_phys_t *ck_vk_dev_phys;
    VkCommandPool vk_cmd_pool_compute;
    VmaAllocator vma_allocator;
    VkDescriptorPool vk_desc_pool;
};

struct ck_vma_bfr_t {
    VkBuffer vk;
    VmaAllocation vma_allocation;
    VmaAllocationInfo vma_allocation_info;
};

struct ck_vk_bfr_t {
    struct ck_vma_bfr_t ck_vma_bfr_shared;
    struct ck_vma_bfr_t ck_vma_bfr_dev;
    VkBufferUsageFlags vk_bfr_usage;
    VkDescriptorType vk_desc_type;
    struct ck_vk_dev_logical_t *ck_vk_dev_logical;
};

struct ck_vk_cmd_bfr_group_t {
    uint32_t n_vk_cmd_bfrs;
    VkCommandBuffer vk_cmd_bfrs[CK_MAX_N_VK_CMD_BFRS_PER_CK_VK_CMD_BFR_GROUP];
    VkCommandPool vk_cmd_pool;
    struct ck_vk_dev_logical_t *ck_vk_dev_logical;
};

struct ck_vk_comp_t {
    struct ck_vk_dev_logical_t *ck_vk_dev_logical;
    VkShaderModule vk_shader_module;
    uint32_t n_vk_dsl_bindings;
    VkDescriptorSetLayout vk_dsl;
    VkDescriptorSet vk_desc_set;
    VkPipelineLayout vk_pipeline_layout;
    VkPipeline vk_pipeline;
};

struct ck_vk_comp_wds_group_t {
    struct ck_vk_comp_t *ck_vk_comp;
    uint32_t n_vk_dbis;
    VkDescriptorBufferInfo vk_dbis[CK_MAX_N_VK_WRITE_DESC_SETS_PER_CK_VK_COMP_WDS_GROUP];
    uint32_t n_vk_wdss;
    VkWriteDescriptorSet vk_wdss[CK_MAX_N_VK_WRITE_DESC_SETS_PER_CK_VK_COMP_WDS_GROUP];
};

extern VkInstance vk_inst;
extern uint32_t n_ck_vk_dev_phys;
extern struct ck_vk_dev_phys_t *m_ck_vk_dev_phys;

void ck_init(uint32_t flags);
void ck_cleanup(void);

void ck_vk_dev_phys_print(const struct ck_vk_dev_phys_t *vk_dev_phys, bool limits);
void ck_vk_dev_phys_print_all(bool limits);

struct ck_vk_dev_logical_t *ck_vk_dev_logical_create(
    uint32_t n_q_computes,
    const float *q_compute_prios,
    struct ck_vk_dev_phys_t *ck_vk_dev_phys,
    uint32_t max_n_vk_desc_uniforms,
    uint32_t max_n_vk_desc_storages,
    uint32_t max_n_vk_desc_sets
);

void ck_vk_cmd_bfr_group_alloc(
    struct ck_vk_dev_logical_t *ck_vk_dev_logical,
    VkCommandPool vk_cmd_pool,
    VkCommandBufferLevel vk_cmd_bfr_level,
    uint32_t n_vk_cmd_bfrs,
    struct ck_vk_cmd_bfr_group_t *ck_vk_cmd_bfr_group
);
void ck_vk_cmd_bfr_begin(
    const struct ck_vk_cmd_bfr_group_t *ck_vk_cmd_bfr_group,
    uint32_t vk_cmd_bfr_index,
    VkCommandBufferUsageFlags vk_cmd_bfr_usage
);
void ck_vk_cmd_bfr_end(
    const struct ck_vk_cmd_bfr_group_t *ck_vk_cmd_bfr_group,
    uint32_t vk_cmd_bfr_index
);
void ck_vk_cmd_bfr_group_submit(
    const struct ck_vk_cmd_bfr_group_t *ck_vk_cmd_bfr_group,
    VkQueue vk_q,
    VkFence vk_fence,
    bool sync
);
void ck_vk_cmd_bfr_group_free(struct ck_vk_cmd_bfr_group_t *ck_vk_cmd_bfr_group);

struct ck_vk_bfr_t *ck_vk_bfr_create(
    struct ck_vk_dev_logical_t *ck_vk_dev_logical,
    VkDeviceSize vk_dev_size,
    VkBufferUsageFlags vk_bfr_usage,
    VkDescriptorType vk_desc_type
);
void ck_vk_bfr_write(const struct ck_vk_bfr_t *ck_vk_bfr, size_t data_size, const void *data);
void ck_vk_bfr_read(const struct ck_vk_bfr_t *ck_vk_bfr, size_t data_size, void *data);

struct ck_vk_comp_t *ck_vk_comp_create(
    struct ck_vk_dev_logical_t *vk_dev_logical,
    size_t code_size,
    uint32_t *m_code,
    uint32_t n_desc_sets,
    const VkDescriptorSetLayoutBinding *dsl_bindings
);
struct ck_vk_comp_t *ck_vk_comp_create_from_stream(
    struct ck_vk_dev_logical_t *vk_dev_logical,
    FILE *in_code,
    uint32_t n_desc_sets,
    const VkDescriptorSetLayoutBinding *dsl_bindings
);
void ck_vk_comp_dispatch(
    const struct ck_vk_comp_t *vk_comp,
    uint32_t n_groups_x,
    uint32_t n_groups_y,
    uint32_t n_groups_z,
    VkFence fence,
    bool sync
);

void ck_vk_comp_wds_group_init(struct ck_vk_comp_wds_group_t *ck_vk_comp_wds_group, struct ck_vk_comp_t *ck_vk_comp);
void ck_vk_comp_wds_group_add_bfr(struct ck_vk_comp_wds_group_t *ck_vk_comp_wds_group, const struct ck_vk_bfr_t *ck_vk_bfr, uint32_t desc_set_binding);
void ck_vk_comp_wds_group_submit(const struct ck_vk_comp_wds_group_t *ck_vk_comp_wds_group);

#endif
