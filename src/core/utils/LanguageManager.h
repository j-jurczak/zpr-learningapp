/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Helper class for managing application language and translations.
 */
#pragma once
#include <QString>
#include <QTranslator>
#include <QCoreApplication>

class LanguageManager {
private:
    static QTranslator* m_translator;

public:
    static void loadLanguage( const QString& langCode );
};
