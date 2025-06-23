# ckomputer - Vulkan C wrapper library that provides compute functionality
# Copyright (C) 2025  Andrew Hall
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# License location: <project-root>/COPYING
# Contact e-mail: camo5hark10@gmail.com
# Git repo: https://github.com/Camo5hark/ckomputer

cmake_minimum_required(VERSION 3.29)
project(ckomputer C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

enable_testing()

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_compile_definitions(-DDEBUG)
endif ()

add_compile_options(-Wall -Wextra -Wpedantic)

add_library(
        ckomputer
        SHARED
        src/linkedlist_impl.c
        src/vma_impl.cpp
        include/ckomputer/ckomputer.h
        src/lib.h
        src/lib.c
        src/vk.h
        src/vk_inst.c
        src/vk_dev_phys.c
        src/vk_dev_logical.c
        src/vk_cmd.c
        src/vk_bfr.c
        src/vk_comp.c
)
set_target_properties(
        ckomputer
        PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
)
target_include_directories(ckomputer PUBLIC include)
target_include_directories(ckomputer PUBLIC ${VULKAN_SDK_DIR}/Include)
target_link_directories(ckomputer PUBLIC ${VULKAN_SDK_DIR}/Lib)
target_link_libraries(ckomputer PUBLIC vulkan-1)

set(COMP_BIN_DIR ${CMAKE_BINARY_DIR}/comps)

add_subdirectory(test/default/comps)
add_executable(
        test-default

        test/default/main.c
)
target_include_directories(test-default PUBLIC include)
target_link_libraries(test-default PUBLIC ckomputer)
add_dependencies(test-default ckomputer TestDefaultComputers)
add_test(NAME TestDefault COMMAND test-default)
