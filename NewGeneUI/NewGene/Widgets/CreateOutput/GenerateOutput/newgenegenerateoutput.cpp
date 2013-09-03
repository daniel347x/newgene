#include "newgenegenerateoutput.h"
#include "ui_newgenegenerateoutput.h"

NewGeneGenerateOutput::NewGeneGenerateOutput(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::NewGeneGenerateOutput)
{
	ui->setupUi(this);
}

NewGeneGenerateOutput::~NewGeneGenerateOutput()
{
	delete ui;
}
