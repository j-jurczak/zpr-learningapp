
add_library(CoreLib STATIC
# core files
    core/Card.cc
    core/Card.h
    core/CardTypes.h
    core/LearningSession.cc
    core/LearningSession.h
    core/StudySet.h
    core/utils/SetImporter.cc
    core/utils/SetImporter.h
    core/utils/importers/IImportStrategy.h
    core/utils/importers/JsonImportStrategy.cc
    core/utils/importers/JsonImportStrategy.h

    # DB
    db/DatabaseManager.cc
    db/DatabaseManager.h
)

target_link_libraries(CoreLib PUBLIC Qt6::Sql) # SQLite 

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON) # automatic file generation for Qt resources

add_library(GuiLib STATIC
    # gui files
    gui/MainWindow.cc
    gui/MainWindow.h
    
    gui/views/ViewFactory.cc
    gui/views/ViewFactory.h
    gui/views/ViewType.h
    gui/views/SetsView.cc
    gui/views/SetsView.h
    gui/views/SettingsView.cc
    gui/views/SettingsView.h
    gui/views/HomeView.cc
    gui/views/HomeView.h
    gui/views/SetView.cc
    gui/views/SetView.h
    gui/views/AddSetView.cc
    gui/views/AddSetView.h

    gui/overlays/CardPreviewOverlay.h
    gui/overlays/CardPreviewOverlay.cc
    gui/overlays/AddCardOverlay.h
    gui/overlays/AddCardOverlay.cc
    gui/overlays/OverlayContainer.h
    gui/overlays/OverlayContainer.cc

    resources.qrc
)

target_link_libraries(GuiLib PRIVATE Qt6::Widgets CoreLib)

add_executable(LearningApp 
    Main.cc
    resources.qrc
)

target_link_libraries(LearningApp PRIVATE 
    GuiLib 
    CoreLib 
    Qt6::Widgets 
    Qt6::Sql
)