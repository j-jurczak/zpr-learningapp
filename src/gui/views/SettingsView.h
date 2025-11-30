/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Settings view of the application - header file.
 */
#pragma once
#include <QWidget>

class SettingsView : public QWidget {
    Q_OBJECT
public:
    explicit SettingsView( QWidget* parent = nullptr );
};