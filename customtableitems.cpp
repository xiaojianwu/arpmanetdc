#include "customtableitems.h"
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QPainter>

HTMLDelegate::HTMLDelegate(QTableView *tableView)
{
	int gridHint = tableView->style()->styleHint(QStyle::SH_Table_GridLineColor, new QStyleOptionViewItemV4());
	QColor gridColor = static_cast<QRgb>(gridHint);
	pGridPen = QPen(gridColor, 0, tableView->gridStyle());
}

//Determines the custom delegate's size
QSize HTMLDelegate::sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QStyleOptionViewItemV4 options = option;
    initStyleOption(&options, index);

    QTextDocument doc;
    doc.setHtml(options.text);
    doc.setTextWidth(options.rect.width());

	QSize iconSize = options.icon.actualSize(options.rect.size());
	int iconExtra = 10;

    return QSize(doc.idealWidth() + iconSize.width() + iconExtra, doc.size().height());
}

//Draws the HTML code
void HTMLDelegate::paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex &index) const
{
    QStyleOptionViewItemV4 options = option;
    initStyleOption(&options, index);

    painter->save();

	//Draw grid lines
	QPen oldPen = painter->pen();
	painter->setPen(pGridPen);
	//painter->drawLine(option.rect.topRight(), option.rect.bottomRight()); //vertical line
	//painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight()); //horizontal line
	painter->setPen(oldPen);

    QTextDocument doc;
    doc.setHtml(options.text);

    options.text = "";
    options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter);

    // shift text right to make icon visible
    QSize iconSize = options.icon.actualSize(options.rect.size());
	int iconExtra = 10;
    painter->translate(options.rect.left()+ iconSize.width() + iconExtra, options.rect.top());
    QRect clip(0, 0, options.rect.width()+ iconSize.width() + iconExtra, options.rect.height());

    //doc.drawContents(painter, clip);

    painter->setClipRect(clip);
    QAbstractTextDocumentLayout::PaintContext ctx;
    // set text color to red for selected item
    if (option.state & QStyle::State_Selected)
        ctx.palette.setColor(QPalette::Text, QColor("white"));
    ctx.clip = clip;
    doc.documentLayout()->draw(painter, ctx);

	

    painter->restore();
}

CTabWidget::CTabWidget(QWidget *parent) : QTabWidget(parent)
{

}