# Trading模块配置
add_library(trading_lib STATIC
    TradingEngine.cpp
    TradingEngine.h
    EastMoneyTrading.cpp
    EastMoneyTrading.h
    OKXTrading.cpp
    OKXTrading.h
    BinanceTrader.cpp
    BinanceTrader.h
    StrategyLoader.cpp
    StrategyLoader.h
)

target_include_directories(trading_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(trading_lib PRIVATE 
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    model_lib
    history_lib
)

# 安装规则
install(TARGETS trading_lib
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION include/trading
    FILES_MATCHING PATTERN "*.h"
)
