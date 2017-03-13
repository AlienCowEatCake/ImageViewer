/// @note This is experimental widget provides Movie wrapper dsplay.

#if !defined(QTIMAGEFORMATS_MOVIE_LABEL_H_INCLUDED)
#define QTIMAGEFORMATS_MOVIE_LABEL_H_INCLUDED

#include <QLabel>
#include <QScopedPointer>
#include "QtImageFormatsMovie.h"

QT_BEGIN_NAMESPACE

class QtImageFormatsMovieLabel : public QLabel
{
    Q_OBJECT

public:
    explicit QtImageFormatsMovieLabel(QWidget *parent = Q_NULLPTR, Qt::WindowFlags flags = Qt::WindowFlags());
    ~QtImageFormatsMovieLabel();

public Q_SLOTS:
    void setMovie(QtImageFormatsMovie *movie);
    void clear();

private Q_SLOTS:
    void movieUpdated();

private:
    Q_DISABLE_COPY(QtImageFormatsMovieLabel)
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

QT_END_NAMESPACE

#endif // QTIMAGEFORMATS_MOVIE_LABEL_H_INCLUDED
