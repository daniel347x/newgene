#ifndef QSORTFILTERPROXYMODEL_NUMBERSLAST_H
#define QSORTFILTERPROXYMODEL_NUMBERSLAST_H

#include <QSortFilterProxyModel>

class QSortFilterProxyModel_NumbersLast : public QSortFilterProxyModel
{

	Q_OBJECT

	public:

		QSortFilterProxyModel_NumbersLast(QObject * parent = nullptr);

		bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

};

#endif // QSORTFILTERPROXYMODEL_NUMBERSLAST_H