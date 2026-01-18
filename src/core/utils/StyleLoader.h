/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: StyleLoader utility class - header file.
 */
#pragma once

class StyleLoader {
public:
    static QString getResourcePath( const QString& filename );
    static void attach( QWidget* widget, const QString& filename );
    static void setGlobalStyle();
};