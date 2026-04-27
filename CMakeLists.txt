cmake_minimum_required(VERSION 3.16)

project(Networkmanager_GUI_Qt VERSION 0.1 LANGUAGES CXX)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.2 COMPONENTS Quick REQUIRED)
find_package(KF6NetworkManagerQt REQUIRED)

qt_add_executable(appNetworkmanager_GUI_Qt
    main.cpp
)

qt_add_qml_module(appNetworkmanager_GUI_Qt
    URI Networkmanager_GUI_Qt
    VERSION 1.0

    QML_FILES
    qml/Main.qml

    # components
    qml/components/TopBar.qml
    qml/components/SideBar.qml
    qml/components/DeviceCard.qml
    qml/components/WifiItem.qml
    qml/components/ConnectionEditorPanel.qml

    # pages
    qml/pages/DevicePage.qml
    qml/pages/WifiPage.qml
    qml/pages/ConnectionsPage.qml

    SOURCES src/model/ap/ap_model.h
    SOURCES src/model/ap/ap_model.cpp

    SOURCES src/model/connection/con_list_model.h
    SOURCES src/model/connection/con_list_model.cpp
    SOURCES src/model/connection/con_setting_info.h
    SOURCES src/model/connection/con_setting_info.cpp
    #SOURCES src/model/connection/con_setting_model.h
    #SOURCES src/model/connection/con_setting_model.cpp

    SOURCES src/model/device/device_info.h
    SOURCES src/model/device/device_info.cpp
    SOURCES src/model/device/device_model.h
    SOURCES src/model/device/device_model.cpp

    SOURCES src/model/connection/con_runtime_model.h
    SOURCES src/model/connection/con_runtime_model.cpp
    SOURCES src/model/connection/con_editor_model.h
    SOURCES src/model/connection/con_editor_model.cpp

    SOURCES src/service/ap_service.h
    SOURCES src/service/ap_service.cpp
    #SOURCES src/service/con_setting_service.cpp
    #SOURCES src/service/con_setting_service.h
    SOURCES src/service/device_service.h
    SOURCES src/service/device_service.cpp

    SOURCES src/service/connection_manager.h
    SOURCES src/service/connection_manager.cpp
    SOURCES src/service/con_runtime_service.h
    SOURCES src/service/con_runtime_service.cpp

    SOURCES src/wrapper/nm_wrapper.h
    SOURCES src/wrapper/nm_wrapper.cpp

    SOURCES src/utils/network_utils.h
    SOURCES src/utils/network_utils.cpp

    SOURCES src/filter/device_filter_proxy_model.h
    SOURCES src/filter/device_filter_proxy_model.cpp

)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appNetworkmanager_GUI_Qt PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appNetworkmanager_GUI_Qt
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appNetworkmanager_GUI_Qt
    PRIVATE Qt6::Quick
    PRIVATE Qt6::Qml
    PRIVATE KF6::NetworkManagerQt)

include(GNUInstallDirs)
install(TARGETS appNetworkmanager_GUI_Qt
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
