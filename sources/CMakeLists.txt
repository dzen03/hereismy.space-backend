file(GLOB_RECURSE sources CONFIGURE_DEPENDS
        */*.cpp
        */*.h
)

if (WIN32)
    link_libraries(Ws2_32 Mswsock AdvApi32)
    add_library(here_is_my_space-sources STATIC ${sources})
else ()
    add_library(here_is_my_space-sources SHARED ${sources})
endif ()

target_link_libraries(here_is_my_space-sources PRIVATE simple-http-server-sources)
target_include_directories(here_is_my_space-sources PUBLIC sources ${CMAKE_CURRENT_SOURCE_DIR})
