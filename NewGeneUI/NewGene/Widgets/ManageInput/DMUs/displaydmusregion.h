#ifndef DISPLAYDMUSREGION_H
#define DISPLAYDMUSREGION_H

#include <QWidget>

namespace Ui {
	class DisplayDMUsRegion;
}

class DisplayDMUsRegion : public QWidget
{
		Q_OBJECT

	public:
		explicit DisplayDMUsRegion(QWidget *parent = 0);
		~DisplayDMUsRegion();

	private:
		Ui::DisplayDMUsRegion *ui;
};

#endif // DISPLAYDMUSREGION_H
