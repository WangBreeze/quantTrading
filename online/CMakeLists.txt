# Online模块配置
add_library(online_lib STATIC
    onlinemarket.cpp
    onlinemarket.h
    onlinemarket_okb.cpp
    onlinemarket_okb.h
    onlinemarket_A.cpp
    onlinemarket_A.h
    AppData.h
)

target_include_directories(online_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(online_lib PRIVATE Qt${QT_VERSION_MAJOR}::Network)

# 安装规则
install(TARGETS online_lib
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION include/online
    FILES_MATCHING PATTERN "*.h"
)