#include "dtabbar.h"
#include <QtCore>
#include <QDebug>
#include <QPropertyAnimation>

TabBar::TabBar(QWidget *parent):QGraphicsView(parent){
    m_scene = new QGraphicsScene(this);
    setObjectName("TabBar");
    m_scene->setSceneRect(0,0,width(),height());
    QPainterPath path;
    path.addRect(m_scene->sceneRect());
    m_scene->setSelectionArea(path,QTransform());
    setContentsMargins(0,0,0,0);
    setScene(m_scene);
    hide();

    m_TabCloseButton = new TabCloseButton;
    m_TabCloseButton->setZValue(4);
    m_TabCloseButton->hide();
    m_scene->addItem(m_TabCloseButton);
    setMouseTracking(true);

    initConnections();
}

int TabBar::addTabWithData(const int &viewIndex, const QString text, const QString url)
{
    Tab *tab = new Tab(0,viewIndex,text);
    m_tabs.append(tab);
    int index = count() - 1;
    tab->setTabIndex(index);
    QJsonObject tabData;
    tabData["viewIndex"] = viewIndex;
    tabData["text"] = text;
    tabData["url"] = url;
    tab->setTabData(QVariant(tabData));

    connect(tab, &Tab::clicked, this, [=]{
       setCurrentIndex(tab->tabIndex());
    });

    connect(tab, &Tab::moveNext, this, &TabBar::onMoveNext);
    connect(tab, &Tab::movePrevius, this, &TabBar::onMovePrevius);
    connect(tab, &Tab::requestNewWindow, this, &TabBar::onRequestNewWindow);
    connect(tab, &Tab::draggingFinished, this, [=]{
        updateScreen();
        m_TabCloseButton->setZValue(2);
        if(tab->isDragOutSide())
            m_TabCloseButton->hide();
    });
    connect(tab, &Tab::draggingStarted, this, [=]{
       m_TabCloseButton->setZValue(0);
    });
    m_scene->addItem(tab);

    setCurrentIndex(index);

    if(count()>=2)
        show();

    tabAddableChanged(count()<8);

    return index;
}

QVariant TabBar::tabData(const int index)
{
    if(index>=count()||index<0)
        return QVariant();
    return m_tabs.at(index)->tabData();
}

int TabBar::count() const
{
    return m_tabs.count();
}

void TabBar::removeTab(const int index)
{
    Tab *tab = m_tabs.at(index);
    int closingIndex = m_TabCloseButton->closingIndex();
    m_scene->removeItem(tab);
    m_tabs.removeAt(m_TabCloseButton->closingIndex());

    for(int i = closingIndex; i<count(); i++){
        Tab *tab = m_tabs.at(i);
        tab->setTabIndex(tab->tabIndex() - 1);
    }
    if(m_TabCloseButton->closingIndex() != count() - 1){
        m_lastDeleteState = true;
    }
    updateScreen();
}

QRect TabBar::tabRect(const int index) const
{
    if(index>=count()||index<0)
        return QRect();
    return m_tabs.at(index)->boundingRect().toRect();
}

int TabBar::currentIndex() const
{
    return m_currentIndex;
}

void TabBar::setCurrentIndex(const int index)
{
    m_currentIndex = index;

    int counter = 0;
    for(auto tab: m_tabs){
        if(counter == index){
            tab->setChecked(true);
            tab->update();
        }
        else{
            tab->setChecked(false);
            tab->update();
        }
        counter ++;
    }
    currentChanged(index);
    updateScreen();
}

bool TabBar::tabAddable()
{
    return count()<8;
}

void TabBar::setTabData(const int &index, const QVariant &tabData)
{
    if(index<0||index>=count())
        return;
    m_tabs.at(index)->setTabData(tabData);
}

void TabBar::setTabText(const int viewIndex, const QString text, const QString url)
{
    int counter = 0;
    for(auto it:m_tabs){
        if(it->tabData().toJsonObject()["viewIndex"].toInt() == viewIndex){
            it->setTabText(text);
            QJsonObject tabData = it->tabData().toJsonObject();
            tabData["text"] = text;
            tabData["url"] = url;
            it->setTabData(QVariant(tabData));
            it->update();
        }
        counter++;
    }
}

QSize TabBar::tabSizeHint(const int &index)
{

    if(m_lastDeleteState)
        return QSize(m_tabs.at(0)->width(),m_tabs.at(0)->height());

    int averageWidth = width()/count();

    qDebug()<<averageWidth<<size()<<","<<count();
    if(index == count() -1)
        return (QSize(width() - averageWidth*(count()-1),24));
    else
        return (QSize(averageWidth,24));
}

void TabBar::updateScreen()
{
    int counter = 0;
    int lastX = 0;
    for(auto tab:m_tabs){
        QRect rect(lastX,0,tabSizeHint(counter).width(),tabSizeHint(counter).height());
        qDebug()<<rect;
        lastX = rect.x() + rect.width();
        if(tab->isDragging()){
            counter ++ ;
            continue;
        }
        tab->setGeometry(rect);
        counter ++;
    }
}

void TabBar::initConnections()
{
    connect(m_TabCloseButton, &TabCloseButton::hovered, this, &TabBar::onTabCloseButtonHovered);
    connect(m_TabCloseButton, &TabCloseButton::unHovered, this, &TabBar::onTabCloseButtonUnHovered);
    connect(m_TabCloseButton, &TabCloseButton::clicked, this, [=]{
        emit tabCloseRequested(m_TabCloseButton->closingIndex());
    });
}

void TabBar::onMoveNext(const int fromTabIndex)
{
    Tab *fromTab = m_tabs.at(fromTabIndex);
    Tab *toTab = m_tabs.at(fromTabIndex+1);
    m_tabs.swap(fromTabIndex,fromTabIndex+1);
    fromTab->setTabIndex(fromTabIndex +1);
    toTab->setTabIndex(fromTabIndex);
    setCurrentIndex(fromTabIndex+1);
}

void TabBar::onMovePrevius(const int fromTabIndex)
{
    Tab *fromTab = m_tabs.at(fromTabIndex);
    Tab *toTab = m_tabs.at(fromTabIndex-1);
    m_tabs.swap(fromTabIndex,fromTabIndex-1);
    fromTab->setTabIndex(fromTabIndex -1);
    toTab->setTabIndex(fromTabIndex);
    setCurrentIndex(fromTabIndex-1);
}

void TabBar::onRequestNewWindow(const int tabIndex)
{
    DUrl url = DUrl::fromUserInput(tabData(tabIndex).toJsonObject()["url"].toString());
    FMEvent event;
    event = FMEvent::FileView;
    event = window()->winId();
    event = url;
    emit tabCloseRequested(tabIndex);
    appController->actionNewWindow(event);
}

void TabBar::onTabCloseButtonUnHovered(int closingIndex)
{
    if(closingIndex<0 || closingIndex>= count())
        return;
    Tab *tab = m_tabs.at(closingIndex);
    tab->setHovered(false);
    tab->update();
}

void TabBar::onTabCloseButtonHovered(int closingIndex)
{
    Tab *tab = m_tabs.at(closingIndex);
    tab->setHovered(true);
    tab->update();
}
void TabBar::resizeEvent(QResizeEvent *event)
{
    m_scene->setSceneRect(0,0,width(),height() );
    QPainterPath path;
    path.addRect(m_scene->sceneRect());
    m_scene->setSelectionArea(path,QTransform());
    updateScreen();
    QGraphicsView::resizeEvent(event);
}

bool TabBar::event(QEvent *event)
{
    if(event->type() == event->Leave){

        m_TabCloseButton->hide();
        m_lastDeleteState = false;
        updateScreen();
    }
    QGraphicsView::event(event);
}

void TabBar::mouseMoveEvent(QMouseEvent *event)
{
    if(!m_TabCloseButton->isVisible())
        m_TabCloseButton->show();
    int closingIndex = 0;
    closingIndex = event->pos().x()/m_tabs.at(0)->width();

    int counter = 0;
    for(auto it:m_tabs){
        if(counter != closingIndex){
            it->setHovered(false);
            it->update();
        }
        counter ++;
    }
    if(closingIndex<count() && closingIndex>=0){
        Tab *tab = m_tabs.at(closingIndex);
        m_TabCloseButton->setClosingIndex(closingIndex);
        m_TabCloseButton->setX(tab->x()+tab->width()-24);
    }
    else{
        if(m_lastDeleteState){
            m_lastDeleteState = false;
            updateScreen();
        }
    }

    QGraphicsView::mouseMoveEvent(event);
}

TabCloseButton::TabCloseButton(QGraphicsItem *parent):
    QGraphicsObject(parent)
{
    setFlag(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);
}

QRectF TabCloseButton::boundingRect() const
{
    return QRectF(0,0,23,23);

}

void TabCloseButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if(m_mousePressed){
        QPixmap pixmap(":/icons/images/icons/tab_close_press.png");
        painter->drawPixmap(boundingRect().toRect(),pixmap);
    }
    else if(m_mouseHovered){
        QPixmap pixmap(":/icons/images/icons/tab_close_hover.png");
        painter->drawPixmap(boundingRect().toRect(),pixmap);
    }
    else{
        QPixmap pixmap(":/icons/images/icons/tab_close_normal.png");
        painter->drawPixmap(boundingRect().toRect(),pixmap);
    }
}

int TabCloseButton::closingIndex()
{
    return m_closingIndex;
}

void TabCloseButton::setClosingIndex(int index)
{
    m_closingIndex = index;
}

void TabCloseButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_mousePressed = true;
    if(m_mouseHovered)
        m_mouseHovered = false;
    update();
}

void TabCloseButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug()<<event;
    m_mousePressed = false;
    emit clicked();
    update();
}

void TabCloseButton::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    event->ignore();
    if(!m_mouseHovered)
        m_mouseHovered = true;
    update();
}

void TabCloseButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    emit unHovered(closingIndex());
    event->ignore();
    m_mouseHovered = false;
    update();
}

void TabCloseButton::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{
    event->ignore();
    m_mouseHovered = true;
    emit hovered(closingIndex());
    update();
}

Tab::Tab(QGraphicsObject *parent, int viewIndex, QString text):
    QGraphicsObject(parent)
{
    m_tabText = text;
    QJsonObject tabData;
    tabData["viewIndex"] = viewIndex;
    tabData["text"] = text;
    m_tabData = tabData;
    setAcceptHoverEvents(true);
    setFlags(ItemIsSelectable);
}

void Tab::setTabIndex(int index)
{
    m_tabIndex = index;
}

int Tab::tabIndex()
{
    return m_tabIndex;
}

void Tab::setTabData(QVariant data)
{
    m_tabData = data;
}

QVariant Tab::tabData()
{
    return m_tabData;
}

void Tab::setTabText(QString text)
{
    m_tabText = text;

}

QString Tab::tabText()
{
    return m_tabText;
}

void Tab::setFixedSize(QSize size)
{
    m_width = size.width();
    m_height = size.height();
}

void Tab::setGeometry(QRect rect)
{

    qDebug()<<"geometry:"<<rect;
    setX(rect.x());
    setY(rect.y());
    m_width = rect.width();
    m_height = rect.height();
    update();
}

QRect Tab::geometry()
{
    return QRect(x(),y(),m_width,m_height);
}

int Tab::width()
{
    return m_width;
}

int Tab::height()
{
    return m_height;
}

bool Tab::isDragging()
{
    return m_isDragging;
}

bool Tab::isChecked()
{
    return m_checked;
}

void Tab::setChecked(bool check)
{
    m_checked = check;
}

void Tab::setHovered(bool hovered)
{
    m_hovered = hovered;
}

bool Tab::isDragOutSide()
{
    return m_dragOutSide;
}

QRectF Tab::boundingRect() const
{
    return QRectF(0,0,m_width-1,m_height-1);
}

void Tab::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen pen;
    QColor color(Qt::yellow);
    pen.setStyle(Qt::SolidLine);
    pen.setColor(color);
    painter->setPen(pen);
    pen.setWidth(1);

    //draw text
    QFont font;
    font.setPixelSize(14);
    QFontMetrics fm(font);
    QString str = fm.elidedText(m_tabText,Qt::ElideRight,m_width-10);

    //draw backgound

    if(m_checked){
        color.setNamedColor("#FFFFFF");
        painter->fillRect(boundingRect(),color);
        color.setNamedColor("#303030");
        pen.setColor(color);
        painter->setPen(pen);
        painter->drawText((m_width-fm.width(str))/2,(m_height-fm.height())/2,
                          fm.width(str),fm.height(),0,str);
    }
    else if(m_hovered||(m_hovered&&!m_checked)){
        color.setNamedColor("#EDEDED");
        painter->fillRect(boundingRect(),color);
        color.setNamedColor("#303030");
        pen.setColor(color);
        painter->setPen(pen);
        painter->drawText((m_width-fm.width(str))/2,(m_height-fm.height())/2,
                          fm.width(str),fm.height(),0,str);
    }
    else{
        color.setNamedColor("#FAFAFA");
        painter->fillRect(boundingRect(),color);
        color.setNamedColor("#949494");
        pen.setColor(color);
        painter->setPen(pen);
        painter->drawText((m_width-fm.width(str))/2,(m_height-fm.height())/2,
                          fm.width(str),fm.height(),0,str);
    }

    //draw line
    color.setRgb(0,0,0,33);
    pen.setColor(color);
    painter->setPen(pen);
    painter->drawLine(QPointF(0,boundingRect().height()),
                      QPointF(boundingRect().width(),boundingRect().height()));

    painter->drawLine(QPointF(boundingRect().width(),0),
                      QPointF(boundingRect().width(),boundingRect().height()-1));
}

QSizeF Tab::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_UNUSED(which)
    Q_UNUSED(constraint)

    return QSizeF(m_width,m_height);
}

void Tab::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        m_pressed = true;
        m_originPos = pos();
        setZValue(3);
//        m_isDragging = true;
    }
    QGraphicsObject::mousePressEvent(event);
}

void Tab::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug()<<event->pos();


    if(event->pos().y()<0 || event->pos().y() > m_height){
        if(!m_dragOutSide){
            m_dragOutSide = true;
            m_dragWidget = new DragWidget;
            m_dragWidget->show();
            m_dragWidget->setText(m_tabText);
            m_dragWidget->setFixedSize(m_width,m_height);
            m_dragWidget->move(QCursor::pos() - QPoint(m_width/2, m_height/2));
            m_cursor.setShape(Qt::DragMoveCursor);
        }
    }

    if(m_dragOutSide){
        if(m_dragWidget)
            m_dragWidget->move(QCursor::pos() - QPoint(m_width/2, m_height/2));
        QGraphicsObject::mouseMoveEvent(event);
        return;
    }

    if(pos().x() == 0 && pos().x() == scene()->width() - m_width){
        QGraphicsObject::mouseMoveEvent(event);
        return;
    }
    setPos(x() + event->pos().x() - event->lastPos().x(),0);
    draggingStarted();
    m_isDragging = true;
    if(pos().x()<0)
        setX(0);
    else if(pos().x()> scene()->width() - m_width)
        setX(scene()->width() - m_width);

    if(pos().x()> m_originPos.x()+m_width/2){
        emit moveNext(tabIndex());
        m_originPos.setX(m_originPos.x() + m_width);
    }
    else if(pos().x()< m_originPos.x() - m_width/2){
        emit movePrevius(tabIndex());
        m_originPos.setX(m_originPos.x() - m_width);
    }

    QGraphicsObject::mouseMoveEvent(event);
}

void Tab::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(m_dragOutSide){
        m_pressed = false;
        setZValue(1);
        QGraphicsObject::mouseReleaseEvent(event);
        emit requestNewWindow(tabIndex());
        emit draggingFinished();
        m_dragOutSide = false;
        m_isDragging = false;
        m_cursor.setShape(Qt::ArrowCursor);
        if(m_dragWidget){
            m_dragWidget->close();
            m_dragWidget = NULL;
        }
        return;
    }

    emit clicked();
    m_pressed = false;
    setZValue(1);
    m_isDragging = false;
    emit draggingFinished();
    QGraphicsObject::mouseReleaseEvent(event);
}

void Tab::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    qDebug()<<event;
    m_hovered = true;
    QGraphicsObject::hoverEnterEvent(event);
}

void Tab::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_hovered = false;
    m_pressed = false;
    QGraphicsObject::hoverLeaveEvent(event);
}


DragWidget::DragWidget(QWidget *parent):QPushButton(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::FramelessWindowHint);
}

void DragWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    QPen pen;
    QColor color;
    color.setRgb(255,255,255);
    pen.setColor(color);
    painter.setPen(pen);
    painter.fillRect(0,0,width(),height(),Qt::white);
//    painter.fillre

    QFont font;
    font.setPixelSize(14);
    QFontMetrics fm(font);
    QString str = fm.elidedText(text(),Qt::ElideRight,width()-10);

    color.setRgb(180,180,180);
    pen.setColor(color);
    painter.setPen(pen);

    painter.drawText((width() - fm.width(str))/2,(height() - fm.height())/2,fm.width(str),fm.height(),0,str);

    QWidget::paintEvent(e);
}
