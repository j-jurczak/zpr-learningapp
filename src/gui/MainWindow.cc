/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Class of the main application window - source file.
 */

#include "MainWindow.h"

#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "ViewFactory.h"

MainWindow::MainWindow( QWidget* parent_widget ) : QMainWindow( parent_widget ) {
    setWindowTitle( "LearningApp" );
    resize( 1250, 750 );

    QWidget* central_widget = new QWidget( this );
    setCentralWidget( central_widget );

    QHBoxLayout* main_layout = new QHBoxLayout( central_widget );
    main_layout->setContentsMargins( 0, 0, 0, 0 );
    main_layout->setSpacing( 0 );

    // sidebar
    QWidget* sidebar_container = new QWidget( this );
    sidebar_container->setFixedWidth( 250 );
    sidebar_container->setObjectName( "sidebar" );

    QVBoxLayout* sidebar_layout = new QVBoxLayout( sidebar_container );
    sidebar_layout->setContentsMargins( 20, 40, 20, 20 );
    sidebar_layout->setSpacing( 20 );

    btn_home_ = new QPushButton( "• Strona główna", this );
    btn_sets_ = new QPushButton( "• Moje zestawy", this );
    btn_settings_ = new QPushButton( "• Ustawienia", this );

    sidebar_layout->addWidget( btn_home_ );
    sidebar_layout->addWidget( btn_sets_ );
    sidebar_layout->addWidget( btn_settings_ );
    sidebar_layout->addStretch();

    main_layout->addWidget( sidebar_container );

    // view 'subwindow'
    main_stack_ = new QStackedWidget( this );

    home_view_ = ViewFactory::create( ViewType::HOME, this );
    sets_view_ = ViewFactory::create( ViewType::SETS, this );
    settings_view_ = ViewFactory::create( ViewType::SETTINGS, this );

    main_stack_->addWidget( home_view_ );      // Index 0 (ViewType::HOME)
    main_stack_->addWidget( sets_view_ );      // Index 1 (ViewType::SETS)
    main_stack_->addWidget( settings_view_ );  // Index 2 (ViewType::SETTINGS)

    main_layout->addWidget( main_stack_ );

    setupStyles();
    setupConnections();
}

void MainWindow::setupConnections() {
    connect( btn_home_, &QPushButton::clicked, this,
             [this]() { main_stack_->setCurrentIndex( static_cast<int>( ViewType::HOME ) ); } );

    connect( btn_sets_, &QPushButton::clicked, this,
             [this]() { main_stack_->setCurrentIndex( static_cast<int>( ViewType::SETS ) ); } );

    connect( btn_settings_, &QPushButton::clicked, this,
             [this]() { main_stack_->setCurrentIndex( static_cast<int>( ViewType::SETTINGS ) ); } );
}

void MainWindow::setupStyles() {
    QFile file( ":/resources/MainWindow.qss" );
    if ( file.open( QFile::ReadOnly ) ) {
        QString style_sheet = QLatin1String( file.readAll() );
        this->setStyleSheet( style_sheet );
        file.close();
    }
}

MainWindow::~MainWindow() {}