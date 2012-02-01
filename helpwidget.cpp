#include "helpwidget.h"
#include "arpmanetdc.h"

HelpWidget::HelpWidget(ArpmanetDC *parent)
{
	//Constructor
	pParent = parent;

	createWidgets();
	placeWidgets();
	connectWidgets();
}

HelpWidget::~HelpWidget()
{
	//Destructor
}

void HelpWidget::createWidgets()
{
    pWidget = new QWidget();
}

void HelpWidget::placeWidgets()
{
    QLabel *iconLabel = new QLabel(pWidget);
    iconLabel->setPixmap(QPixmap(":/ArpmanetDC/Resources/Logo.png").scaled(100,100, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    QLabel *gplLabel = new QLabel(pWidget);
    gplLabel->setPixmap(QPixmap(":/ArpmanetDC/Resources/GPLv3 Logo 128px.png").scaled(48,20, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
   
    QHBoxLayout *aboutLayout = new QHBoxLayout();
    aboutLayout->addWidget(iconLabel);
    
    QLabel *aboutLabel = new QLabel(tr("<b><a href=\"http://code.google.com/p/arpmanetdc/\">ArpmanetDC</a></b><br/>Version %1 Alpha<br/>Copyright (C) 2012, Arpmanet Community<p>Free software licenced under <a href=\"http://www.gnu.org/licenses/\">GPLv3</a></p>").arg(VERSION_STRING), pWidget);
    aboutLabel->setOpenExternalLinks(true);

    QVBoxLayout *aboutVLayout = new QVBoxLayout();
    aboutVLayout->addStretch(1);
    aboutVLayout->addWidget(aboutLabel);
    aboutVLayout->addWidget(gplLabel);
    aboutVLayout->addStretch(1);

    aboutLayout->addLayout(aboutVLayout);
    aboutLayout->addStretch(1);

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addLayout(aboutLayout);

    QLabel *linkLabel = new QLabel("<p><font size=5><b>Q: \"Is x out yet?\"</b><br/>A1: <a href=\"http://arpmanet.ath.cx/phpBB2/index.php\">Check the BBS</a><br/>A2: Press the massive Search button at the top left and check</font></p>", pWidget);
    linkLabel->setOpenExternalLinks(true);
    
    vlayout->addWidget(linkLabel);
    vlayout->addWidget(new QLabel("<p><font size=5><b>Q: \"Does anyone have x?\"</b><br/>A: See above</font></p>", pWidget));    
    vlayout->addStretch(1);

	pWidget->setLayout(vlayout);
}

void HelpWidget::connectWidgets()
{

}

QWidget *HelpWidget::widget()
{
	return pWidget;
}