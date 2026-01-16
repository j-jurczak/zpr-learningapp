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
#include <QDir>
#include <QApplication>

#include "SetsView.h"
#include "../../core/utils/SetImporter.h"

using namespace std;

SetsView::SetsView( DatabaseManager& db, QWidget* parent ) : QWidget( parent ), db_manager_( db ) {
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setContentsMargins( 40, 40, 40, 40 );
    layout->setSpacing( 20 );

    QLabel* title = new QLabel( "Twoje Zestawy", this );
    title->setObjectName( "title" );
    title->setAlignment( Qt::AlignLeft );
    layout->addWidget( title );

    QHBoxLayout* btn_layout = new QHBoxLayout();
    btn_layout->setAlignment( Qt::AlignLeft );

    QPushButton* btn_new = new QPushButton( "+ Nowy Zestaw", this );
    btn_new->setObjectName( "btn_new_set" );
    btn_new->setCursor( Qt::PointingHandCursor );

    connect( btn_new, &QPushButton::clicked, this, [this]() { emit newSetClicked(); } );

    QPushButton* btn_import = new QPushButton( "Importuj", this );
    btn_import->setObjectName( "btn_import" );
    btn_import->setCursor( Qt::PointingHandCursor );

    connect( btn_import, &QPushButton::clicked, this, [this]() {
        QApplication::beep();

        QFileDialog dialog( nullptr, "Wybierz plik zestawu" );

        dialog.setDirectory( QDir::homePath() );
        dialog.setNameFilter( "Pliki danych (*.json *.csv);;Wszystkie pliki (*)" );
        dialog.setFileMode( QFileDialog::ExistingFile );

        dialog.setOption( QFileDialog::DontUseNativeDialog, true );
        dialog.setWindowModality( Qt::ApplicationModal );
        Qt::WindowFlags flags = dialog.windowFlags();
        dialog.setWindowFlags( flags | Qt::WindowStaysOnTopHint );

        if ( dialog.exec() ) {
            QStringList files = dialog.selectedFiles();
            if ( !files.isEmpty() ) {
                QString fileName = files.first();

                QString error;
                if ( SetImporter::importFile( fileName, db_manager_, error ) ) {
                    QMessageBox::information( this, "Sukces", "Zestaw zaimportowany pomyślnie!" );
                    refreshSetsList();
                } else {
                    QApplication::beep();
                    QMessageBox::critical( nullptr, "Błąd Importu", error );
                }
            }
        }
    } );

    btn_layout->addWidget( btn_new );
    btn_layout->addWidget( btn_import );

    layout->addLayout( btn_layout );

    list_widget_ = new QListWidget( this );
    list_widget_->setObjectName( "list_widget" );
    layout->addWidget( list_widget_ );

    connect( list_widget_, &QListWidget::itemClicked, this, [this]( QListWidgetItem* item ) {
        QVariant data = item->data( Qt::UserRole );
        if ( !data.isValid() ) return;

        int id = data.toInt();
        if ( id <= 0 ) return;

        emit setClicked( id );
    } );

    setupStyles();
}

void SetsView::refreshSetsList() {
    list_widget_->clear();
    vector<StudySet> sets = db_manager_.getAllSets();

    if ( sets.empty() ) {
        QListWidgetItem* item = new QListWidgetItem( "Brak zestawów. Kliknij '+', aby dodać." );
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

void SetsView::setupStyles() {
    QFile file( ":/resources/SetsView.qss" );
    if ( file.open( QFile::ReadOnly ) ) {
        this->setStyleSheet( QString::fromLatin1( file.readAll() ) );
        file.close();
    }
}

void SetsView::showEvent( QShowEvent* event ) {
    QWidget::showEvent( event );
    refreshSetsList();
}