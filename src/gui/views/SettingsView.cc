/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Implementation of SettingsView.
 */
#include "SettingsView.h"
#include <QLabel>
#include <QGroupBox>

SettingsView::SettingsView( QWidget* parent ) : QWidget( parent ) { setupUi(); }

void SettingsView::setupUi() {
    QVBoxLayout* main_layout = new QVBoxLayout( this );
    main_layout->setContentsMargins( 30, 40, 30, 40 );
    main_layout->setSpacing( 20 );

    QLabel* title = new QLabel( "Ustawienia Nauki", this );
    title->setStyleSheet( "font-size: 24px; font-weight: bold; color: white;" );
    main_layout->addWidget( title );

    QGroupBox* group = new QGroupBox( "Dozwolone formy odpowiedzi", this );
    group->setStyleSheet(
        "QGroupBox { font-weight: bold; border: 1px solid #444; border-radius: 5px; margin-top: "
        "20px; } QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }" );

    QVBoxLayout* group_layout = new QVBoxLayout( group );
    group_layout->setSpacing( 15 );
    group_layout->setContentsMargins( 20, 30, 20, 20 );

    QLabel* info = new QLabel(
        "Wybierz, jakie typy zadań chcesz widzieć podczas sesji. Jeśli odznaczysz typ przypisany "
        "do karty, zostanie ona wyświetlona jako standardowa Fiszka.",
        group );
    info->setWordWrap( true );
    info->setStyleSheet( "color: #aaa; margin-bottom: 10px;" );
    group_layout->addWidget( info );

    QSettings settings( "ZPR", "LearningApp" );

    chk_enable_flashcards_ = new QCheckBox( "Fiszki (Standard)", group );
    chk_enable_flashcards_->setChecked( true );
    chk_enable_flashcards_->setEnabled( false );
    chk_enable_flashcards_->setStyleSheet( "color: #ccc;" );
    group_layout->addWidget( chk_enable_flashcards_ );

    bool quiz_enabled = settings.value( "enable_quiz", true ).toBool();
    chk_enable_quiz_ = new QCheckBox( "Quizy (Wybór odpowiedzi)", group );
    chk_enable_quiz_->setChecked( quiz_enabled );
    chk_enable_quiz_->setCursor( Qt::PointingHandCursor );
    group_layout->addWidget( chk_enable_quiz_ );

    connect( chk_enable_quiz_, &QCheckBox::stateChanged, this, []( int state ) {
        QSettings s( "ZPR", "LearningApp" );
        s.setValue( "enable_quiz", state == Qt::Checked );
    } );

    bool input_enabled = settings.value( "enable_input", true ).toBool();
    chk_enable_input_ = new QCheckBox( "Wpisywanie tekstu (Input)", group );
    chk_enable_input_->setChecked( input_enabled );
    chk_enable_input_->setCursor( Qt::PointingHandCursor );
    group_layout->addWidget( chk_enable_input_ );

    bool random_input = settings.value( "randomize_simple_cards", false ).toBool();
    chk_random_input_ = new QCheckBox( "Losowo wymuszaj wpisywanie odpowiedzi tekstowych", group );
    chk_random_input_->setChecked( random_input );
    chk_random_input_->setCursor( Qt::PointingHandCursor );
    chk_random_input_->setToolTip(
        "Jeśli zaznaczone, zwykłe karty będą czasami pojawiać się jako pole do wpisywania zamiast "
        "przycisku 'Pokaż'." );
    chk_random_input_->setStyleSheet( "margin-left: 20px; color: #aaa;" );
    group_layout->addWidget( chk_random_input_ );

    connect( chk_enable_input_, &QCheckBox::stateChanged, this, [this]( int state ) {
        bool is_input_active = ( state == Qt::Checked );

        QSettings s( "ZPR", "LearningApp" );
        s.setValue( "enable_input", is_input_active );

        chk_random_input_->setEnabled( is_input_active );

        if ( !is_input_active ) {
            chk_random_input_->setChecked( false );
            s.setValue( "randomize_simple_cards", false );
        }
    } );

    connect( chk_random_input_, &QCheckBox::stateChanged, this, []( int state ) {
        QSettings s( "ZPR", "LearningApp" );
        s.setValue( "randomize_simple_cards", state == Qt::Checked );
    } );

    chk_random_input_->setEnabled( input_enabled );
    if ( !input_enabled ) {
        chk_random_input_->setChecked( false );
    }

    main_layout->addWidget( group );
    main_layout->addStretch();
}