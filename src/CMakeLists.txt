file(GLOB SOURCES "*.cpp")

# Create libraries
foreach(SRC ${SOURCES})
    if(NOT ${SRC} MATCHES main.cpp)
        get_filename_component(LIB_NAME ${SRC} NAME_WE)
        add_library(${LIB_NAME} SHARED ${SRC})
    endif()
endforeach(SRC)

add_executable(fp-GA-architecture main.cpp ${SOURCES})
