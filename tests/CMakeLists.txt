file(GLOB TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/tests/*.cpp)

foreach(file ${TEST_SRC_FILES})
    cmake_path(GET file STEM stem)
    add_executable(${stem})
    target_sources(${stem} PRIVATE ${file})

    target_link_libraries(${stem} PRIVATE glm::glm)
    target_link_libraries(${stem} PRIVATE SDL2::SDL2)
    target_link_libraries(${stem} PRIVATE "prototype")

    set_target_properties(${stem} PROPERTIES CXX_STANDARD 20)
    set_target_properties(${stem} PROPERTIES CMAKE_CXX_STANDARD_REQUIRED ON)
    set_target_properties(${stem} PROPERTIES COMPILE_OPTIONS -Wall -Wextra -pedantic)
endforeach()

