#include "displaydmusregion.h"
#include "ui_displaydmusregion.h"

DisplayDMUsRegion::DisplayDMUsRegion(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::DisplayDMUsRegion)
{
	ui->setupUi(this);
}

DisplayDMUsRegion::~DisplayDMUsRegion()
{
	delete ui;
}
