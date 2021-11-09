cmake_minimum_required(VERSION 3.12)

add_library(StbiLib STATIC stb_image.h)

set_target_properties(StbiLib PROPERTIES LINKER_LANGUAGE CXX)