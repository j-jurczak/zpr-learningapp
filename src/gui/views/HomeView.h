/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Home view of the application - header file.
 */

#pragma once
#include <QPushButton>
#include <QWidget>

class HomeView : public QWidget {
    Q_OBJECT
public:
    explicit HomeView( QWidget* parent = nullptr );
    // to-do: add getters for buttons
    QPushButton* btn_new_set_;
    QPushButton* btn_import_;
};