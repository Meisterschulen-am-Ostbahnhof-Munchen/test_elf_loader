idf_component_register(SRCS "lib_b.cpp" INCLUDE_DIRS "../../common")
target_compile_options(${COMPONENT_LIB} PRIVATE -fvisibility=hidden)
