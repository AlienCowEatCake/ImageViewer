/*
   Copyright (C) 2017-2018 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined (QTUTILS_STYLESHEET_EDITOR_P_H_INCLUDED)
#define QTUTILS_STYLESHEET_EDITOR_P_H_INCLUDED

#include "StylesheetEditor.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QPointer>

namespace StylesheetEditorImpl {

// ====================================================================================================

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
    Q_DISABLE_COPY(CodeEditor)

signals:
    void applyRequested();

public:
    CodeEditor(QWidget *parent = NULL);

    void repaintLineNumbersArea(const QRect &rect);
    int lineNumbersAreaWidth();

protected:
    void resizeEvent(QResizeEvent *event);
    void changeEvent(QEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void hideEvent(QHideEvent *event);

private:
    enum IndentOperation
    {
        INDENT_INCREASE_LEVEL,
        INDENT_DECREASE_LEVEL
    };

    enum RemoveDirection
    {
        DIRECTION_FORWARD,
        DIRECTION_BACKWARD
    };

private slots:
    void updateLineNumbersAreaWidth();
    void updateLineNumbersArea(const QRect &rect, int dy);
    void showSearchDialog();
    bool toggleCommentSelection();
    bool indentSelectedLines(IndentOperation operation);
    bool insertIndentSequence();
    bool removeIndentSequence(RemoveDirection direction);

private:
    QWidget *m_lineNumberArea;
    QPointer<QDialog> m_searchDialog;
};

// ====================================================================================================

class LineNumberArea : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(LineNumberArea)

public:
    LineNumberArea(CodeEditor *editor);

    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent *event);

private:
    CodeEditor *m_codeEditor;
};

// ====================================================================================================

class SearchDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(SearchDialog)

public:
    SearchDialog(CodeEditor *editor);

public slots:
    void onSearchClicked();

protected:
    void showEvent(QShowEvent *event);

private:
    CodeEditor *m_codeEditor;
    QLineEdit *m_searchLineEdit;
    QPushButton *m_searchPushButton;
    QCheckBox *m_findBackwardCheckBox;
    QCheckBox *m_findCaseSensitivelyCheckBox;
    QCheckBox *m_findWholeWordsCheckBox;
};

// ====================================================================================================

} // namespace StylesheetEditorImpl

#endif // QTUTILS_STYLESHEET_EDITOR_P_H_INCLUDED

