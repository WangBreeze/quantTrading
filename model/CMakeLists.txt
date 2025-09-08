# Model模块配置
add_library(model_lib STATIC
    BaseModel.cpp
    BaseModel.h
    HistoryDataModel.cpp
    HistoryDataModel.h
    RealtimeDataModel.cpp
    RealtimeDataModel.h
)

target_include_directories(model_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(model_lib PRIVATE 
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    history_lib
)

# 安装规则
install(TARGETS model_lib
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION include/model
    FILES_MATCHING PATTERN "*.h"
)