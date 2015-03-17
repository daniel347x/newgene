#include "splash.h"
#include "ui_splash.h"
#include <QCloseEvent>
#include <QDesktopServices>
#include <QMovie>

Splash::Splash(QWidget * parent, NewGeneMainWindow * mainWindow_, bool const opened_as_about_box_) :
	QWidget{parent},
	mainWindow{mainWindow_},
	closed_via_click{false},
	opened_as_about_box{opened_as_about_box_},
	ui{new Ui::Splash}
{
	ui->setupUi(this);
	QTimer::singleShot(10, mainWindow, SLOT(doDisable()));
	ui->webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
	ui->webView->setVisible(false);
	QMovie * movie = new QMovie(":/spinner.gif");
	ui->label->setMovie(movie);
	movie->start();
	connect(ui->webView, SIGNAL(linkClicked(const QUrl &)), this, SLOT(receiveLinkClicked(const QUrl &)));
	connect(ui->webView, SIGNAL(loadFinished(bool)), this, SLOT(receiveLoadFinished(bool)));
}

Splash::~Splash()
{
	delete ui;
}

void Splash::receiveLinkClicked(const QUrl & url)
{
	QDesktopServices::openUrl(url);
	closeMyself();
}

void Splash::receiveLoadFinished(bool)
{
	ui->label->setVisible(false);
	ui->webView->setVisible(true);
}

bool Splash::eventFilter(QObject * obj, QEvent * event)
{
	bool ret = QWidget::eventFilter(obj, event);
	return ret;
}

void Splash::closeEvent(QCloseEvent * event)
{
	if (!closed_via_click)
	{
		closeAndRefreshSequence();
	}

	event->accept();
}

void Splash::closeAndRefreshSequence()
{
	// Deal with visual artifacts - I'm not sure why the window needs to be repainted,
	// but repainting to remove artifacts after the splash screen has been removed
	// only works after the splash screen has been deleted, and there is a safety delay
	// prior to deleting it.
	QTimer::singleShot(0, this, SLOT(deleteMe()));
	QTimer::singleShot(100, mainWindow, SLOT(doEnable()));
	QTimer::singleShot(1000, mainWindow, SLOT(update()));
}

void Splash::deleteMe()
{
	deleteLater();
}

void Splash::on_pushButton_clicked()
{
	closeMyself();
}

void Splash::closeMyself()
{
	QTimer::singleShot(10, this, SLOT(close()));
	QTimer::singleShot(1000, this, SLOT(closeAndRefreshSequence()));
}
