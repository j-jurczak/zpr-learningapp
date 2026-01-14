/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Class of the main application window - source file.
 */
#include "MainWindow.h"
#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QDebug>

#include "views/ViewType.h"
#include "views/ViewFactory.h"
#include "views/SetsView.h"
#include "views/SetView.h"
#include "views/AddSetView.h"
#include "views/HomeView.h"

MainWindow::MainWindow( ViewFactory& factory, QWidget* parent )
    : QMainWindow( parent ), factory_( factory ) {
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

    home_view_ = factory_.create( ViewType::HOME, {}, this );
    sets_view_ = factory_.create( ViewType::SETS, {}, this );
    settings_view_ = factory_.create( ViewType::SETTINGS, {}, this );

    main_stack_->addWidget( home_view_ );
    main_stack_->addWidget( sets_view_ );
    main_stack_->addWidget( settings_view_ );

    main_layout->addWidget( main_stack_ );

    setupStyles();
    setupConnections();
}

void MainWindow::setupConnections() {
    // sidebar buttons
    connect( btn_home_, &QPushButton::clicked, this,
             [this]() { main_stack_->setCurrentWidget( home_view_ ); } );

    connect( btn_sets_, &QPushButton::clicked, this,
             [this]() { main_stack_->setCurrentWidget( sets_view_ ); } );

    connect( btn_settings_, &QPushButton::clicked, this,
             [this]() { main_stack_->setCurrentWidget( settings_view_ ); } );

    // sets view logic
    if ( auto* sets_view_ptr = qobject_cast<SetsView*>( sets_view_ ) ) {
        // qDebug() << "MainWindow: SetsView identified.";

        connect( sets_view_ptr, &SetsView::setClicked, this, [this]( int set_id ) {
            QWidget* detail_view = factory_.create( ViewType::SET_VIEW, set_id, this );

            main_stack_->addWidget( detail_view );
            main_stack_->setCurrentWidget( detail_view );

            if ( auto* detail_ptr = qobject_cast<SetView*>( detail_view ) ) {
                connect( detail_ptr, &SetView::backToSetsClicked, this, [this, detail_view]() {
                    main_stack_->setCurrentWidget( sets_view_ );
                    main_stack_->removeWidget( detail_view );
                    detail_view->deleteLater();
                } );
            }
        } );
        connect( sets_view_ptr, &SetsView::newSetClicked, this, [this, sets_view_ptr]() {
            // qDebug() << "MainWindow: received signal: newSetClicked";

            QWidget* add_view = factory_.create( ViewType::ADD_SET, {}, this );

            main_stack_->addWidget( add_view );
            main_stack_->setCurrentWidget( add_view );

            if ( auto* add_ptr = qobject_cast<AddSetView*>( add_view ) ) {
                connect( add_ptr, &AddSetView::creationCancelled, this, [this, add_view]() {
                    main_stack_->setCurrentWidget( sets_view_ );
                    main_stack_->removeWidget( add_view );
                    add_view->deleteLater();
                } );

                connect( add_ptr, &AddSetView::setCreated, this, [this, add_view, sets_view_ptr]() {
                    qDebug() << "MainWindow: Zestaw utworzony - odświeżam listę.";

                    sets_view_ptr->refreshSetsList();

                    main_stack_->setCurrentWidget( sets_view_ );
                    main_stack_->removeWidget( add_view );
                    add_view->deleteLater();
                } );

            } else {
                qCritical() << "MainWindow BŁĄD: Nie udało się rzutować na AddSetView!";
            }
        } );
    }
    // home view logic
    if ( auto* home_ptr = qobject_cast<HomeView*>( home_view_ ) ) {
        connect( home_ptr, &HomeView::newSetClicked, this, [this]() {
            QWidget* add_view = factory_.create( ViewType::ADD_SET, {}, this );

            main_stack_->addWidget( add_view );
            main_stack_->setCurrentWidget( add_view );

            if ( auto* add_ptr = qobject_cast<AddSetView*>( add_view ) ) {
                connect( add_ptr, &AddSetView::creationCancelled, this, [this, add_view]() {
                    main_stack_->setCurrentWidget( home_view_ );
                    main_stack_->removeWidget( add_view );
                    add_view->deleteLater();
                } );

                connect( add_ptr, &AddSetView::setCreated, this, [this, add_view]() {
                    if ( auto* sets_ptr = qobject_cast<SetsView*>( sets_view_ ) ) {
                        sets_ptr->refreshSetsList();
                    }

                    main_stack_->setCurrentWidget( sets_view_ );
                    main_stack_->removeWidget( add_view );
                    add_view->deleteLater();
                } );
            }
        } );
    } else {
        qCritical() << "MainWindow BŁĄD KRYTYCZNY: sets_view_ nie jest typu SetsView!";
    }
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