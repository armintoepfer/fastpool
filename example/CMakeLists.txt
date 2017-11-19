set(FP_FLAGS "-Wall -Wextra -Wno-unused-parameter -Wno-unused-variable")
set(CMAKE_CXX_STANDARD          14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(pool ${FP_ExampleDir}/pool.cpp)
target_link_libraries(pool fastpool)
set_target_properties(pool PROPERTIES COMPILE_FLAGS ${FP_FLAGS})
