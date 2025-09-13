cmake_minimum_required(VERSION 3.19)
project(EncodeHelper LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(EncodeHelper
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    qencoder.h
    qencoder.cpp

)

target_include_directories(EncodeHelper PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(EncodeHelper
    PRIVATE
        Qt::Core
        Qt::Widgets
)

# 添加自定义命令，将 encoder.exe 拷贝到 EncodeHelper.exe 的输出路径
add_custom_command(
    TARGET EncodeHelper POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:EncodeHelper>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/dist/encoder.exe
        $<TARGET_FILE_DIR:EncodeHelper>/encoder.exe
    COMMENT "Copying encoder.exe to the output directory"
)

include(GNUInstallDirs)

install(TARGETS EncodeHelper
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)


# 安装 encoder.exe
install(FILES ${CMAKE_SOURCE_DIR}/dist/encoder.exe
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# set(QT_DEPLOY_LIBS
#     Qt6Core
#     Qt6Gui
#     Qt6Widgets
# )

qt_generate_deploy_app_script(
    TARGET EncodeHelper
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
