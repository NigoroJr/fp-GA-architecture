file(GLOB SOURCES "*.cpp")

find_package(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# Get library names
foreach(SRC ${SOURCES})
    if(NOT ${SRC} MATCHES main.cpp)
        get_filename_component(LIB_NAME ${SRC} NAME_WE)
        list(APPEND LIBS ${SRC})
    endif()
endforeach(SRC)

# Create libraries
foreach(SRC ${SOURCES})
    if(NOT ${SRC} MATCHES main.cpp)
        get_filename_component(LIB_NAME ${SRC} NAME_WE)
        add_library(${LIB_NAME} SHARED ${LIBS})
    endif()
endforeach(SRC)

add_executable(fp-GA-architecture main.cpp ${SOURCES})
