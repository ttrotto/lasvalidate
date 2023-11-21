# Include LASread headers
include_directories(${CMAKE_SOURCE_DIR}/LASread/inc)

# LASread source files and libraries
file(GLOB EXTERNAL_SOURCES ${CMAKE_SOURCE_DIR}/LASread/src/*.cpp ${CMAKE_SOURCE_DIR}/LASread/src/*.hpp)
file(GLOB LIB_FILES ${CMAKE_SOURCE_DIR}/LASread/lib/*.lib)

add_library(LASread ${EXTERNAL_SOURCES})

target_include_directories(LASread
  PUBLIC
    ${CMAKE_SOURCE_DIR}/LASread/inc
)
target_link_libraries(LASread
  PUBLIC
    ${LIB_FILES}
)
target_compile_definitions(LASread
  PUBLIC
    LASZIP_ENABLED
#    _WIN32
    _MBCS
    _LIB
)