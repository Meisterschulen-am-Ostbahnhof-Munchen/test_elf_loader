cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(lib_b)

include(elf_loader)
project_elf(lib_b)
