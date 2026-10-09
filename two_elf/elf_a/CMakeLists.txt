cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(lib_a)

include(elf_loader)
project_elf(lib_a)
