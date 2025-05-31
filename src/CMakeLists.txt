cmake_minimum_required(VERSION 3.30)
project(problem3 C CXX)
set(PROJECT_FOLDER ${CMAKE_CURRENT_LIST_DIR}/../)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CSS_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(task_solver helper.cpp wrap_map_region.cpp task.cpp task_manager.cpp)
