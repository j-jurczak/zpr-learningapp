/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Implementation of the single card addition logic within an overlay.
 */
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QDateTime>
#include <QCoreApplication>

#include "AddCardOverlay.h"

using namespace std;

QString getRootPath() {
#ifdef PROJECT_ROOT
    return QString( PROJECT_ROOT );
#else
    return QDir::currentPath();
#endif
}

AddCardOverlay::AddCardOverlay( QWidget* parent ) : QWidget( parent ) {
    setupUi();
    setupConnections();
}

void AddCardOverlay::setupUi() {
    setStyleSheet( "background-color: rgba(0, 0, 0, 0.85);" );

    QWidget* dialog = new QWidget( this );
    dialog->setFixedSize( 600, 700 );
    dialog->setStyleSheet(
        "background-color: #1e1e1e; border-radius: 10px; border: 1px solid #3e3e42;" );

    QVBoxLayout* main_layout = new QVBoxLayout( this );
    main_layout->addWidget( dialog, 0, Qt::AlignCenter );

    QVBoxLayout* dialog_layout = new QVBoxLayout( dialog );
    dialog_layout->setContentsMargins( 30, 30, 30, 30 );
    dialog_layout->setSpacing( 15 );

    QLabel* title = new QLabel( tr( "New Card" ), dialog );
    title->setStyleSheet(
        "font-size: 24px; font-weight: bold; color: white; border: none; margin-bottom: 10px;" );
    dialog_layout->addWidget( title );

    QHBoxLayout* type_layout = new QHBoxLayout();

    QVBoxLayout* q_type_box = new QVBoxLayout();
    q_type_box->addWidget( new QLabel( tr( "Question Type:" ), dialog ) );
    combo_question_type_ = new QComboBox( dialog );
    combo_question_type_->addItem( tr( "Text" ), 0 );
    combo_question_type_->addItem( tr( "Image" ), 1 );
    combo_question_type_->addItem( tr( "Sound" ), 2 );
    q_type_box->addWidget( combo_question_type_ );
    type_layout->addLayout( q_type_box );

    QVBoxLayout* a_type_box = new QVBoxLayout();
    a_type_box->addWidget( new QLabel( tr( "Answer Type:" ), dialog ) );
    combo_answer_type_ = new QComboBox( dialog );
    combo_answer_type_->addItem( tr( "Flashcard" ), (int)AnswerType::FLASHCARD );
    combo_answer_type_->addItem( tr( "Quiz (Choice)" ), (int)AnswerType::TEXT_CHOICE );
    combo_answer_type_->addItem( tr( "Typing (Input)" ), (int)AnswerType::INPUT );
    a_type_box->addWidget( combo_answer_type_ );
    type_layout->addLayout( a_type_box );

    dialog_layout->addLayout( type_layout );

    dialog_layout->addWidget( new QLabel( tr( "Question Content:" ), dialog ) );

    stack_question_content_ = new QStackedWidget( dialog );
    stack_question_content_->setStyleSheet( "border: none;" );
    stack_question_content_->setFixedHeight( 150 );

    input_question_text_ = new QTextEdit( stack_question_content_ );
    input_question_text_->setPlaceholderText( tr( "Enter question..." ) );
    input_question_text_->setStyleSheet(
        "background-color: #252526; color: white; border: 1px solid #3e3e42; padding: 10px;" );
    stack_question_content_->addWidget( input_question_text_ );

    page_image_ = new QWidget( stack_question_content_ );
    QVBoxLayout* img_layout = new QVBoxLayout( page_image_ );
    btn_select_image_ = new QPushButton( tr( "Choose image..." ), page_image_ );
    label_image_preview_ = new QLabel( tr( "No image selected" ), page_image_ );
    label_image_preview_->setAlignment( Qt::AlignCenter );
    label_image_preview_->setStyleSheet( "color: #888; border: 1px dashed #555;" );
    img_layout->addWidget( btn_select_image_ );
    img_layout->addWidget( label_image_preview_ );
    stack_question_content_->addWidget( page_image_ );

    page_sound_ = new QWidget( stack_question_content_ );
    QVBoxLayout* snd_layout = new QVBoxLayout( page_sound_ );
    btn_select_sound_ = new QPushButton( tr( "Choose audio..." ), page_sound_ );
    label_sound_info_ = new QLabel( tr( "No sound selected" ), page_sound_ );
    label_sound_info_->setAlignment( Qt::AlignCenter );
    label_sound_info_->setStyleSheet( "color: #888; border: 1px dashed #555;" );
    snd_layout->addWidget( btn_select_sound_ );
    snd_layout->addWidget( label_sound_info_ );
    stack_question_content_->addWidget( page_sound_ );

    dialog_layout->addWidget( stack_question_content_ );

    dialog_layout->addWidget( new QLabel( tr( "Correct Answer:" ), dialog ) );
    input_correct_answer_ = new QLineEdit( dialog );
    input_correct_answer_->setPlaceholderText( tr( "E.g. Dog, 1944..." ) );
    input_correct_answer_->setStyleSheet(
        "padding: 8px; background-color: #252526; color: white; border: 1px solid #3e3e42;" );
    dialog_layout->addWidget( input_correct_answer_ );

    quiz_container_ = new QWidget( dialog );
    QVBoxLayout* quiz_layout = new QVBoxLayout( quiz_container_ );
    quiz_layout->setContentsMargins( 0, 0, 0, 0 );

    quiz_layout->addWidget( new QLabel( tr( "Wrong answers (Distractors):" ), quiz_container_ ) );
    input_wrong1_ = new QLineEdit( quiz_container_ );
    input_wrong1_->setPlaceholderText( tr( "Wrong option 1" ) );
    input_wrong2_ = new QLineEdit( quiz_container_ );
    input_wrong2_->setPlaceholderText( tr( "Wrong option 2" ) );
    input_wrong3_ = new QLineEdit( quiz_container_ );
    input_wrong3_->setPlaceholderText( tr( "Wrong option 3" ) );

    QString q_style =
        "padding: 6px; background-color: #252526; color: #ccc; border: 1px solid #3e3e42;";
    input_wrong1_->setStyleSheet( q_style );
    input_wrong2_->setStyleSheet( q_style );
    input_wrong3_->setStyleSheet( q_style );

    quiz_layout->addWidget( input_wrong1_ );
    quiz_layout->addWidget( input_wrong2_ );
    quiz_layout->addWidget( input_wrong3_ );

    dialog_layout->addWidget( quiz_container_ );
    quiz_container_->hide();

    dialog_layout->addStretch();

    QHBoxLayout* btn_layout = new QHBoxLayout();
    btn_cancel_ = new QPushButton( tr( "Cancel" ), dialog );
    btn_save_ = new QPushButton( tr( "Save" ), dialog );

    btn_cancel_->setCursor( Qt::PointingHandCursor );
    btn_save_->setCursor( Qt::PointingHandCursor );

    btn_cancel_->setStyleSheet(
        "background-color: transparent; color: #aaa; border: 1px solid #555; padding: 8px 16px; "
        "border-radius: 4px;" );
    btn_save_->setStyleSheet(
        "background-color: #0078d4; color: white; padding: 8px 16px; border-radius: 4px; "
        "font-weight: bold;" );

    btn_layout->addStretch();
    btn_layout->addWidget( btn_cancel_ );
    btn_layout->addWidget( btn_save_ );

    dialog_layout->addLayout( btn_layout );
}

void AddCardOverlay::setupConnections() {
    connect( combo_question_type_, QOverload<int>::of( &QComboBox::currentIndexChanged ), this,
             &AddCardOverlay::onQuestionTypeChanged );

    connect( combo_answer_type_, QOverload<int>::of( &QComboBox::currentIndexChanged ), this,
             &AddCardOverlay::onAnswerTypeChanged );

    connect( btn_select_image_, &QPushButton::clicked, this, &AddCardOverlay::selectImageFile );
    connect( btn_select_sound_, &QPushButton::clicked, this, &AddCardOverlay::selectSoundFile );

    connect( btn_cancel_, &QPushButton::clicked, this, &AddCardOverlay::cancelled );

    connect( btn_save_, &QPushButton::clicked, this, [this]() {
        QString correct = input_correct_answer_->text().trimmed();
        if ( correct.isEmpty() ) {
            QMessageBox::warning( this, tr( "Error" ), tr( "You must enter a correct answer!" ) );
            return;
        }

        DraftCard draft;
        draft.correct_answer = correct.toStdString();
        draft.answer_type = (AnswerType)combo_answer_type_->currentData().toInt();

        int q_type = combo_question_type_->currentIndex();
        if ( q_type == 0 ) {
            QString txt = input_question_text_->toPlainText().trimmed();
            if ( txt.isEmpty() ) {
                QMessageBox::warning( this, tr( "Error" ), tr( "Question cannot be empty!" ) );
                return;
            }
            draft.question = TextContent{ txt.toStdString() };
        } else if ( q_type == 1 ) {
            if ( selected_image_path_.isEmpty() ) {
                QMessageBox::warning( this, tr( "Error" ), tr( "You must select an image!" ) );
                return;
            }
            QString final_name = copyFileToMedia( selected_image_path_, "images" );
            if ( final_name.isEmpty() ) return;
            draft.question = ImageContent{ final_name.toStdString() };
        } else if ( q_type == 2 ) {
            if ( selected_sound_path_.isEmpty() ) {
                QMessageBox::warning( this, tr( "Error" ), tr( "You must select an audio file!" ) );
                return;
            }
            QString final_name = copyFileToMedia( selected_sound_path_, "sounds" );
            if ( final_name.isEmpty() ) return;
            draft.question = SoundContent{ final_name.toStdString() };
        }
        if ( draft.answer_type == AnswerType::TEXT_CHOICE ) {
            if ( !input_wrong1_->text().isEmpty() )
                draft.wrong_answers.push_back( input_wrong1_->text().toStdString() );
            if ( !input_wrong2_->text().isEmpty() )
                draft.wrong_answers.push_back( input_wrong2_->text().toStdString() );
            if ( !input_wrong3_->text().isEmpty() )
                draft.wrong_answers.push_back( input_wrong3_->text().toStdString() );

            if ( draft.wrong_answers.empty() ) {
                QMessageBox::warning( this, tr( "Warning" ),
                                      tr( "You selected Quiz mode but didn't provide wrong "
                                          "answers. The card will work but will be easy!" ) );
            }
        }

        emit cardSaved( draft );
    } );
}

void AddCardOverlay::onQuestionTypeChanged( int index ) {
    stack_question_content_->setCurrentIndex( index );
}

void AddCardOverlay::onAnswerTypeChanged( int index ) {
    AnswerType type = (AnswerType)combo_answer_type_->itemData( index ).toInt();
    if ( type == AnswerType::TEXT_CHOICE ) {
        quiz_container_->show();
    } else {
        quiz_container_->hide();
    }
}

void AddCardOverlay::selectImageFile() {
    QString startDir = getRootPath();

    QString path = QFileDialog::getOpenFileName( this, tr( "Choose Image" ), startDir,
                                                 tr( "Images (*.png *.jpg *.jpeg *.bmp)" ) );
    if ( !path.isEmpty() ) {
        selected_image_path_ = path;
        QFileInfo fi( path );
        label_image_preview_->setText( tr( "Selected: " ) + fi.fileName() );
        label_image_preview_->setStyleSheet(
            "color: #4caf50; border: 1px solid #4caf50; font-weight: bold;" );
    }
}

void AddCardOverlay::selectSoundFile() {
    QString startDir = getRootPath();

    QString path = QFileDialog::getOpenFileName( this, tr( "Choose Audio" ), startDir,
                                                 tr( "Audio (*.mp3 *.wav *.ogg)" ) );
    if ( !path.isEmpty() ) {
        selected_sound_path_ = path;
        QFileInfo fi( path );
        label_sound_info_->setText( tr( "Selected: " ) + fi.fileName() );
        label_sound_info_->setStyleSheet(
            "color: #4caf50; border: 1px solid #4caf50; font-weight: bold;" );
    }
}

QString AddCardOverlay::copyFileToMedia( const QString& sourcePath, const string& subfolder ) {
    QDir dir( getRootPath() );

    qDebug() << "Root dla mediów:" << dir.absolutePath();

    if ( !dir.exists( "data" ) ) dir.mkdir( "data" );
    dir.cd( "data" );
    if ( !dir.exists( "media" ) ) dir.mkdir( "media" );
    dir.cd( "media" );

    QString sub = QString::fromStdString( subfolder );
    if ( !dir.exists( sub ) ) dir.mkdir( sub );
    dir.cd( sub );
    QFileInfo sourceInfo( sourcePath );
    QString ext = sourceInfo.suffix();
    QString newName = QString::number( QDateTime::currentMSecsSinceEpoch() ) + "." + ext;

    QString destPath = dir.filePath( newName );

    qDebug() << "Kopiowanie z:" << sourcePath;
    qDebug() << "Do:" << destPath;

    if ( QFile::copy( sourcePath, destPath ) ) {
        return QString::fromStdString( subfolder ) + "/" + newName;
    } else {
        qCritical() << "Błąd kopiowania pliku!";
        QMessageBox::critical( this, tr( "Save Error" ),
                               tr( "Could not copy file to app directory!\n"
                                   "Destination: " ) +
                                   destPath );
        return "";
    }
}