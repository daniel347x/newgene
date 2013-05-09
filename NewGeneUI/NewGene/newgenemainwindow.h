#ifndef NEWGENEMAINWINDOW_H
#define NEWGENEMAINWINDOW_H

#include "globals.h"
#include <QMainWindow>
#include "newgenewidget.h"

#include <memory>

class UIProject;

namespace Ui {
class NewGeneMainWindow;
}

class NewGeneMainWindow : public QMainWindow, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{
	Q_OBJECT

public:
	explicit NewGeneMainWindow(QWidget *parent = 0);
	~NewGeneMainWindow();

protected:
	void changeEvent(QEvent *e);

private:
	Ui::NewGeneMainWindow *ui;

	std::unique_ptr<UIProject> project;

	friend class NewGeneWidget;
};

#endif // NEWGENEMAINWINDOW_H
