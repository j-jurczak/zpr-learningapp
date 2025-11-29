/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Class for the main window of the application (source file)
 */

#include "MainWindow.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow( QWidget* parent_widget ) : QMainWindow( parent_widget ) {
    setWindowTitle( "System Nauki ZPR" );
    resize( 800, 600 );

    QWidget* central_widget = new QWidget( this );
    setCentralWidget( central_widget );

    welcome_label_ = new QLabel( "Witaj w aplikacji do nauki LearningApp", this );
    welcome_label_->setAlignment( Qt::AlignCenter );

    QVBoxLayout* main_layout = new QVBoxLayout( central_widget );
    main_layout->addWidget( welcome_label_ );
}

MainWindow::~MainWindow() {}