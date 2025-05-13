cmake_minimum_required(VERSION 3.13)
project(microbench CXX)

include(FetchContent)
option(BUILD_DOC OFF)
set(BUILD_SANDBOX OFF)
FetchContent_Declare(
    cereal
    GIT_REPOSITORY https://github.com/USCiLab/cereal.git
    GIT_TAG        ebef1e929807629befafbb2918ea1a08c7194554 # release-1.3.2
)
FetchContent_MakeAvailable(cereal)

# thread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(tcp_server tcp_server.cpp tcp_connection_manager.cpp configuration.cpp transaction.cpp background.cpp logger.cpp)
target_link_libraries(tcp_server Threads::Threads cereal)
# target_compile_options(tcp_server PUBLIC
#     $<$<CONFIG:Release>: -O2 -Wall> # Release
#     $<$<CONFIG:Debug>: -g -O2 -Wall> # Debug
# )
target_compile_options(tcp_server PUBLIC -O2 -Wall)
target_compile_features(tcp_server PUBLIC cxx_std_20)