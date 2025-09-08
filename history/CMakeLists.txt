# History模块配置
add_library(history_lib STATIC
    Strategy.cpp
    Strategy.h
    HistoryDataManager.cpp
    HistoryDataManager.h
    BacktestEngine.cpp
    BacktestEngine.h
)

target_include_directories(history_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(history_lib PRIVATE 
    Qt${QT_VERSION_MAJOR}::Core
    online_lib
)

# 安装规则
install(TARGETS history_lib
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION include/history
    FILES_MATCHING PATTERN "*.h"
)