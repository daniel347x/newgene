#ifndef HTMLDELEGATE
#define HTMLDELEGATE

#include <QStyledItemDelegate>

// From http://stackoverflow.com/a/2039745/368896

class HtmlDelegate : public QStyledItemDelegate
{
protected:
    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};

#endif // HTMLDELEGATE
