# Indicators模块配置
add_library(indicators_lib STATIC
    IndicatorBase.cpp
    IndicatorBase.h
    MovingAverage.cpp
    MovingAverage.h
    MACD.cpp
    MACD.h
    RSI.cpp
    RSI.h
    BollingerBands.cpp
    BollingerBands.h
)

target_include_directories(indicators_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(indicators_lib PRIVATE 
    Qt${QT_VERSION_MAJOR}::Core
    model_lib
)

# 安装规则
install(TARGETS indicators_lib
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION include/indicators
    FILES_MATCHING PATTERN "*.h"
)