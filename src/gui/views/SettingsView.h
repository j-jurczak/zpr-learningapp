/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: View for application settings.
 */
#pragma once
#include <QWidget>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QSettings>

class SettingsView : public QWidget {
    Q_OBJECT
public:
    explicit SettingsView( QWidget* parent = nullptr );

private:
    void setupUi();

    QCheckBox* chk_enable_flashcards_;
    QCheckBox* chk_enable_quiz_;
    QCheckBox* chk_enable_input_;
    QCheckBox* chk_random_input_;
};