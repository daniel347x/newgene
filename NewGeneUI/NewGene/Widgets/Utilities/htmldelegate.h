#ifndef HTMLDELEGATE
#define HTMLDELEGATE

#include <QStyledItemDelegate>

// From http://stackoverflow.com/a/2039745/368896

class HtmlDelegate : public QStyledItemDelegate
{
	public:
		HtmlDelegate(bool const modifiedMargin = false) : modifiedMargin(modifiedMargin) {}
	protected:
		void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
		QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;

		bool const modifiedMargin;
};

#endif // HTMLDELEGATE
