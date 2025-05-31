cmake_minimum_required(VERSION 3.30)
project(problem3 C CXX)
set(PROJECT_FOLDER ${CMAKE_CURRENT_LIST_DIR})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CSS_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(flipper "./cmd/flipper.cpp")

set(BOOST_ROOT "/app/3rd_party/boost")
set(BOOST_INCLUDE_DIRS "/app/3rd_party/boost/include")
set(BOOST_LIBRARY_DIRS "/app/3rd_party/boost/include")

find_package(absl REQUIRED)
target_link_libraries(flipper PUBLIC absl::flags absl::flags_parse)

find_package(Boost 1.88.0 COMPONENTS interprocess filesystem)
target_link_libraries(flipper PUBLIC Boost::interprocess Boost::filesystem)

add_subdirectory(src)
target_link_libraries(flipper PUBLIC task_solver)
target_include_directories(flipper PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/src"
                          )
                          