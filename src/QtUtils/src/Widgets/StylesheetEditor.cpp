/*
   Copyright (C) 2017-2025 Peter S. Zhigalov <peter.zhigalov@gmail.com>

   This file is part of the `QtUtils' library.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "StylesheetEditor.h"
#include "StylesheetEditor_p.h"

#include <algorithm>

#include <QApplication>
#include <QEvent>
#include <QTimer>
#include <QStyle>
#include <QPainter>
#include <QFile>
#include <QFont>
#include <QFontInfo>
#include <QPoint>
#include <QSize>
#include <QRect>
#include <QString>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSpacerItem>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QSyntaxHighlighter>

namespace {

const int LINE_NUMBER_AREA_MARGIN = 3;
const int CODE_EDITOR_TAB_WIDTH   = 4;
const int WINDOW_DEFAULT_WIDTH    = 640;
const int WINDOW_DEFAULT_HEIGHT   = 480;

const QString INDENT_STRING = QString().fill(QChar::fromLatin1(' '), CODE_EDITOR_TAB_WIDTH);

static QFont getMonospaceFont()
{
    QFont font;
    font.setFamily(QString::fromLatin1("monospace"));
    if(QFontInfo(font).fixedPitch())
        return font;
#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
    font.setStyleHint(QFont::Monospace);
    if(QFontInfo(font).fixedPitch())
        return font;
#endif
    font.setFamily(QString::fromLatin1("Courier New"));
    if(QFontInfo(font).fixedPitch())
        return font;
    font.setFamily(QString::fromLatin1("courier"));
    if(QFontInfo(font).fixedPitch())
        return font;
    font.setStyleHint(QFont::TypeWriter);
    if(QFontInfo(font).fixedPitch())
        return font;
    font.setFixedPitch(true);
    return font;
}

static int getWidth(const QFont &font, const QChar &symbol = QChar::fromLatin1('M'))
{
    const QFontMetrics fontMetrics(font);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    return fontMetrics.horizontalAdvance(symbol);
#else
    return fontMetrics.width(symbol);
#endif
}

static void overrideLeftToRight(QWidget *widget, bool enabled)
{
    QList<QWidget*> widgets = widget->findChildren<QWidget*>();
    widgets.prepend(widget);
    if(enabled)
    {
        for(QList<QWidget*>::ConstIterator it = widgets.constBegin(); it != widgets.constEnd(); ++it)
            (*it)->setLayoutDirection(Qt::LeftToRight);
    }
    else
    {
        for(QList<QWidget*>::ConstIterator it = widgets.constBegin(); it != widgets.constEnd(); ++it)
            (*it)->unsetLayoutDirection();
    }
}

} // namespace

namespace StylesheetEditorImpl {

// ====================================================================================================

// https://github.com/qt/qttools/blob/5.12/src/designer/src/lib/shared/csshighlighter.cpp
class CssHighlighter : public QSyntaxHighlighter
{
public:
    CssHighlighter(QTextDocument *document)
        : QSyntaxHighlighter(document)
    {}

protected:
    void highlightBlock(const QString &text) Q_DECL_OVERRIDE
    {
        enum Token { ALNUM, LBRACE, RBRACE, COLON, SEMICOLON, COMMA, QUOTE, SLASH, STAR };
        static const int transitions[10][9] =
        {
            // ALNUM    , LBRACE    , RBRACE    , COLON     , SEMICOLON , COMMA     , QUOTE     , SLASH         , STAR
            { Selector  , Property  , Selector  , Pseudo    , Property  , Selector  , Quote     , MaybeComment  , Selector          }, // Selector
            { Property  , Property  , Selector  , Value     , Property  , Property  , Quote     , MaybeComment  , Property          }, // Property
            { Value     , Property  , Selector  , Value     , Property  , Value     , Quote     , MaybeComment  , Value             }, // Value
            { Pseudo1   , Property  , Selector  , Pseudo2   , Selector  , Selector  , Quote     , MaybeComment  , Pseudo            }, // Pseudo
            { Pseudo1   , Property  , Selector  , Pseudo    , Selector  , Selector  , Quote     , MaybeComment  , Pseudo1           }, // Pseudo1
            { Pseudo2   , Property  , Selector  , Pseudo    , Selector  , Selector  , Quote     , MaybeComment  , Pseudo2           }, // Pseudo2
            { Quote     , Quote     , Quote     , Quote     , Quote     , Quote     , -1        , Quote         , Quote             }, // Quote
            { -1        , -1        , -1        , -1        , -1        , -1        , -1        , -1            , Comment           }, // MaybeComment
            { Comment   , Comment   , Comment   , Comment   , Comment   , Comment   , Comment   , Comment       , MaybeCommentEnd   }, // Comment
            { Comment   , Comment   , Comment   , Comment   , Comment   , Comment   , Comment   , -1            , MaybeCommentEnd   }  // MaybeCommentEnd
        };

        int lastIndex = 0;
        bool lastWasSlash = false;
        int state = previousBlockState(), save_state;
        if(state == -1)
        {
            // As long as the text is empty, leave the state undetermined
            if(text.isEmpty())
            {
                setCurrentBlockState(-1);
                return;
            }
            // The initial state is based on the precense of a : and the absense of a {.
            // This is because Qt style sheets support both a full stylesheet as well as
            // an inline form with just properties.
            state = save_state = (text.indexOf(QLatin1Char(':')) > -1 &&
                                  text.indexOf(QLatin1Char('{')) == -1) ? Property : Selector;
        }
        else
        {
            save_state = state >> 16;
            state &= 0x00ff;
        }

        if(state == MaybeCommentEnd)
            state = Comment;
        else if (state == MaybeComment)
            state = save_state;

        for(int i = 0; i < text.length(); i++)
        {
            int token = ALNUM;
            const QChar c = text.at(i);
            const char a = c.toLatin1();

            if(state == Quote)
            {
                if(a == '\\')
                {
                    lastWasSlash = true;
                }
                else
                {
                    if((a == '\"') && !lastWasSlash)
                        token = QUOTE;
                    lastWasSlash = false;
                }
            }
            else
            {
                switch(a)
                {
                case '{': token = LBRACE; break;
                case '}': token = RBRACE; break;
                case ':': token = COLON; break;
                case ';': token = SEMICOLON; break;
                case ',': token = COMMA; break;
                case '\"': token = QUOTE; break;
                case '/': token = SLASH; break;
                case '*': token = STAR; break;
                default: break;
                }
            }

            int new_state = transitions[state][token];

            if(new_state != state)
            {
                bool include_token = new_state == MaybeCommentEnd || (state == MaybeCommentEnd && new_state!= Comment) || state == Quote;
                highlight(text, lastIndex, i-lastIndex+include_token, state);

                if(new_state == Comment)
                    lastIndex = i - 1; // include the slash and star
                else
                    lastIndex = i + ((token == ALNUM || new_state == Quote) ? 0 : 1);
            }

            if(new_state == -1)
            {
                state = save_state;
            }
            else if(state <= Pseudo2)
            {
                save_state = state;
                state = new_state;
            }
            else
            {
                state = new_state;
            }
        }

        highlight(text, lastIndex, text.length() - lastIndex, state);
        setCurrentBlockState(state + (save_state << 16));
    }

    void highlight(const QString &text, int start, int length, int state)
    {
        if(start >= text.length() || length <= 0)
            return;

        switch (state)
        {
        case Selector:
            setFormat(start, length, Qt::darkRed);
            break;
        case Property:
            setFormat(start, length, Qt::blue);
            break;
        case Value:
            setFormat(start, length, Qt::black);
            break;
        case Pseudo1:
            setFormat(start, length, Qt::darkRed);
            break;
        case Pseudo2:
            setFormat(start, length, Qt::darkRed);
            break;
        case Quote:
            setFormat(start, length, Qt::darkMagenta);
            break;
        case Comment:
        case MaybeCommentEnd:
            setFormat(start, length, Qt::darkGreen);
            break;
        default:
            break;
        }
    }

private:
    enum State { Selector, Property, Value, Pseudo, Pseudo1, Pseudo2, Quote, MaybeComment, Comment, MaybeCommentEnd };
};

// ====================================================================================================

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_lineNumberArea(new LineNumberArea(this))
{
    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumbersAreaWidth()));
    connect(this, SIGNAL(updateRequest(QRect,int)), this,SLOT(updateLineNumbersArea(QRect,int)));
    updateLineNumbersAreaWidth();
    new CssHighlighter(document());
    const QString styleSheet = QString::fromLatin1("background: white; color: black;");
    setStyleSheet(styleSheet);
    viewport()->setStyleSheet(styleSheet);
}

int CodeEditor::lineNumbersAreaWidth()
{
    int digits = 1;
    for(int max = qMax(1, blockCount()); max >= 10; max /= 10)
        digits++;
    return 2 * LINE_NUMBER_AREA_MARGIN + getWidth(font()) * digits;
}

void CodeEditor::updateLineNumbersAreaWidth()
{
    if(layoutDirection() == Qt::RightToLeft)
        setViewportMargins(0, 0, lineNumbersAreaWidth(), 0);
    else
        setViewportMargins(lineNumbersAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumbersArea(const QRect &rect, int dy)
{
    if(dy)
        m_lineNumberArea->scroll(0, dy);
    else
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
    if(rect.contains(viewport()->rect()))
        updateLineNumbersAreaWidth();
}

void CodeEditor::updateLineNumbersAreaGeometry()
{
    const QRect cr = contentsRect();
    if(layoutDirection() == Qt::RightToLeft)
        m_lineNumberArea->setGeometry(QRect(cr.left() + cr.width() - lineNumbersAreaWidth(), cr.top(), lineNumbersAreaWidth(), cr.height()));
    else
        m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumbersAreaWidth(), cr.height()));
}

void CodeEditor::showSearchDialog()
{
    if(m_searchDialog.isNull())
        m_searchDialog = new SearchDialog(this);
    const QString selectedText = textCursor().selectedText();
    if(!selectedText.isEmpty())
        m_searchDialog->setText(selectedText);
    const bool dialogIsHidden = m_searchDialog->isHidden();
    m_searchDialog->show();
    m_searchDialog->raise();
    m_searchDialog->activateWindow();
    if(dialogIsHidden)
    {
        const QSize dialogHalfSize = m_searchDialog->frameSize() / 2;
        const QPoint dialogLocalPos = geometry().center() - QPoint(dialogHalfSize.width(), dialogHalfSize.height());
        m_searchDialog->move(mapToGlobal(dialogLocalPos));
    }
}

bool CodeEditor::toggleCommentSelection()
{
    QTextCursor cursor = textCursor();
    if(!cursor.hasSelection())
        return false;

    int startPos = cursor.anchor();
    int endPos = cursor.position();
    if(startPos > endPos)
        std::swap(startPos, endPos);

    static const QString beginComment = QString::fromLatin1("/*");
    static const QString endComment = QString::fromLatin1("*/");

    cursor.setPosition(startPos, QTextCursor::MoveAnchor);
    cursor.setPosition(startPos + beginComment.length(), QTextCursor::KeepAnchor);
    const QString startText = cursor.selectedText();

    cursor.setPosition(endPos, QTextCursor::MoveAnchor);
    cursor.setPosition(endPos - endComment.length(), QTextCursor::KeepAnchor);
    const QString endText = cursor.selectedText();

    cursor.beginEditBlock();
    if(startText == beginComment && endText == endComment)
    {
        cursor.setPosition(startPos, QTextCursor::MoveAnchor);
        cursor.setPosition(startPos + beginComment.length(), QTextCursor::KeepAnchor);
        cursor.removeSelectedText();

        cursor.setPosition(endPos - beginComment.length(), QTextCursor::MoveAnchor);
        cursor.setPosition(endPos - beginComment.length() - endComment.length(), QTextCursor::KeepAnchor);
        cursor.removeSelectedText();

        cursor.setPosition(startPos, QTextCursor::MoveAnchor);
        cursor.setPosition(endPos - beginComment.length() - endComment.length(), QTextCursor::KeepAnchor);
    }
    else
    {
        cursor.setPosition(startPos, QTextCursor::MoveAnchor);
        cursor.insertText(beginComment);

        cursor.setPosition(endPos + beginComment.length(), QTextCursor::MoveAnchor);
        cursor.insertText(endComment);

        cursor.setPosition(startPos, QTextCursor::MoveAnchor);
        cursor.setPosition(endPos + beginComment.length() + endComment.length(), QTextCursor::KeepAnchor);
    }
    cursor.endEditBlock();

    setTextCursor(cursor);
    return true;
}

bool CodeEditor::indentSelectedLines(IndentOperation operation)
{
    QTextCursor cursor = textCursor();
    if(!cursor.hasSelection())
        return false;

    int startPos = cursor.anchor();
    int endPos = cursor.position();
    if(startPos > endPos)
        std::swap(startPos, endPos);

    cursor.setPosition(startPos, QTextCursor::MoveAnchor);
    if(cursor.atBlockEnd())
    {
        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    }
    const int startBlock = cursor.block().blockNumber();
    startPos = cursor.anchor();

    cursor.setPosition(endPos, QTextCursor::MoveAnchor);
    if(cursor.atBlockStart())
    {
        cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
    }
    const int endBlock = cursor.block().blockNumber();
    endPos = cursor.anchor();

    if(startPos >= endPos)
        return false;

    cursor.setPosition(startPos, QTextCursor::MoveAnchor);
    cursor.beginEditBlock();
    for(int i = 0, blocksDelta = endBlock - startBlock; i <= blocksDelta; ++i)
    {
        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        switch(operation)
        {
        case INDENT_INCREASE_LEVEL:
            if(!cursor.block().text().simplified().isEmpty())
                cursor.insertText(INDENT_STRING);
            break;
        case INDENT_DECREASE_LEVEL:
            if(cursor.block().text().startsWith(INDENT_STRING))
            {
                cursor.setPosition(cursor.position() + INDENT_STRING.length(), QTextCursor::KeepAnchor);
                cursor.removeSelectedText();
            }
            break;
        }
        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
    }
    cursor.endEditBlock();

    cursor.setPosition(startPos, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    while(cursor.block().blockNumber() < endBlock)
        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

    setTextCursor(cursor);
    return true;
}

bool CodeEditor::insertIndentSequence()
{
    QTextCursor cursor = textCursor();
    if(cursor.hasSelection())
        return false;

    cursor.beginEditBlock();
    cursor.insertText(INDENT_STRING);
    cursor.endEditBlock();
    setTextCursor(cursor);
    return true;
}

bool CodeEditor::removeIndentSequence(RemoveDirection direction)
{
    QTextCursor cursor = textCursor();
    if(cursor.hasSelection())
        return false;

    int offset = 0;
    switch(direction)
    {
    case DIRECTION_BACKWARD:
        offset = - INDENT_STRING.length();
        break;
    case DIRECTION_FORWARD:
        offset = INDENT_STRING.length();
        break;
    }
    cursor.setPosition(cursor.position() + offset, QTextCursor::KeepAnchor);
    if(cursor.selectedText() != INDENT_STRING)
        return false;

    cursor.beginEditBlock();
    cursor.removeSelectedText();
    cursor.endEditBlock();
    setTextCursor(cursor);
    return true;
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    updateLineNumbersAreaGeometry();
}

void CodeEditor::changeEvent(QEvent *event)
{
    QPlainTextEdit::changeEvent(event);
    if(event->type() == QEvent::StyleChange)
    {
        const int tabStopWidth = CODE_EDITOR_TAB_WIDTH * getWidth(font());
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
        setTabStopDistance(static_cast<qreal>(tabStopWidth));
#else
        setTabStopWidth(tabStopWidth);
#endif
    }
    else if(event->type() == QEvent::LayoutDirectionChange)
    {
        updateLineNumbersAreaWidth();
        updateLineNumbersAreaGeometry();
    }
}

void CodeEditor::keyPressEvent(QKeyEvent *event)
{
    if(event->matches(QKeySequence::Find))
        showSearchDialog();
    else if(event->matches(QKeySequence::Save))
        Q_EMIT applyRequested();
    else if((event->key() == Qt::Key_Slash || event->key() == Qt::Key_Question) && event->modifiers().testFlag(Qt::ControlModifier) && toggleCommentSelection())
        return;
    else if(((event->key() == Qt::Key_Tab && event->modifiers().testFlag(Qt::ShiftModifier)) || (event->key() == Qt::Key_Backtab)) && indentSelectedLines(INDENT_DECREASE_LEVEL))
        return;
    else if(event->key() == Qt::Key_Tab && indentSelectedLines(INDENT_INCREASE_LEVEL))
        return;
    else if(event->key() == Qt::Key_Tab && insertIndentSequence())
        return;
    else if(event->key() == Qt::Key_Delete && removeIndentSequence(DIRECTION_FORWARD))
        return;
    else if(event->key() == Qt::Key_Backspace && removeIndentSequence(DIRECTION_BACKWARD))
        return;
    else
        QPlainTextEdit::keyPressEvent(event);
}

void CodeEditor::hideEvent(QHideEvent *event)
{
    if(!m_searchDialog.isNull())
        m_searchDialog->close();
    QPlainTextEdit::hideEvent(event);
}

void CodeEditor::repaintLineNumbersArea(const QRect &rect)
{
    QTextCursor cursor = textCursor();
    int startSelectedPos = cursor.anchor();
    int endSelectedPos = cursor.position();
    if(startSelectedPos > endSelectedPos)
        std::swap(startSelectedPos, endSelectedPos);
    cursor.setPosition(startSelectedPos, QTextCursor::MoveAnchor);
    const int startSelectedBlock = cursor.block().blockNumber();
    cursor.setPosition(endSelectedPos, QTextCursor::MoveAnchor);
    const int endSelectedBlock = cursor.block().blockNumber();

    QPainter painter(m_lineNumberArea);
    painter.fillRect(rect, Qt::lightGray);
    painter.setPen(Qt::black);
    painter.setLayoutDirection(Qt::LeftToRight);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + static_cast<int>(blockBoundingRect(block).height());

    while(block.isValid() && top <= rect.bottom())
    {
        blockNumber++;
        if(block.isVisible() && bottom >= rect.top())
        {
            if(blockNumber > startSelectedBlock && blockNumber <= endSelectedBlock + 1)
                painter.fillRect(QRect(QPoint(0, top), QPoint(m_lineNumberArea->width(), bottom)), Qt::gray);
            const QRect textRect = QRect(0, top, m_lineNumberArea->width() - LINE_NUMBER_AREA_MARGIN, fontMetrics().height());
            painter.drawText(textRect, Qt::AlignRight, QString::number(blockNumber));
        }
        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(blockBoundingRect(block).height());
    }
}

// ====================================================================================================

LineNumberArea::LineNumberArea(CodeEditor *editor)
    : QWidget(editor)
    , m_codeEditor(editor)
{}

QSize LineNumberArea::sizeHint() const
{
    return QSize(m_codeEditor->lineNumbersAreaWidth(), m_codeEditor->sizeHint().height());
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    m_codeEditor->repaintLineNumbersArea(event->rect());
}

// ====================================================================================================

SearchDialog::SearchDialog(CodeEditor *editor)
    : QDialog(editor->window())
    , m_codeEditor(editor)
    , m_searchLineEdit(new QLineEdit(this))
    , m_searchPushButton(new QPushButton(QString::fromLatin1("Find"), this))
    , m_findBackwardCheckBox(new QCheckBox(QString::fromLatin1("Find Backward"), this))
    , m_findCaseSensitivelyCheckBox(new QCheckBox(QString::fromLatin1("Find Case Sensitively"), this))
    , m_findWholeWordsCheckBox(new QCheckBox(QString::fromLatin1("Find Whole Words"), this))
{
    setWindowTitle(QString::fromLatin1("Search"));
    setWindowFlags(Qt::Dialog
                   | Qt::MSWindowsFixedSizeDialogHint
                   | Qt::CustomizeWindowHint
                   | Qt::WindowTitleHint
                   | Qt::WindowSystemMenuHint
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
                   | Qt::WindowCloseButtonHint
#endif
                   );
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_searchLineEdit);
    layout->addWidget(m_findBackwardCheckBox);
    layout->addWidget(m_findCaseSensitivelyCheckBox);
    layout->addWidget(m_findWholeWordsCheckBox);
    layout->addWidget(m_searchPushButton);
    connect(m_searchPushButton, SIGNAL(clicked(bool)), this, SLOT(onSearchClicked()));
}

void SearchDialog::setText(const QString &text)
{
    m_searchLineEdit->setText(text);
}

void SearchDialog::onSearchClicked()
{
    const QString searchString = m_searchLineEdit->text();
    if(searchString.isEmpty())
        return;
    QTextDocument *document = m_codeEditor->document();
    QTextCursor cursor = m_codeEditor->textCursor();
    QTextDocument::FindFlags flags;
    if(m_findBackwardCheckBox->isChecked())
        flags |= QTextDocument::FindBackward;
    if(m_findCaseSensitivelyCheckBox->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    if(m_findWholeWordsCheckBox->isChecked())
        flags |= QTextDocument::FindWholeWords;
    QTextCursor newCursor = document->find(searchString, cursor, flags);
    if(!newCursor.isNull())
        m_codeEditor->setTextCursor(newCursor);
}

void SearchDialog::showEvent(QShowEvent *event)
{
    m_searchLineEdit->setFocus();
    QDialog::showEvent(event);
}

// ====================================================================================================

LayoutDirectionComboBox::LayoutDirectionComboBox(QWidget *parent)
    : QComboBox(parent)
    , m_updatePending(false)
{
    addItem(QString::fromLatin1("LeftToRight"), Qt::LeftToRight);
    addItem(QString::fromLatin1("RightToLeft"), Qt::RightToLeft);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
    addItem(QString::fromLatin1("LayoutDirectionAuto"), Qt::LayoutDirectionAuto);
#endif
    connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged(int)));
    updateState();
    qApp->installEventFilter(this);
}

bool LayoutDirectionComboBox::eventFilter(QObject *watched, QEvent *event)
{
    if(event && event->type() == QEvent::ApplicationLayoutDirectionChange)
        updateStateLater();
    return QComboBox::eventFilter(watched, event);
}

void LayoutDirectionComboBox::updateState()
{
    setCurrentIndex(findData(qApp->layoutDirection()));
    m_updatePending = false;
}

void LayoutDirectionComboBox::updateStateLater()
{
    if(m_updatePending)
        return;
    m_updatePending = true;
    QTimer::singleShot(0, this, SLOT(updateState()));
}

void LayoutDirectionComboBox::onCurrentIndexChanged(int index)
{
    qApp->setLayoutDirection(static_cast<Qt::LayoutDirection>(itemData(index).toInt()));
    updateStateLater();
}

// ====================================================================================================

} // namespace StylesheetEditorImpl

typedef StylesheetEditorImpl::CodeEditor CodeEditor;
typedef StylesheetEditorImpl::LayoutDirectionComboBox LayoutDirectionComboBox;

struct StylesheetEditor::Impl
{
    const QString originalStyleSheet;

    LayoutDirectionComboBox * const layoutDirectionComboBox;
    QCheckBox * const autoApplyCheckBox;
    QCheckBox * const protectCheckBox;
    QPushButton * const searchButton;
    CodeEditor * const codeEditor;
    QPushButton * const resetButton;
    QPushButton * const applyButton;
    QPushButton * const readButton;

    explicit Impl(StylesheetEditor *stylesheetEditor)
        : originalStyleSheet(qApp->styleSheet())
        , layoutDirectionComboBox(new LayoutDirectionComboBox(stylesheetEditor))
        , autoApplyCheckBox(new QCheckBox(QString::fromLatin1("Auto apply style"), stylesheetEditor))
        , protectCheckBox(new QCheckBox(QString::fromLatin1("Protect stylesheet editor"), stylesheetEditor))
        , searchButton(new QPushButton(QString::fromLatin1("Search"), stylesheetEditor))
        , codeEditor(new CodeEditor(stylesheetEditor))
        , resetButton(new QPushButton(QString::fromLatin1("Reset style"), stylesheetEditor))
        , applyButton(new QPushButton(QString::fromLatin1("Apply style"), stylesheetEditor))
        , readButton(new QPushButton(QString::fromLatin1("Read style"), stylesheetEditor))
    {
        QGridLayout *layout = new QGridLayout(stylesheetEditor);

        layout->addWidget(layoutDirectionComboBox, 0, 0);
        layout->addWidget(autoApplyCheckBox, 0, 1);
        layout->addWidget(protectCheckBox, 0, 2);
        layout->addItem(new QSpacerItem(0, 0), 0, 3);
        layout->addWidget(searchButton, 0, 4);

        layout->addWidget(codeEditor, 1, 0, 1, 5);

        layout->addWidget(resetButton, 2, 0);
        layout->addWidget(applyButton, 2, 1, 1, 3);
        layout->addWidget(readButton, 2, 4);
    }
};

StylesheetEditor::StylesheetEditor(QWidget *parent)
    : QDialog(parent)
    , m_impl(new Impl(this))
{
    resize(WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT);
    setWindowFlags(Qt::Window);

    m_impl->codeEditor->setPlainText(m_impl->originalStyleSheet);
    setProtected(true);
    updateProtection();

    connect(m_impl->searchButton, SIGNAL(clicked(bool)), m_impl->codeEditor, SLOT(showSearchDialog()));
    connect(m_impl->resetButton, SIGNAL(clicked(bool)), this, SLOT(resetStyleSheet()));
    connect(m_impl->applyButton, SIGNAL(clicked(bool)), this, SLOT(applyStylesheet()));
    connect(m_impl->readButton, SIGNAL(clicked(bool)), this, SLOT(readStyleSheet()));
    connect(m_impl->protectCheckBox, SIGNAL(stateChanged(int)), this, SLOT(updateProtection()));
    connect(m_impl->autoApplyCheckBox, SIGNAL(stateChanged(int)), this, SLOT(updateAutoApply()));
    connect(m_impl->codeEditor, SIGNAL(applyRequested()), this, SLOT(applyStylesheet()));

    m_impl->codeEditor->setFocus();
}

StylesheetEditor::~StylesheetEditor()
{}

bool StylesheetEditor::isProtected() const
{
    return m_impl->protectCheckBox->isChecked();
}

void StylesheetEditor::setProtected(bool isProtected)
{
    m_impl->protectCheckBox->setChecked(isProtected);
}

void StylesheetEditor::repolishSelf()
{
    QList<QWidget*> widgets = findChildren<QWidget*>();
    widgets.prepend(this);
    for(QList<QWidget*>::ConstIterator it = widgets.constBegin(); it != widgets.constEnd(); ++it)
    {
        QWidget *widget = *it;
        QStyle *style = widget->style();
        if(!style)
            style = qApp->style();
        style->unpolish(widget);
        style->polish(widget);
    }
}

void StylesheetEditor::applyStylesheet()
{
    qApp->setStyleSheet(m_impl->codeEditor->toPlainText());
    qApp->style()->unpolish(qApp);
    qApp->style()->polish(qApp);
    const QWidgetList allWidgets = qApp->allWidgets();
    for(QWidgetList::ConstIterator it = allWidgets.constBegin(), itEnd = allWidgets.constEnd(); it != itEnd; ++it)
    {
        (*it)->update();
        (*it)->repaint();
    }
    m_impl->codeEditor->setFont(getMonospaceFont());
}

void StylesheetEditor::resetStyleSheet()
{
    m_impl->codeEditor->setPlainText(m_impl->originalStyleSheet);
    applyStylesheet();
}

void StylesheetEditor::readStyleSheet()
{
    m_impl->codeEditor->setPlainText(qApp->styleSheet());
}

void StylesheetEditor::updateProtection()
{
    overrideLeftToRight(this, isProtected());
    QString styleSheet;
    if(isProtected())
    {
        QFile style(QString::fromLatin1(":/style/StylesheetEditor.qss"));
        if(style.open(QIODevice::ReadOnly))
            styleSheet = QString::fromUtf8(style.readAll());
    }
    setStyleSheet(styleSheet);
    m_impl->codeEditor->setFont(getMonospaceFont());
}

void StylesheetEditor::updateAutoApply()
{
    if(m_impl->autoApplyCheckBox->isChecked())
        connect(m_impl->codeEditor, SIGNAL(textChanged()), this, SLOT(applyStylesheet()));
    else
        disconnect(m_impl->codeEditor, SIGNAL(textChanged()), this, SLOT(applyStylesheet()));
}

void StylesheetEditor::changeEvent(QEvent *event)
{
    QDialog::changeEvent(event);
    if(event->type() == QEvent::LayoutDirectionChange)
        QTimer::singleShot(0, this, SLOT(repolishSelf()));
}

