/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Class of the main application window - source file.
 */
#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QDebug>
#include <QMessageBox>
#include <QPushButton>
#include <QTranslator>
#include <QApplication>

#include "MainWindow.h"
#include "views/SettingsView.h"
#include "../core/utils/LanguageManager.h"
#include "views/ViewType.h"
#include "views/ViewFactory.h"
#include "views/SetsView.h"
#include "views/SetView.h"
#include "views/AddSetView.h"
#include "views/HomeView.h"
#include "views/LearningView.h"

MainWindow::MainWindow( ViewFactory& factory, QWidget* parent )
    : QMainWindow( parent ), factory_( factory ) {
    setWindowTitle( tr( "LearningApp" ) );
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

    btn_home_ = new QPushButton( "• " + tr( "Home" ), this );
    btn_sets_ = new QPushButton( "• " + tr( "My Sets" ), this );
    btn_settings_ = new QPushButton( "• " + tr( "Settings" ), this );

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
    connect( btn_home_, &QPushButton::clicked, this, [this]() {
        if ( confirmSessionExit() ) {
            main_stack_->setCurrentWidget( home_view_ );
        }
    } );

    connect( btn_sets_, &QPushButton::clicked, this, [this]() {
        if ( confirmSessionExit() ) {
            if ( auto* sets_ptr = qobject_cast<SetsView*>( sets_view_ ) ) {
                sets_ptr->refreshSetsList();
            }
            main_stack_->setCurrentWidget( sets_view_ );
        }
    } );

    connect( btn_settings_, &QPushButton::clicked, this, [this]() {
        if ( confirmSessionExit() ) {
            main_stack_->setCurrentWidget( settings_view_ );
        }
    } );

    // settings view language change
    if ( auto* settings_ptr = qobject_cast<SettingsView*>( settings_view_ ) ) {
        connect( settings_ptr, &SettingsView::languageChanged, this,
                 &MainWindow::handleLanguageChange );
    }

    // sets view logic
    if ( auto* sets_view_ptr = qobject_cast<SetsView*>( sets_view_ ) ) {
        connect( sets_view_ptr, &SetsView::setClicked, this, [this]( int set_id ) {
            QWidget* detail_view = factory_.create( ViewType::SET_VIEW, set_id, this );

            main_stack_->addWidget( detail_view );
            main_stack_->setCurrentWidget( detail_view );

            if ( auto* detail_ptr = qobject_cast<SetView*>( detail_view ) ) {
                connect( detail_ptr, &SetView::backToSetsClicked, this, [this, detail_view]() {
                    main_stack_->setCurrentWidget( sets_view_ );
                    main_stack_->removeWidget( detail_view );
                    detail_view->deleteLater();
                    if ( auto* sets_ptr = qobject_cast<SetsView*>( sets_view_ ) ) {
                        sets_ptr->refreshSetsList();
                    }
                } );

                // LearningView start logic
                connect(
                    detail_ptr, &SetView::learnClicked, this,
                    [this, detail_view]( int id, LearningMode mode ) {
                        QWidget* learning_widget = factory_.create( ViewType::LEARNING, {}, this );

                        if ( auto* learning_ptr = qobject_cast<LearningView*>( learning_widget ) ) {
                            main_stack_->addWidget( learning_widget );
                            main_stack_->setCurrentWidget( learning_widget );
                            learning_ptr->startSession( id, mode );

                            connect(
                                learning_ptr, &LearningView::sessionFinished, this,
                                [this, learning_widget]() {
                                    if ( auto* sets_ptr = qobject_cast<SetsView*>( sets_view_ ) ) {
                                        sets_ptr->refreshSetsList();
                                    }
                                    main_stack_->setCurrentWidget( sets_view_ );
                                    main_stack_->removeWidget( learning_widget );
                                    learning_widget->deleteLater();
                                } );
                            main_stack_->removeWidget( detail_view );
                            detail_view->deleteLater();

                        } else {
                            qCritical() << "MainWindow error: could not create LearningView!";
                        }
                    } );
            }
        } );

        // new set button
        connect( sets_view_ptr, &SetsView::newSetClicked, this, [this, sets_view_ptr]() {
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
                    qDebug() << "MainWindow: Set created - refreshing list.";
                    sets_view_ptr->refreshSetsList();
                    main_stack_->setCurrentWidget( sets_view_ );
                    main_stack_->removeWidget( add_view );
                    add_view->deleteLater();
                } );
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
        qCritical() << "MainWindow type error: sets_view_ is not a SetsView!";
    }
}

bool MainWindow::confirmSessionExit() {
    if ( qobject_cast<LearningView*>( main_stack_->currentWidget() ) ) {
        auto reply =
            QMessageBox::question( this, tr( "Session in progress" ),
                                   tr( "Learning session is in progress. Are you sure you want to "
                                       "quit? \nProgress is saved automatically." ),
                                   QMessageBox::Yes | QMessageBox::No );

        return ( reply == QMessageBox::Yes );
    }
    return true;
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

void MainWindow::handleLanguageChange( QString langCode ) {
    LanguageManager::loadLanguage( langCode );
    reloadUi();
}

void MainWindow::reloadUi() {
    main_stack_->removeWidget( home_view_ );
    main_stack_->removeWidget( sets_view_ );
    main_stack_->removeWidget( settings_view_ );

    home_view_->deleteLater();
    sets_view_->deleteLater();
    settings_view_->deleteLater();

    home_view_ = factory_.create( ViewType::HOME, {}, this );
    sets_view_ = factory_.create( ViewType::SETS, {}, this );
    settings_view_ = factory_.create( ViewType::SETTINGS, {}, this );

    main_stack_->addWidget( home_view_ );
    main_stack_->addWidget( sets_view_ );
    main_stack_->addWidget( settings_view_ );

    setWindowTitle( tr( "LearningApp" ) );
    btn_home_->setText( "• " + tr( "Home" ) );
    btn_sets_->setText( "• " + tr( "My Sets" ) );
    btn_settings_->setText( "• " + tr( "Settings" ) );

    setupConnections();

    main_stack_->setCurrentWidget( settings_view_ );
}