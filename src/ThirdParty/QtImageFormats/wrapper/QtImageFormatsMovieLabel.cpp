/// @note This is experimental widget provides Movie wrapper dsplay.

#include "QtImageFormatsMovieLabel.h"

QT_BEGIN_NAMESPACE

struct QtImageFormatsMovieLabel::Impl
{
    Impl(QtImageFormatsMovieLabel *frame)
        : frame(frame)
        , movie(Q_NULLPTR)
    {}

    QtImageFormatsMovieLabel *frame;
    QtImageFormatsMovie *movie;
};

QtImageFormatsMovieLabel::QtImageFormatsMovieLabel(QWidget *parent, Qt::WindowFlags flags)
    : QLabel(parent, flags)
    , m_impl(new Impl(this))
{}

QtImageFormatsMovieLabel::~QtImageFormatsMovieLabel()
{}

void QtImageFormatsMovieLabel::setMovie(QtImageFormatsMovie *movie)
{
    clear();
    if(!movie)
        return;

    m_impl->movie = movie;
    connect(m_impl->movie, SIGNAL(updated(QRect)), this, SLOT(movieUpdated()));
}

void QtImageFormatsMovieLabel::clear()
{
    if(m_impl->movie)
        disconnect(m_impl->movie, SIGNAL(updated(QRect)), this, SLOT(movieUpdated()));
    m_impl->movie = Q_NULLPTR;
    QLabel::clear();
}

void QtImageFormatsMovieLabel::movieUpdated()
{
    if(m_impl->movie && m_impl->movie->isValid())
        setPixmap(m_impl->movie->currentPixmap());
}

QT_END_NAMESPACE
