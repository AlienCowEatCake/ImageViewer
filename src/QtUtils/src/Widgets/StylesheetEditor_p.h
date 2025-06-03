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

#if !defined (QTUTILS_STYLESHEET_EDITOR_P_H_INCLUDED)
#define QTUTILS_STYLESHEET_EDITOR_P_H_INCLUDED

#include "StylesheetEditor.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QPointer>

namespace StylesheetEditorImpl {

// ====================================================================================================

class SearchDialog;

// ====================================================================================================

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
    Q_DISABLE_COPY(CodeEditor)

Q_SIGNALS:
    void applyRequested();

public:
    explicit CodeEditor(QWidget *parent = Q_NULLPTR);

    void repaintLineNumbersArea(const QRect &rect);
    int lineNumbersAreaWidth();

protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void changeEvent(QEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void hideEvent(QHideEvent *event) Q_DECL_OVERRIDE;

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

private Q_SLOTS:
    void updateLineNumbersAreaWidth();
    void updateLineNumbersArea(const QRect &rect, int dy);
    void updateLineNumbersAreaGeometry();
    void showSearchDialog();
    bool toggleCommentSelection();
    bool indentSelectedLines(IndentOperation operation);
    bool insertIndentSequence();
    bool removeIndentSequence(RemoveDirection direction);

private:
    QWidget *m_lineNumberArea;
    QPointer<SearchDialog> m_searchDialog;
};

// ====================================================================================================

class LineNumberArea : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(LineNumberArea)

public:
    explicit LineNumberArea(CodeEditor *editor);

    QSize sizeHint() const Q_DECL_OVERRIDE;

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    CodeEditor *m_codeEditor;
};

// ====================================================================================================

class SearchDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(SearchDialog)

public:
    explicit SearchDialog(CodeEditor *editor);
    void setText(const QString &text);

public Q_SLOTS:
    void onSearchClicked();

protected:
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

private:
    CodeEditor *m_codeEditor;
    QLineEdit *m_searchLineEdit;
    QPushButton *m_searchPushButton;
    QCheckBox *m_findBackwardCheckBox;
    QCheckBox *m_findCaseSensitivelyCheckBox;
    QCheckBox *m_findWholeWordsCheckBox;
};

// ====================================================================================================

class LayoutDirectionComboBox : public QComboBox
{
    Q_OBJECT
    Q_DISABLE_COPY(LayoutDirectionComboBox)

public:
    explicit LayoutDirectionComboBox(QWidget *parent = Q_NULLPTR);

protected:
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void updateState();
    void updateStateLater();
    void onCurrentIndexChanged(int index);

private:
    bool m_updatePending;
};

// ====================================================================================================

} // namespace StylesheetEditorImpl

#endif // QTUTILS_STYLESHEET_EDITOR_P_H_INCLUDED

