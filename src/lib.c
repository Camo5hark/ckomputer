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

void ck_cleanup(void) {
    vk_comp_cleanup();
    vk_bfr_cleanup();
    vk_dev_logical_cleanup();
    vk_dev_phys_cleanup();
    vk_inst_cleanup();
}

__attribute__((noreturn))
void fatal_exit(void) {
    fprintf(stderr, "FATAL EXIT; errno = %x\n", errno);
    ck_cleanup();
    exit(EXIT_FAILURE);
}

__attribute__((noreturn))
void fatal_exit_alloc(const char *alloc_func_name, const char *ctx_desc) {
    fprintf(
        stderr,
        "allocation failed; "
        "allocation function name = %s; "
        "context description = \"%s\"\n",
        alloc_func_name,
        ctx_desc
    );
    fatal_exit();
}

__attribute__((noreturn))
void fatal_exit_vk(const VkResult vk_result, const char *vk_func_name) {
    fprintf(
        stderr,
        "Vulkan error; "
        "VkResult = %x; "
        "Vulkan function name = %s\n",
        vk_result,
        vk_func_name
    );
    fatal_exit();
}

void ck_init(const uint32_t flags) {
    vk_inst_init((flags & CK_INIT_FLAG_ENABLE_VK_VAL_LAYER) == CK_INIT_FLAG_ENABLE_VK_VAL_LAYER);
    vk_dev_phys_init();
}
