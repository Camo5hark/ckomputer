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
#include <stdio.h>

int main() {
    ck_init();

    struct ck_vk_dev_logical_t *m_dev_logical = ck_vk_dev_logical_create(1, NULL, &m_ck_vk_dev_phys[1], 0, 0, 0);

    struct ck_vk_bfr_t *m_bfr_mul = ck_vk_bfr_create(m_dev_logical, sizeof(float), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    const float mul = 0.5F;
    ck_vk_bfr_write(m_bfr_mul, sizeof(float), &mul);
    struct ck_vk_bfr_t *m_bfr_in = ck_vk_bfr_create(m_dev_logical, 10 * sizeof(float), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    const float dat_in[] = {1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F, 10.0F};
    ck_vk_bfr_write(m_bfr_in, 10 * sizeof(float), &dat_in);
    struct ck_vk_bfr_t *m_bfr_out = ck_vk_bfr_create(m_dev_logical, 10 * sizeof(float), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

    const VkDescriptorSetLayoutBinding dsl_bindings[] = {
        {
            .binding = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            .pImmutableSamplers = NULL
        },
        {
            .binding = 1,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            .pImmutableSamplers = NULL
        },
        {
            .binding = 2,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            .pImmutableSamplers = NULL
        }
    };
    FILE *code_in = fopen("./comps/a_comp.comp.spv", "rb");
    struct ck_vk_comp_t *m_vk_comp = ck_vk_comp_create_from_stream(m_dev_logical, code_in, 3, dsl_bindings);

    struct ck_vk_comp_wds_group_t comp_wds_group = {0};
    ck_vk_comp_wds_group_init(&comp_wds_group, m_vk_comp);
    ck_vk_comp_wds_group_add_bfr(&comp_wds_group, m_bfr_mul, 0);
    ck_vk_comp_wds_group_add_bfr(&comp_wds_group, m_bfr_in, 1);
    ck_vk_comp_wds_group_add_bfr(&comp_wds_group, m_bfr_out, 2);
    ck_vk_comp_wds_group_submit(&comp_wds_group);
    ck_vk_comp_dispatch(m_vk_comp, 1, 1, 1, VK_NULL_HANDLE, true);

    float dat_out[10] = {0.0F};
    ck_vk_bfr_read(m_bfr_out, 10 * sizeof(float), &dat_out);
    for (uint32_t i = 0; i < 10; ++i) {
        printf("%f\n", dat_out[i]);
    }

    ck_cleanup();
    return 0;
}
