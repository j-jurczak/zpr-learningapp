/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Sets view of the application - source file.
 */
#include <QFile>
#include <QLabel>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDebug>
#include <QShowEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>
#include <QDir>
#include <QApplication>

#include "SetsView.h"
#include "../../core/utils/SetImporter.h"
#include "../../core/utils/SetExporter.h"
#include "../../core/utils/StyleLoader.h"

using namespace std;

SetsView::SetsView( DatabaseManager& db, QWidget* parent ) : QWidget( parent ), db_manager_( db ) {
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setContentsMargins( 40, 40, 40, 40 );
    layout->setSpacing( 20 );

    QLabel* title = new QLabel( tr( "Your Sets" ), this );
    title->setObjectName( "title" );
    title->setAlignment( Qt::AlignLeft );
    layout->addWidget( title );

    QHBoxLayout* btn_layout = new QHBoxLayout();
    btn_layout->setAlignment( Qt::AlignLeft );
    btn_layout->setSpacing( 10 );

    QPushButton* btn_new = new QPushButton( "+ " + tr( "New Set" ), this );
    btn_new->setProperty( "type", "primary" );
    btn_new->setCursor( Qt::PointingHandCursor );
    connect( btn_new, &QPushButton::clicked, this, [this]() { emit newSetClicked(); } );

    QPushButton* btn_import = new QPushButton( tr( "Import" ), this );
    btn_import->setCursor( Qt::PointingHandCursor );
    connect( btn_import, &QPushButton::clicked, this, [this]() {
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
                    refreshSetsList();
                    vector<StudySet> sets = db_manager_.getAllSets();
                    if ( !sets.empty() ) {
                        int maxId = -1;
                        for ( const auto& s : sets ) {
                            if ( s.id > maxId ) maxId = s.id;
                        }
                        if ( maxId > 0 ) {
                            emit setImported( maxId );
                        }
                    }
                } else {
                    QMessageBox::critical( nullptr, tr( "Import Error" ), error );
                }
            }
        }
    } );

    QPushButton* btn_export = new QPushButton( tr( "Export" ), this );
    btn_export->setCursor( Qt::PointingHandCursor );

    connect( btn_export, &QPushButton::clicked, this, [this]() {
        auto selected_items = list_widget_->selectedItems();
        if ( selected_items.isEmpty() ) {
            QMessageBox::warning( this, tr( "Export" ),
                                  tr( "Please select a set from the list to export." ) );
            return;
        }

        QListWidgetItem* item = selected_items.first();
        int id = item->data( Qt::UserRole ).toInt();
        QString setName = item->text();

        QString defaultName = setName + ".zip";
        QString fileName = QFileDialog::getSaveFileName( this, tr( "Export Set" ),
                                                         QDir::homePath() + "/" + defaultName,
                                                         tr( "Archive files (*.zip)" ) );

        if ( !fileName.isEmpty() ) {
            if ( SetExporter::exportSet( id, db_manager_, fileName ) ) {
                QMessageBox::information( this, tr( "Success" ),
                                          tr( "Set exported successfully!" ) );
            } else {
                QMessageBox::critical( this, tr( "Error" ), tr( "Failed to export set." ) );
            }
        }
    } );

    btn_layout->addWidget( btn_new );
    btn_layout->addWidget( btn_import );
    btn_layout->addWidget( btn_export );

    layout->addLayout( btn_layout );

    list_widget_ = new QListWidget( this );
    list_widget_->setContextMenuPolicy( Qt::CustomContextMenu );
    layout->addWidget( list_widget_ );

    connect( list_widget_, &QListWidget::itemClicked, this, [this]( QListWidgetItem* item ) {
        QVariant data = item->data( Qt::UserRole );
        if ( data.isValid() && data.toInt() > 0 ) {
        }
    } );

    connect( list_widget_, &QListWidget::itemDoubleClicked, this, [this]( QListWidgetItem* item ) {
        QVariant data = item->data( Qt::UserRole );
        if ( data.isValid() && data.toInt() > 0 ) {
            emit setClicked( data.toInt() );
        }
    } );

    connect(
        list_widget_, &QListWidget::customContextMenuRequested, this, [this]( const QPoint& pos ) {
            QListWidgetItem* item = list_widget_->itemAt( pos );
            if ( !item ) return;

            int id = item->data( Qt::UserRole ).toInt();
            if ( id <= 0 ) return;

            QMenu contextMenu( tr( "Set Options" ), this );

            QAction* openAction = contextMenu.addAction( tr( "Open" ) );
            QAction* exportAction = contextMenu.addAction( tr( "Export (ZIP)" ) );

            connect( openAction, &QAction::triggered, this,
                     [this, id]() { emit setClicked( id ); } );

            connect( exportAction, &QAction::triggered, this, [this, id, item]() {
                QString defaultName = item->text() + ".zip";
                QString fileName = QFileDialog::getSaveFileName(
                    this, tr( "Export Set" ), QDir::homePath() + "/" + defaultName,
                    tr( "Archive files (*.zip)" ) );
                if ( !fileName.isEmpty() ) {
                    if ( SetExporter::exportSet( id, db_manager_, fileName ) ) {
                        QMessageBox::information( this, tr( "Success" ),
                                                  tr( "Set exported successfully!" ) );
                    } else {
                        QMessageBox::critical( this, tr( "Error" ), tr( "Failed to export set." ) );
                    }
                }
            } );

            contextMenu.exec( list_widget_->mapToGlobal( pos ) );
        } );

    StyleLoader::attach( this, "views/SetsView.qss" );
}

void SetsView::refreshSetsList() {
    list_widget_->clear();
    vector<StudySet> sets = db_manager_.getAllSets();

    if ( sets.empty() ) {
        QListWidgetItem* item = new QListWidgetItem( tr( "No sets. Click '+' to add." ) );
        item->setFlags( Qt::NoItemFlags );
        item->setTextAlignment( Qt::AlignCenter );
        list_widget_->addItem( item );
        return;
    }

    for ( const auto& set : sets ) {
        QListWidgetItem* item = new QListWidgetItem( QString::fromStdString( set.name ) );
        item->setData( Qt::UserRole, set.id );
        list_widget_->addItem( item );
    }
}

void SetsView::showEvent( QShowEvent* event ) {
    QWidget::showEvent( event );
    refreshSetsList();
}