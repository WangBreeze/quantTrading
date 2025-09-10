cmake_minimum_required(VERSION 3.16)

# 项目基本信息
project(kquant 
    VERSION 0.1 
    DESCRIPTION "Quantitative Trading Framework"
    LANGUAGES CXX
)

# 全局设置
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 编译选项
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#     add_compile_options(-g -O0 -Wall -Wextra)
# else()
#     add_compile_options(-O2 -DNDEBUG)
# endif()
set(PROJECT_SOURCES
    main.cpp
    qml.qrc
    AppData.h

)
# 查找Qt依赖
find_package(QT NAMES  Qt5 REQUIRED COMPONENTS
    Core 
    Quick 
    Widgets 
    Network
    Concurrent
    WebSockets
)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS 
    Core 
    Quick 
    Widgets 
    Network
    Concurrent
    WebSockets
)

# 添加子目录
add_subdirectory(online)
add_subdirectory(history)
add_subdirectory(model)
add_subdirectory(trading)
add_subdirectory(indicators)


# 主可执行文件配置
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(kquant MANUAL_FINALIZATION ${PROJECT_SOURCES})
else()
    add_executable(kquant ${PROJECT_SOURCES})
endif()
target_include_directories(kquant PRIVATE ${Qt${QT_VERSION_MAJOR}WebSockets_INCLUDE_DIRS})
# 链接库
target_link_libraries(kquant PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::WebSockets
    online_lib
    history_lib
    model_lib
    trading_lib
)
# if(MSVC)
#     add_compile_options(/W4 /WX)
# else()
#     add_compile_options(-Wall -Wextra -Werror)
# endif()
# 安装规则
install(TARGETS kquant
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_import_qml_plugins(kquant)
    qt_finalize_executable(kquant)
endif()
