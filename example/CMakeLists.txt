set(TP_FLAGS "-Wall -Wextra -Wno-unused-parameter -Wno-unused-variable")
set(CMAKE_CXX_STANDARD          14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(pool ${TP_ExampleDir}/pool.cpp)
target_link_libraries(pool threadpool)
set_target_properties(pool PROPERTIES COMPILE_FLAGS ${TP_FLAGS})
