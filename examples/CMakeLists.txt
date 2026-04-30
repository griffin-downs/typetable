add_executable(interface interface.cpp)
target_include_directories(interface PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_executable(plain_struct plain_struct.cpp)
target_include_directories(plain_struct PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_executable(value value.cpp)
target_include_directories(value PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_executable(ordered ordered.cpp)
target_include_directories(ordered PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_executable(composition composition.cpp)
target_include_directories(composition PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_executable(property property.cpp)
target_include_directories(property PRIVATE ${CMAKE_SOURCE_DIR}/include)
