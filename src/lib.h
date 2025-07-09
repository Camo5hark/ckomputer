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

#ifndef LIB_H
#define LIB_H

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <vulkan/vulkan.h>
#include <c-linked-list/linkedlist.h>

#define _VK_FAILURE (vk_result != VK_SUCCESS)

void fatal_exit(void) __attribute__((noreturn));
void fatal_exit_alloc(const char *alloc_func_name, const char *ctx_desc) __attribute__((noreturn));
void fatal_exit_vk(VkResult vk_result, const char *vk_func_name) __attribute__((noreturn));

#endif
