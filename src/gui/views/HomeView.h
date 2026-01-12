/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Home view of the application - header file.
 */
#pragma once
#include <QWidget>
#include <QPushButton>

class HomeView : public QWidget {
    Q_OBJECT
public:
    explicit HomeView( QWidget* parent = nullptr );

signals:
    void newSetClicked();

private:
    QPushButton* btn_new_set_;
    QPushButton* btn_import_;
};