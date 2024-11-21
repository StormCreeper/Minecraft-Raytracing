file(GLOB SOURCES *.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})

include_directories(${CoolTest_SOURCE_DIR}/dep/glad/include/)

target_sources(${PROJECT_NAME} PRIVATE ../dep/glad/src/gl.c)
target_include_directories(${PROJECT_NAME} PRIVATE ../dep/glad/include/)

target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} glm)
target_link_libraries(${PROJECT_NAME} IMGUI)
target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS})