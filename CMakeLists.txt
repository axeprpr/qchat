cmake_minimum_required(VERSION 3.21)

project(QChat VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# ============================================================================
# Qt6 Dependencies
# ============================================================================
find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Quick
    QuickControls2
    Network
    Svg
    LinguistTools
)

qt_standard_project_setup(REQUIRES 6.5)

# ============================================================================
# FluentUI Integration
# ============================================================================
# FluentUI is included as a subdirectory.
# Clone it first: git clone https://github.com/zhuzichu520/FluentUI.git third_party/FluentUI
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/FluentUI/CMakeLists.txt")
    add_subdirectory(third_party/FluentUI)
    set(FLUENTUI_AVAILABLE ON)
    message(STATUS "FluentUI found and enabled")
else()
    set(FLUENTUI_AVAILABLE OFF)
    message(STATUS "FluentUI not found - using fallback styling")
endif()

# ============================================================================
# Source Files
# ============================================================================
set(CPP_SOURCES
    src/main.cpp
    src/core/ChatManager.h
    src/core/ChatManager.cpp
    src/core/MessageModel.h
    src/core/MessageModel.cpp
    src/core/ConversationListModel.h
    src/core/ConversationListModel.cpp
    src/core/ModelProvider.h
    src/core/OpenAIProvider.h
    src/core/OpenAIProvider.cpp
    src/core/ClaudeProvider.h
    src/core/ClaudeProvider.cpp
    src/core/OllamaProvider.h
    src/core/OllamaProvider.cpp
    src/core/SettingsManager.h
    src/core/SettingsManager.cpp
    src/core/DocumentParser.h
    src/core/DocumentParser.cpp
    src/core/MarkdownHelper.h
    src/core/MarkdownHelper.cpp
    src/core/ThinkingParser.h
    src/core/ThinkingParser.cpp
)

set(QML_FILES
    src/qml/Main.qml
    src/qml/ChatView.qml
    src/qml/Sidebar.qml
    src/qml/MessageBubble.qml
    src/qml/SettingsDialog.qml
    src/qml/WelcomeView.qml
    src/qml/ThinkingBlock.qml
    src/qml/DocumentChip.qml
    src/qml/ProviderConfig.qml
    src/qml/components/IconButton.qml
    src/qml/components/Tag.qml
)

set(RESOURCE_FILES
    resources/resources.qrc
)

# ============================================================================
# Executable
# ============================================================================
qt_add_executable(${PROJECT_NAME} ${CPP_SOURCES} ${RESOURCE_FILES})

qt_add_qml_module(${PROJECT_NAME}
    URI QChat
    VERSION 1.0
    QML_FILES ${QML_FILES}
)

# ============================================================================
# Link Libraries
# ============================================================================
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
    Qt6::Svg
)

if(FLUENTUI_AVAILABLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE fluentuiplugin)
    target_compile_definitions(${PROJECT_NAME} PRIVATE FLUENTUI_AVAILABLE)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
)

# ============================================================================
# Install & Deploy
# ============================================================================
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET ${PROJECT_NAME}
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

# ============================================================================
# Platform-specific
# ============================================================================
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    # App icon
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources/app.rc")
        target_sources(${PROJECT_NAME} PRIVATE resources/app.rc)
    endif()
elseif(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.qchat.app"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )
endif()
