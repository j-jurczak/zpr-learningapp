/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Home view of the application - source file.
 */
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QDir>

#include "HomeView.h"
#include "../../core/utils/StyleLoader.h"
#include "../../core/utils/SetImporter.h"

using namespace std;

HomeView::HomeView( DatabaseManager& db, QWidget* parent ) : QWidget( parent ), db_manager_( db ) {
    this->setObjectName( "content" );

    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setAlignment( Qt::AlignCenter );
    layout->setSpacing( 20 );

    QLabel* title_label = new QLabel( tr( "Welcome to LearningApp!" ), this );
    title_label->setObjectName( "title" );
    title_label->setAlignment( Qt::AlignCenter );

    QLabel* subtitle_label = new QLabel( tr( "What do you want to learn today?" ), this );
    subtitle_label->setObjectName( "subtitle" );
    subtitle_label->setAlignment( Qt::AlignCenter );

    QHBoxLayout* buttons_layout = new QHBoxLayout();
    buttons_layout->setSpacing( 30 );
    buttons_layout->setAlignment( Qt::AlignCenter );

    btn_new_set_ = new QPushButton( tr( "New Set" ), this );
    btn_new_set_->setFixedSize( 160, 50 );
    btn_new_set_->setProperty( "type", "primary" );
    btn_new_set_->setCursor( Qt::PointingHandCursor );

    btn_import_ = new QPushButton( tr( "Import" ), this );
    btn_import_->setFixedSize( 160, 50 );
    btn_import_->setCursor( Qt::PointingHandCursor );

    buttons_layout->addWidget( btn_new_set_ );
    buttons_layout->addWidget( btn_import_ );

    layout->addStretch();
    layout->addWidget( title_label );
    layout->addWidget( subtitle_label );
    layout->addLayout( buttons_layout );
    layout->addStretch();

    connect( btn_new_set_, &QPushButton::clicked, this, &HomeView::newSetClicked );

    connect( btn_import_, &QPushButton::clicked, this, [this]() {
        QApplication::beep();
        QFileDialog dialog( nullptr, tr( "Choose set file" ) );
        dialog.setDirectory( QDir::homePath() );
        dialog.setNameFilter( tr( "Archive files (*.zip);;All files (*)" ) );
        dialog.setFileMode( QFileDialog::ExistingFile );
        dialog.setOption( QFileDialog::DontUseNativeDialog, true );

        if ( dialog.exec() ) {
            QStringList files = dialog.selectedFiles();
            if ( !files.isEmpty() ) {
                QString error;
                if ( SetImporter::importFile( files.first(), db_manager_, error ) ) {
                    QMessageBox::information( this, tr( "Success" ),
                                              tr( "Set imported successfully!" ) );

                    vector<StudySet> sets = db_manager_.getAllSets();
                    int maxId = -1;
                    for ( const auto& s : sets ) {
                        if ( s.id > maxId ) maxId = s.id;
                    }

                    if ( maxId > 0 ) {
                        emit setImported( maxId );
                    }
                } else {
                    QMessageBox::critical( nullptr, tr( "Import Error" ), error );
                }
            }
        }
    } );

    StyleLoader::attach( this, "views/HomeView.qss" );
}