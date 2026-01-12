/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: View for a specific study set - header file.
 */
#pragma once
#include <QWidget>
#include <QListWidget>
#include "../../db/DatabaseManager.h"

class SetView : public QWidget {
    Q_OBJECT
public:
    explicit SetView( int set_id, DatabaseManager& db, QWidget* parent = nullptr );

signals:
    void backToSetsClicked();

private:
    void setupUi();
    void loadData();

    int set_id_;
    DatabaseManager& db_;

    QListWidget* cards_list_;
};