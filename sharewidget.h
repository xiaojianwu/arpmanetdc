#ifndef SHAREWIDGET_H
#define SHAREWIDGET_H

#include <QtGui>
#include "checkableproxymodel.h"
#include "sharesearch.h"

class ArpmanetDC;

//Class encapsulating all widgets/signals for search tab
class ShareWidget : public QObject
{
	Q_OBJECT

public:
	ShareWidget(ShareSearch *share, ArpmanetDC *parent);
	~ShareWidget();

	//Get the encapsulating widget
	QWidget *widget();


private slots:
	//Slots
	void selectedItemsChanged();

	void saveSharePressed();

	void changeRoot(QString path);

signals:
	//Signals
	void saveButtonPressed();

private:
	//Functions
	void createWidgets();
	void placeWidgets();
	void connectWidgets();

	//Objects
	QWidget *pWidget;
	ArpmanetDC *pParent;
	ShareSearch *pShare;

	//Params
	QList<QString> pSharesList;

	//GUI elements
	QTreeView *fileTree;
	QFileSystemModel *fileModel;
	CheckableProxyModel *checkProxyModel;

	QPushButton *saveButton;

};

#endif