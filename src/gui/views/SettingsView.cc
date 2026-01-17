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

    QLabel* title = new QLabel( tr( "Learning Settings" ), this );
    title->setStyleSheet( "font-size: 24px; font-weight: bold; color: white;" );
    main_layout->addWidget( title );

    QGroupBox* lang_group = new QGroupBox( tr( "Language" ), this );
    lang_group->setStyleSheet(
        "QGroupBox { font-weight: bold; border: 1px solid #444; border-radius: 5px; margin-top: "
        "20px; } QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }" );
    QVBoxLayout* lang_layout = new QVBoxLayout( lang_group );
    lang_layout->setContentsMargins( 20, 30, 20, 20 );

    QLabel* lang_info = new QLabel(
        tr( "Application language (requires restart):" ), lang_group );
    lang_info->setStyleSheet( "color: #aaa; margin-bottom: 5px;" );
    lang_layout->addWidget( lang_info );

    combo_language_ = new QComboBox( lang_group );
    combo_language_->addItem( "English", "en" );
    combo_language_->addItem( "Polski", "pl" );

    QSettings settings( "ZPR", "LearningApp" );
    QString current_lang = settings.value( "language", "en" ).toString();
    int index = combo_language_->findData( current_lang );
    if ( index != -1 ) combo_language_->setCurrentIndex( index );

    connect( combo_language_, QOverload<int>::of( &QComboBox::currentIndexChanged ), this,
             [this]( int idx ) {
                 QString lang_code = combo_language_->itemData( idx ).toString();
                 QSettings s( "ZPR", "LearningApp" );
                 s.setValue( "language", lang_code );
                 emit languageChanged( lang_code );
             } );

    lang_layout->addWidget( combo_language_ );
    main_layout->addWidget( lang_group );

    QGroupBox* group = new QGroupBox( tr( "Allowed answer forms" ), this );
    group->setStyleSheet(
        "QGroupBox { font-weight: bold; border: 1px solid #444; border-radius: 5px; margin-top: "
        "20px; } QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }" );

    QVBoxLayout* group_layout = new QVBoxLayout( group );
    group_layout->setSpacing( 15 );
    group_layout->setContentsMargins( 20, 30, 20, 20 );

    QLabel* info = new QLabel(
        tr( "Choose which task types you want to see during the session. If you uncheck a type "
            "assigned to a card, it will be displayed as a standard Flashcard." ),
        group );
    info->setWordWrap( true );
    info->setStyleSheet( "color: #aaa; margin-bottom: 10px;" );
    group_layout->addWidget( info );

    // QSettings settings( "ZPR", "LearningApp" ); // Already declared above

    chk_enable_flashcards_ = new QCheckBox( tr( "Flashcards (Standard)" ), group );
    chk_enable_flashcards_->setChecked( true );
    chk_enable_flashcards_->setEnabled( false );
    chk_enable_flashcards_->setStyleSheet( "color: #ccc;" );
    group_layout->addWidget( chk_enable_flashcards_ );

    bool quiz_enabled = settings.value( "enable_quiz", true ).toBool();
    chk_enable_quiz_ = new QCheckBox( tr( "Quizzes (Multiple Choice)" ), group );
    chk_enable_quiz_->setChecked( quiz_enabled );
    chk_enable_quiz_->setCursor( Qt::PointingHandCursor );
    group_layout->addWidget( chk_enable_quiz_ );

    connect( chk_enable_quiz_, &QCheckBox::stateChanged, this, []( int state ) {
        QSettings s( "ZPR", "LearningApp" );
        s.setValue( "enable_quiz", state == Qt::Checked );
    } );

    bool input_enabled = settings.value( "enable_input", true ).toBool();
    chk_enable_input_ = new QCheckBox( tr( "Typing (Input)" ), group );
    chk_enable_input_->setChecked( input_enabled );
    chk_enable_input_->setCursor( Qt::PointingHandCursor );
    group_layout->addWidget( chk_enable_input_ );

    bool random_input = settings.value( "randomize_simple_cards", false ).toBool();
    chk_random_input_ = new QCheckBox( tr( "Randomly force type-in answers" ), group );
    chk_random_input_->setChecked( random_input );
    chk_random_input_->setCursor( Qt::PointingHandCursor );
    chk_random_input_->setToolTip(
        tr( "If checked, simple cards will sometimes appear as an input field instead of a 'Show' "
            "button." ) );
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