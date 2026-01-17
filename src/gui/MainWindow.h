/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Class of the main application window - header file.
 */
#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QStackedWidget>
#include "views/ViewFactory.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow( ViewFactory& factory, QWidget* parent = nullptr );
    ~MainWindow();

private:
    ViewFactory& factory_;
    void setupStyles();
    void setupConnections();
    bool confirmSessionExit();

    QStackedWidget* main_stack_;

    QPushButton* btn_home_;
    QPushButton* btn_sets_;
    QPushButton* btn_settings_;

    QWidget* home_view_;
    QWidget* sets_view_;
    QWidget* settings_view_;
};