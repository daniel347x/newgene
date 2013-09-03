#ifndef NEWGENEGENERATEOUTPUT_H
#define NEWGENEGENERATEOUTPUT_H

#include <QWidget>

namespace Ui {
	class NewGeneGenerateOutput;
}

class NewGeneGenerateOutput : public QWidget
{
		Q_OBJECT
		
	public:
		explicit NewGeneGenerateOutput(QWidget *parent = 0);
		~NewGeneGenerateOutput();
		
	private:
		Ui::NewGeneGenerateOutput *ui;
};

#endif // NEWGENEGENERATEOUTPUT_H
