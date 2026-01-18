/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: StyleLoader utility class - source file.
 */
#include <QFile>
#include <QString>
#include <QApplication>
#include <QWidget>
#include <QDebug>
#include <QDir>

#include "StyleLoader.h"

QString StyleLoader::getResourcePath( const QString& filename ) {
    QString path = QDir::current().filePath( "src/resources/" + filename );
    if ( !QFile::exists( path ) ) {
        path = QDir::current().filePath( "../src/resources/" + filename );
    }
    return path;
}

void StyleLoader::attach( QWidget* widget, const QString& filename ) {
    QString path = getResourcePath( filename );
    QFile file( path );
    if ( file.open( QFile::ReadOnly | QFile::Text ) ) {
        QString styleSheet = QLatin1String( file.readAll() );
        widget->setStyleSheet( styleSheet );
    } else {
        qWarning() << "Could not load style:" << path;
    }
}

void StyleLoader::setGlobalStyle() {
    QString path = getResourcePath( "Global.qss" );
    QFile file( path );
    if ( file.open( QFile::ReadOnly | QFile::Text ) ) {
        qApp->setStyleSheet( QLatin1String( file.readAll() ) );
    } else {
        qWarning() << "Could not load global style:" << path;
    }
}