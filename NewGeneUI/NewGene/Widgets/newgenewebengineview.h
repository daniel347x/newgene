#ifndef NEWGENEWEBENGINEVIEW
#define NEWGENEWEBENGINEVIEW

#include <QWebEngineView>
#include <QDesktopServices>
#include <vector>

class NewGeneWebEngineView : public QWebEngineView
{
	Q_OBJECT
public:
	NewGeneWebEngineView(QWidget * parent) : QWebEngineView(parent) {}
	virtual ~NewGeneWebEngineView()
	{
		for (auto webView : webViews)
		{
			//webView->deleteLater();
		}
	}
protected:
	QWebEngineView * createWindow(QWebEnginePage::WebWindowType type)
	{
		QWebEngineView * webView = new QWebEngineView();
		connect(webView, SIGNAL(urlChanged(const QUrl &)), this, SLOT(receiveLinkClicked(const QUrl &)));
		webViews.push_back(webView);
		return webView;
	}
	std::vector<QWebEngineView*> webViews;
private slots:
	void receiveLinkClicked(const QUrl & url)
	{
		QWebEngineView* webView = dynamic_cast<QWebEngineView*>(sender());
		if( webView != NULL )
		{
			std::vector<QWebEngineView*> tmpViews;
			bool found {false};
			for (auto webViewCreated : webViews)
			{
				if (webViewCreated == webView)
				{
					found = true;
				}
				else
				{
					tmpViews.push_back(webViewCreated);
				}
				//webView->deleteLater();
			}
			if (found)
			{
				// Only allow each web view to open a URL once
				webView->deleteLater();
				QDesktopServices::openUrl(url);
				webViews.swap(tmpViews);
			}
		}
	}
};

#endif // NEWGENEWEBENGINEVIEW
