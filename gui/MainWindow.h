/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Class for the main window of the application (header file)
 */

#pragma once
#include <QMainWindow>

class QLabel;  // forward declaration

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow( QWidget* parent_widget = nullptr );
    ~MainWindow();

private:
    QLabel* welcome_label_;
};