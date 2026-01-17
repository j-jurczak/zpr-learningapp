/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Implementation of LanguageManager.
 */
#include "LanguageManager.h"
#include <QDir>
#include <QDebug>
#include <QCoreApplication>

QTranslator* LanguageManager::m_translator = nullptr;

void LanguageManager::loadLanguage( const QString& langCode ) {
    if ( !m_translator ) {
        m_translator = new QTranslator( qApp );
    }

    qApp->removeTranslator( m_translator );

    if ( langCode == "en" ) {
        return;
    }

    QString translationPath = QString( PROJECT_ROOT ) + "/src/translations";
    QString filename = "learningapp_" + langCode;

    if ( m_translator->load( filename, translationPath ) ) {
        qApp->installTranslator( m_translator );
    } else {
        qDebug() << "LanguageManager: Failed to load translation for:" << langCode << "at" << translationPath;
    }
}
