/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: View for application settings.
 */
#pragma once
#include <QWidget>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QSettings>
#include <QComboBox>

class SettingsView : public QWidget {
    Q_OBJECT
public:
    explicit SettingsView( QWidget* parent = nullptr );

signals:
    void languageChanged( QString langCode );

private:
    void setupUi();

    QCheckBox* chk_enable_flashcards_;
    QCheckBox* chk_enable_quiz_;
    QCheckBox* chk_enable_input_;
    QCheckBox* chk_random_input_;
    QComboBox* combo_language_;
};