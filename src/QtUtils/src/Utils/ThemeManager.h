/*
   Copyright (C) 2018-2020 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined (QTUTILS_THEMEMANAGER_H_INCLUDED)
#define QTUTILS_THEMEMANAGER_H_INCLUDED

#include <QObject>
#include <QString>
#include <QStringList>

#include "Global.h"
#include "ScopedPointer.h"

class QMenu;
class QAction;
class QComboBox;

/// @brief Менеджер по управлению темами.
class ThemeManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ThemeManager)

Q_SIGNALS:
    /// @brief Сигнал об изменении темы.
    /// @param themeId - Идентификатор новой темы.
    void themeChanged(const QString &themeId);

public:
    /// @brief Получить указатель на экземпляр текущего менеджера тем.
    /// @return Указатель на экземпляр текущего менеджера тем.
    static ThemeManager *instance();

    /// @brief Зарегистрировать тему.
    /// @param themeId - Уникальный идентификатор темы.
    /// @param styleSheets - Список путей до QSS файлов со стилями.
    /// @param translateContext - Контекст локализации. В этом контексте должна
    ///  быть строка с ключем themeId, содержащая локализованное название темы.
    /// @param isDefault - Тема по умолчанию. Тема по умолчанию должна быть одна.
    void registerTheme(const QString &themeId, const QStringList &styleSheets,
                       const QString &translateContext, const bool isDefault = false);

    /// @brief Применить текущую тему. Допускается использование только один раз
    /// при старте приложения, в противном случае результат не гарантируется.
    void applyCurrentTheme();

    /// @brief Получить идентификатор текущей темы.
    /// @return Идентификатор текущей темы.
    QString currentTheme() const;

    /// @brief Установить текущей тему с указанным идентификатором. Сама тема не
    /// применится, пока не будет вызван метод applyCurrentTheme().
    /// @param themeId - Идентификатор темы.
    /// @param showMessage - Показать ли сообщение что тема будет применена после
    ///  перезапуска программы.
    /// @param parent - Родитель для сообщения, показываемого при showMessage=true.
    void setTheme(const QString &themeId, const bool showMessage = false, QWidget *parent = Q_NULLPTR);

    /// @brief Заполнить меню элементами для выбора темы. Все необходимые
    ///  соединения будут установлены автоматически.
    /// @param menu - меню, которое должно быть заполнено.
    void fillMenu(QMenu *menu);

    /// @brief Заполнить комбобокс элементами для выбора темы. Все необходимые
    ///  соединения будут установлены автоматически.
    /// @param comboBox - комбобокс, который должен быть заполнен.
    /// @param autoApply - автоматически применять изменения при выборе темы.
    void fillComboBox(QComboBox *comboBox, const bool autoApply = true);

private:
    ThemeManager();
    ~ThemeManager();

private Q_SLOTS:
    void onActionTriggered(QAction *action);
    void onComboBoxActivated(int index);
    void onActionDestroyed(QObject *object);
    void onComboBoxDestroyed(QObject *object);

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // QTUTILS_THEMEMANAGER_H_INCLUDED

