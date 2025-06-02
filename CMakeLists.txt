cmake_minimum_required(VERSION 3.30)
project(DesktopGirlClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 启用RC资源编译器
enable_language(RC)

set(CMAKE_PREFIX_PATH "C:/Qt/6.9.0/mingw_64")

set(D_OR_R ON)

set(CONSOLE ON)

set(FRAMEWORK_SOURCE OpenGL)

file(GLOB LApp2D "LAppLive2D/Src/*.cpp")

# 添加库文件搜索路径
link_directories(
        "${CMAKE_SOURCE_DIR}/Lib/Debug/"
)

find_package(Qt6 COMPONENTS
        Core
        Gui
        OpenGLWidgets
        Widgets
        Sql
        Svg
        Multimedia
        REQUIRED)

# 添加ProjectSources文件
set(PROJECT_SOURCES
        ProjectSources/Src/AudioOutput.cpp
        ProjectSources/Inc/AudioOutput.h
)

#message(STATUS ${LApp2D})

if (NOT CONSOLE)
    set(CON WIN32)
endif()

add_executable(DesktopGirlClient ${CON} main.cpp GLCore.cpp SpeechBubbleWidget.cpp resources.qrc app.rc ${LApp2D} ${PROJECT_SOURCES})

# --- 新增/修改：为目标指定链接器搜索路径 ---
#target_link_directories(DesktopGirlClient PRIVATE "${CMAKE_SOURCE_DIR}/Lib/Debug/")

target_link_libraries(DesktopGirlClient
        Framework
        glew32d
        glfw3
        Live2DCubismCore_d
        opengl32
        glu32
        dwmapi
        Qt::Core
        Qt::Gui
        Qt::OpenGLWidgets
        Qt::Widgets
        Qt::Sql
        Qt::Svg
        Qt::Multimedia
)

# 添加头文件搜索路径
include_directories(
        ./
        LAppLive2D/Inc
        ProjectSources/Inc
        Thirdparty/Framework/src
        Thirdparty/Core/include
        Thirdparty/glew/include
        Thirdparty/glew/include/GL
        Thirdparty/glfw/include
        Thirdparty/glfw/include/GLFW
        Thirdparty/stb
)

# 添加库文件搜索路径
link_directories(
        "${CMAKE_SOURCE_DIR}/Lib/Debug/"
)
#message(STATUS CSD: "${CMAKE_SOURCE_DIR}/Lib/Debug/")

# 复制Resources目录到生成目录cmake-build-debug
file(GLOB RESOURCE_W "${CMAKE_BINARY_DIR}/Resources")
#message(STATUS RW: ${RESOURCE_W})
if (NOT RESOURCE_W) # 如果没有这个Resources目录就复制过去
    file(COPY
            "${CMAKE_SOURCE_DIR}/Resources"
            DESTINATION
            "${CMAKE_BINARY_DIR}/"
    )
endif ()

# 复制contact目录到生成目录cmake-build-debug
file(GLOB CONTACT_W "${CMAKE_BINARY_DIR}/contact")
if (NOT CONTACT_W) # 如果没有这个contact目录就复制过去
    file(COPY
            "${CMAKE_SOURCE_DIR}/contact"
            DESTINATION
            "${CMAKE_BINARY_DIR}/"
    )
endif ()

# 复制库文件中的dll文件到生成目录cmake-build-debug
file(GLOB GLEW_DLL_W "${CMAKE_BINARY_DIR}/glew32d.dll")
file(GLOB LIVE2D_DLL_W "${CMAKE_BINARY_DIR}/Live2DCubismCore.dll")
if (NOT GLEW_DLL_W)
    file(COPY
            "${CMAKE_SOURCE_DIR}/Lib/Debug/glew32d.dll"
            DESTINATION
            "${CMAKE_BINARY_DIR}/"
    )
endif ()

if (NOT LIVE2D_DLL_W)
    file(COPY
            "${CMAKE_SOURCE_DIR}/Lib/Debug/Live2DCubismCore.dll"
            DESTINATION
            "${CMAKE_BINARY_DIR}/"
    )
endif ()

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()
    foreach (QT_LIB Core Gui Widgets OpenGLWidgets Multimedia)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()
