/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Home view of the application - header file.
 */
#pragma once
#include <QWidget>
#include <QPushButton>

#include "../../db/DatabaseManager.h"

class HomeView : public QWidget {
    Q_OBJECT

public:
    explicit HomeView( DatabaseManager& db, QWidget* parent = nullptr );

signals:
    void newSetClicked();
    void setImported( int set_id );

private:
    DatabaseManager& db_manager_;
    QPushButton* btn_new_set_;
    QPushButton* btn_import_;
};