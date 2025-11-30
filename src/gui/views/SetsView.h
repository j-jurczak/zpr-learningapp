/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Sets view of the application - header file.
 */
#pragma once
#include <QListWidget>
#include <QWidget>
#include "../../db/DatabaseManager.h"

class SetsView : public QWidget {
    Q_OBJECT
public:
    explicit SetsView( DatabaseManager& db, QWidget* parent = nullptr );
    void refreshSetsList();

private:
    void setupStyles();
    QListWidget* list_widget_;
    DatabaseManager& db_manager_;
};