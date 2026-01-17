
add_library(CoreLib STATIC
    # core files
    core/learning/Card.cc
    core/learning/Card.h
    core/learning/CardTypes.h
    core/learning/LearningSession.cc
    core/learning/LearningSession.h
    core/learning/StudySet.h
    core/learning/strategies/ICardSelectionStrategy.h
    core/learning/strategies/SelectionStrategies.h
    core/learning/SuperMemo.cc
    core/learning/SuperMemo.h
    core/utils/LanguageManager.cc
    core/utils/LanguageManager.h
    core/utils/SetImporter.cc
    core/utils/SetImporter.h
    core/utils/SetExporter.cc
    core/utils/SetExporter.h
    core/utils/importers/IImportStrategy.h
    core/utils/importers/JsonImportStrategy.cc
    core/utils/importers/JsonImportStrategy.h
    core/utils/importers/ZipImportStrategy.cc
    core/utils/importers/ZipImportStrategy.h
    core/utils/exporters/IExportStrategy.h
    core/utils/exporters/ZipExportStrategy.cc
    core/utils/exporters/ZipExportStrategy.h

    # DB
    db/DatabaseManager.cc
    db/DatabaseManager.h


)

target_link_libraries(CoreLib PUBLIC Qt6::Sql) # SQLite

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON) # automatic file generation for Qt resources

# gui files
add_library(GuiLib STATIC
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
    gui/views/LearningView.cc
    gui/views/LearningView.h

    gui/overlays/CardPreviewOverlay.h
    gui/overlays/CardPreviewOverlay.cc
    gui/overlays/AddCardOverlay.h
    gui/overlays/AddCardOverlay.cc
    gui/overlays/OverlayContainer.h
    gui/overlays/OverlayContainer.cc

    resources.qrc
)

target_link_libraries(GuiLib PRIVATE Qt6::Widgets CoreLib Qt6::Multimedia)

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