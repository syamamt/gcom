cmake_minimum_required(VERSION 3.16)
project(gcom LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# ================================
# 静的ライブラリを作成
# ================================
# ディレクトリ直下の .cpp と .hpp ファイルを収集
file(GLOB SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
# 静的ライブラリとしてビルド
add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
# インクルードディレクトリを指定（ヘッダーが直下にある場合）
target_include_directories(gcom PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# gcomライブラリのコンパイル警告
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)


# ================================
# Google Testの追加
# ================================
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# テストコード（tests/*.cpp）を探す
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)
# テスト用の実行ファイルを作成する
add_executable(${PROJECT_NAME}-googletest ${TEST_SOURCES})
# テスト用実行ファイルにリンクする
target_link_libraries(${PROJECT_NAME}-googletest PRIVATE ${PROJECT_NAME} gtest gtest_main)
# テスト登録
include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}-googletest)

# ================================
# サブディレクトリの追加
# ================================
add_subdirectory(test_send_all)
