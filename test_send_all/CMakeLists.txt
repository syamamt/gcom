# 実行ファイルの追加する
add_executable(test_send_all server.cpp)
# gcomをリンクする
target_link_libraries(test_send_all PRIVATE gcom)
# gcomヘッダーを追加する
target_include_directories(test_send_all PRIVATE ${PROJECT_SOURCE_DIR})
# コンパイルオプションを指定する
target_compile_options(test_send_all PRIVATE -Wall -Wextra -g)
# # マクロを指定
# target_compile_definitions(test_send_all PRIVATE DEBUG=1)
