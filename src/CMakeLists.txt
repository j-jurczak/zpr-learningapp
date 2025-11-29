
add_library(CoreLib STATIC
    core/LearningSession.cc
    core/LearningSession.h
)

target_link_libraries(CoreLib PRIVATE Qt6::Sql) # SQLite 

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON) # automatic file generation for Qt resources

add_library(GuiLib STATIC
    ../gui/MainWindow.cc
    ../gui/MainWindow.h
)

target_link_libraries(GuiLib PRIVATE Qt6::Widgets CoreLib)

add_executable(LearningApp Main.cc)

target_link_libraries(LearningApp PRIVATE 
    GuiLib 
    CoreLib 
    Qt6::Widgets 
    Qt6::Sql
)