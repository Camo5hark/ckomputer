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
#include <stdalign.h>

static ll_List *all_m_vk_comps = NULL;

static void create_shader_module(struct ck_vk_comp_t *vk_comp, const size_t code_size, uint32_t *m_code) {
    VkShaderModuleCreateInfo shader_module_create_info = {0};
    shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_create_info.codeSize = code_size;
    shader_module_create_info.pCode = m_code;
    const VkResult vk_result = vkCreateShaderModule(
        vk_comp->ck_vk_dev_logical->vk,
        &shader_module_create_info,
        NULL,
        &vk_comp->vk_shader_module
    );
    _aligned_free(m_code);
    if (_VK_FAILURE) {
        fatal_exit_vk(vk_result, "vkCreateShaderModule");
    }
}

static void create_desc_sets(
    struct ck_vk_comp_t *vk_comp,
    const VkDescriptorSetLayoutBinding *dsl_bindings
) {
    VkDescriptorSetLayoutCreateInfo dsl_create_info = {0};
    dsl_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    dsl_create_info.bindingCount = vk_comp->n_vk_dsl_bindings;
    dsl_create_info.pBindings = dsl_bindings;
    VkResult vk_result = vkCreateDescriptorSetLayout(
        vk_comp->ck_vk_dev_logical->vk,
        &dsl_create_info,
        NULL,
        &vk_comp->vk_dsl
    );
    if (_VK_FAILURE) {
        fatal_exit_vk(vk_result, "vkCreateDescriptorSetLayout");
    }
    VkDescriptorSetAllocateInfo desc_set_alloc_info = {0};
    desc_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    desc_set_alloc_info.descriptorPool = vk_comp->ck_vk_dev_logical->vk_desc_pool;
    desc_set_alloc_info.descriptorSetCount = 1;
    desc_set_alloc_info.pSetLayouts = &vk_comp->vk_dsl;
    vk_result = vkAllocateDescriptorSets(vk_comp->ck_vk_dev_logical->vk, &desc_set_alloc_info, &vk_comp->vk_desc_set);
    if (_VK_FAILURE) {
        fatal_exit_vk(vk_result, "vkAllocateDescriptorSets");
    }
}

static void create_pipeline(struct ck_vk_comp_t *vk_comp) {
    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {0};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.setLayoutCount = 1;
    pipeline_layout_create_info.pSetLayouts = &vk_comp->vk_dsl;
    VkResult vk_result = vkCreatePipelineLayout(vk_comp->ck_vk_dev_logical->vk, &pipeline_layout_create_info, NULL, &vk_comp->vk_pipeline_layout);
    if (_VK_FAILURE) {
        fatal_exit_vk(vk_result, "vkCreatePipelineLayout");
    }
    VkComputePipelineCreateInfo comp_pipeline_create_info = {0};
    comp_pipeline_create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    comp_pipeline_create_info.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    comp_pipeline_create_info.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    comp_pipeline_create_info.stage.module = vk_comp->vk_shader_module;
    comp_pipeline_create_info.stage.pName = "main";
    comp_pipeline_create_info.layout = vk_comp->vk_pipeline_layout;
    vk_result = vkCreateComputePipelines(vk_comp->ck_vk_dev_logical->vk, VK_NULL_HANDLE, 1, &comp_pipeline_create_info, NULL, &vk_comp->vk_pipeline);
    vkDestroyShaderModule(vk_comp->ck_vk_dev_logical->vk, vk_comp->vk_shader_module, NULL);
    vk_comp->vk_shader_module = VK_NULL_HANDLE;
    if (_VK_FAILURE) {
        fatal_exit_vk(vk_result, "vkCreateComputePipelines");
    }
}

static void cleanup(struct ck_vk_comp_t *m_vk_comp) {
    if (m_vk_comp->vk_pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(m_vk_comp->ck_vk_dev_logical->vk, m_vk_comp->vk_pipeline, NULL);
    }
    if (m_vk_comp->vk_pipeline_layout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(m_vk_comp->ck_vk_dev_logical->vk, m_vk_comp->vk_pipeline_layout, NULL);
    }
    if (m_vk_comp->vk_dsl != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(m_vk_comp->ck_vk_dev_logical->vk, m_vk_comp->vk_dsl, NULL);
    }
    if (m_vk_comp->vk_shader_module != VK_NULL_HANDLE) {
        vkDestroyShaderModule(m_vk_comp->ck_vk_dev_logical->vk, m_vk_comp->vk_shader_module, NULL);
    }
    free(m_vk_comp);
}

struct ck_vk_comp_t *ck_vk_comp_create(
    struct ck_vk_dev_logical_t *vk_dev_logical,
    const size_t code_size,
    uint32_t *m_code,
    const uint32_t n_desc_sets,
    const VkDescriptorSetLayoutBinding *dsl_bindings
) {
    struct ck_vk_comp_t *m_vk_comp = calloc(1, sizeof(struct ck_vk_comp_t));
    if (m_vk_comp == NULL) {
        fatal_exit_alloc("calloc", "Vulkan computer");
    }
    if (all_m_vk_comps == NULL) {
        all_m_vk_comps = ll_makelist();
    }
    ll_add((uint64_t)m_vk_comp, all_m_vk_comps);
    m_vk_comp->ck_vk_dev_logical = vk_dev_logical;
    create_shader_module(m_vk_comp, code_size, m_code);
    m_vk_comp->n_vk_dsl_bindings = n_desc_sets;
    create_desc_sets(m_vk_comp, dsl_bindings);
    create_pipeline(m_vk_comp);
    return m_vk_comp;
}

struct ck_vk_comp_t *ck_vk_comp_create_from_stream(
    struct ck_vk_dev_logical_t *vk_dev_logical,
    FILE *in_code,
    const uint32_t n_desc_sets,
    const VkDescriptorSetLayoutBinding *dsl_bindings
) {
    fseek(in_code, 0, SEEK_END);
    const size_t code_size = ftell(in_code);
    rewind(in_code);
    if (code_size == 0) {
        fclose(in_code);
        fprintf(stderr, "size of Vulkan compute shader code stream is 0\n");
        fatal_exit();
    }
    char *m_code = _aligned_malloc(code_size * sizeof(char), alignof(uint32_t));
    if (m_code == NULL) {
        fclose(in_code);
        fatal_exit_alloc("_aligned_malloc", "Vulkan compute shader code stream");
    }
    if (fread(m_code, sizeof(char), code_size, in_code) != code_size) {
        fprintf(
            stderr,
            "failed to read Vulkan compute shader code stream; "
            "ferror = %x\n",
            ferror(in_code)
        );
        fatal_exit();
    }
    fclose(in_code);
    return ck_vk_comp_create(
        vk_dev_logical,
        code_size,
        (uint32_t *)m_code,
        n_desc_sets,
        dsl_bindings
    );
}

void ck_vk_comp_dispatch(const struct ck_vk_comp_t *vk_comp, const uint32_t n_groups_x, const uint32_t n_groups_y, const uint32_t n_groups_z, VkFence fence, const bool sync) {
    struct ck_vk_cmd_bfr_group_t vk_cmd_bfr_group;
    ck_vk_cmd_bfr_group_alloc(vk_comp->ck_vk_dev_logical, vk_comp->ck_vk_dev_logical->vk_cmd_pool_compute, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, &vk_cmd_bfr_group);
    ck_vk_cmd_bfr_begin(&vk_cmd_bfr_group, 0, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    vkCmdBindPipeline(vk_cmd_bfr_group.vk_cmd_bfrs[0], VK_PIPELINE_BIND_POINT_COMPUTE, vk_comp->vk_pipeline);
    vkCmdBindDescriptorSets(vk_cmd_bfr_group.vk_cmd_bfrs[0], VK_PIPELINE_BIND_POINT_COMPUTE, vk_comp->vk_pipeline_layout, 0, 1, &vk_comp->vk_desc_set, 0, NULL);
    vkCmdDispatch(vk_cmd_bfr_group.vk_cmd_bfrs[0], n_groups_x, n_groups_y, n_groups_z);
    ck_vk_cmd_bfr_end(&vk_cmd_bfr_group, 0);
    ck_vk_cmd_bfr_group_submit(&vk_cmd_bfr_group, vk_comp->ck_vk_dev_logical->m_vk_q_computes[0], fence, sync);
    ck_vk_cmd_bfr_group_free(&vk_cmd_bfr_group);
}

void ck_vk_comp_wds_group_init(struct ck_vk_comp_wds_group_t *ck_vk_comp_wds_group, struct ck_vk_comp_t *ck_vk_comp) {
    memset(ck_vk_comp_wds_group, 0, sizeof(struct ck_vk_comp_wds_group_t));
    ck_vk_comp_wds_group->ck_vk_comp = ck_vk_comp;
}

void ck_vk_comp_wds_group_add_bfr(struct ck_vk_comp_wds_group_t *ck_vk_comp_wds_group, const struct ck_vk_bfr_t *ck_vk_bfr, const uint32_t desc_set_binding) {
    VkDescriptorBufferInfo *vk_dbi = &ck_vk_comp_wds_group->vk_dbis[ck_vk_comp_wds_group->n_vk_dbis];
    ++ck_vk_comp_wds_group->n_vk_dbis;
    vk_dbi->buffer = ck_vk_bfr->ck_vma_bfr_dev.vk;
    // struct should be zero'd anyways
    //vk_dbi->offset = 0;
    vk_dbi->range = VK_WHOLE_SIZE;
    VkWriteDescriptorSet *vk_wds = &ck_vk_comp_wds_group->vk_wdss[ck_vk_comp_wds_group->n_vk_wdss];
    ++ck_vk_comp_wds_group->n_vk_wdss;
    vk_wds->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vk_wds->dstSet = ck_vk_comp_wds_group->ck_vk_comp->vk_desc_set;
    vk_wds->dstBinding = desc_set_binding;
    //vk_wds->dstArrayElement = 0;
    vk_wds->descriptorType = ck_vk_bfr->vk_desc_type;
    vk_wds->descriptorCount = 1;
    vk_wds->pBufferInfo = vk_dbi;
}

void ck_vk_comp_wds_group_submit(const struct ck_vk_comp_wds_group_t *ck_vk_comp_wds_group) {
    vkUpdateDescriptorSets(ck_vk_comp_wds_group->ck_vk_comp->ck_vk_dev_logical->vk, ck_vk_comp_wds_group->n_vk_wdss, ck_vk_comp_wds_group->vk_wdss, 0, NULL);
}

void vk_comp_cleanup(void) {
    if (all_m_vk_comps == NULL) {
        return;
    }
    const ll_Node *comp_node = all_m_vk_comps->head;
    while (comp_node != NULL) {
        cleanup((struct ck_vk_comp_t *)comp_node->data);
        comp_node = comp_node->next;
    }
    ll_destroy(all_m_vk_comps);
}
