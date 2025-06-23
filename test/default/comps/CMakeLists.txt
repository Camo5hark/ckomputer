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
project(ckomputer)

file(MAKE_DIRECTORY ${COMP_BIN_DIR})
file(GLOB_RECURSE COMP_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.comp")

foreach (COMP_SRC_FILE ${COMP_SRC_FILES})
    get_filename_component(FILE_NAME ${COMP_SRC_FILE} NAME)
    set(COMP_OUT_FILE ${COMP_BIN_DIR}/${FILE_NAME}.spv)
    add_custom_command(
            OUTPUT ${COMP_OUT_FILE}
            COMMAND ${VULKAN_SDK_DIR}/Bin/glslc ${COMP_SRC_FILE} -o ${COMP_OUT_FILE}
            DEPENDS ${COMP_SRC_FILE}
            COMMENT "building computer SPIR-V binary: ${FILE_NAME}"
            VERBATIM
    )
    list(APPEND COMP_OUT_FILES ${COMP_OUT_FILE})
endforeach ()
add_custom_target(TestDefaultComputers ALL DEPENDS ${COMP_OUT_FILES})
