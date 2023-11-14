file(GLOB SOURCES "*.cpp" "*.hpp")

add_executable(LASvalidate ${SOURCES}
        lasrepair.cpp
        lasrepair.hpp)

target_link_libraries(LASvalidate
  PUBLIC
    LASread
)