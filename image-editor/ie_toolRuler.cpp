#include "ie_toolRuler.h"

IERuler::IERuler(_global_ie *gi, QObject *parent):IE_Tool_Line(gi, parent, ToolType::Ruler, LineSettings::None)
{
    selectionOffset = 10;
}

void IERuler::wheelMode(QWheelEvent *pe)
{

}

void IERuler::paint(QPainter *painter,
                    const QStyleOptionGraphicsItem *option,
                    QWidget *widget)
{
    IE_Tool_Line::paint(painter, option, widget);
    QPen locPen = painter->pen();
    locPen.setWidth(1);
    painter->setPen(locPen);
    qreal radAngle = line().angle()* M_PI / 180;
    qreal dx = selectionOffset * sin(radAngle);
    qreal dy = selectionOffset * cos(radAngle);
    QPointF offset1 = QPointF(dx, dy);
    QPointF offset2 = QPointF(-dx, -dy);

    painter->drawLine(line().p1() + offset1, line().p1() + offset2);
    painter->drawLine(line().p2() + offset2, line().p2() + offset1);
    paintLength(painter, option, widget);
    widget->update();
}

QRectF IERuler::boundingRect() const
{
    qreal radAngle = line().angle()* M_PI / 180;
    qreal dx = selectionOffset * sin(radAngle);
    qreal dy = selectionOffset * cos(radAngle);
    QPointF offset1 = QPointF(dx, dy);
    QPointF offset2 = QPointF(-dx, -dy);
    QPolygonF polf;
    polf << line().p1() + offset1
                 << line().p1() + offset2
                 << line().p2() + offset2
                 << line().p2() + offset1;
    return polf.boundingRect();

}

int IERuler::read(const QJsonObject &json)
{
    return IE_Tool_Line::read(json);
}

int IERuler::write(QJsonObject &json) const
{
    IE_Tool_Line::write(json);
    json["typeTitle"] = getToolTitle(ToolType::Ruler);
    return 0;
}


