#include "arxprofiler.h"

//Qt includes
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QTextStream>
#include <QScrollBar>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>
#include <QFileDialog>
#include <QGraphicsSimpleTextItem>
#include <QHash>

ArxProfiler::ArxProfiler(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags) {
	ui.setupUi(this);

    view = new ProfilerView(this);
	setCentralWidget(view);
    
    connect(ui.action_Open, SIGNAL(triggered()), this, SLOT(openFile()));
}

ArxProfiler::~ArxProfiler() {
}

struct ProfilePoint {
    QString tag;
    unsigned int threadId;
    quint64 startTime;
    quint64 endTime;
};

struct ThreadInfo {
	QString      threadName;
	unsigned int threadId;
	quint64      startTime;
	quint64      endTime;
};


class QGraphicsProfilePoint : public QGraphicsRectItem
{
	static const int Type = UserType + 1;
	int type() const
    {
        // Enable the use of qgraphicsitem_cast with this item.
        return Type;
    }

public:
	QGraphicsProfilePoint(QGraphicsItem *parent = 0, QGraphicsScene *scene = 0) : QGraphicsRectItem(parent, scene)
	{
	}
    
	QGraphicsProfilePoint(const QRectF &rect, QGraphicsItem *parent = 0, QGraphicsScene *scene = 0) : QGraphicsRectItem(rect, parent, scene)
	{
	}
    
	QGraphicsProfilePoint(qreal x, qreal y, qreal w, qreal h, QGraphicsItem *parent = 0, QGraphicsScene *scene = 0) : QGraphicsRectItem(x, y, w, h, parent, scene)
	{
	}
    
	~QGraphicsProfilePoint()
	{
	}

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0)
	{
		QGraphicsRectItem::paint(painter, option, widget);

		if(rect().width() * painter->transform().m11() >= 5)
		{
			QTransform transBefore = painter->transform();
			painter->setTransform(QTransform());

			QRectF newRect = transBefore.mapRect(rect());
			newRect.adjust(4, 1, 0, 0);
			painter->drawText(newRect, m_Text);

			painter->setTransform(transBefore);
		}
	}

	void setText(const QString& text)
	{
		m_Text = text;
	}

private:
	QString m_Text;
    Q_DISABLE_COPY(QGraphicsProfilePoint)
};

class QGraphicsHud : public QGraphicsItem {
public:
	QGraphicsHud() : QGraphicsItem(0, 0) {
	}

protected:
	QRectF boundingRect(void) const {
		return scene()->views().at(0)->rect();
	}

	void QGraphicsItem::paint(QPainter *,const QStyleOptionGraphicsItem *,QWidget *) {
	}

	QVariant itemChange ( GraphicsItemChange change, const QVariant & value ) {
		return QGraphicsItem::itemChange(change, value);
	}
};

void ArxProfiler::openFile() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Ax performance log (*.perf)"));

    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly))
         return;

	struct ThreadData {
		ThreadData() {
			info.threadId = 0;
			info.startTime = 0;
			info.endTime = 0;

			maxDepth = 0;
			group = 0;
		}

		ThreadInfo                  info;
		std::vector<qint64>         currentStack;
		std::vector<ProfilePoint>   profilePoints;
		quint32                     maxDepth;

		QGraphicsItemGroup*         group;
	};

	typedef std::map<unsigned int, ThreadData> ThreadsData;
	ThreadsData threadsData;

	quint32 numItems;
	file.read((char*)&numItems, sizeof(numItems));
	for(quint32 i = 0; i < numItems; i++) 	{
		quint32 len;
		file.read((char*)&len, sizeof(len));
		
		char* threadName = (char*)alloca(len+1);
		file.read(threadName, len);
		threadName[len] = 0;

		quint32 threadId;
		file.read((char*)&threadId, sizeof(threadId));

		ThreadInfo& threadInfo = threadsData[threadId].info;
		threadInfo.threadId = threadId;
		threadInfo.threadName = threadName;

		file.read((char*)&threadInfo.startTime, sizeof(threadInfo.startTime));
		file.read((char*)&threadInfo.endTime, sizeof(threadInfo.endTime));
	}

	file.read((char*)&numItems, sizeof(numItems));
		
	QByteArray profilePointData = file.read(file.size());
	const char* data = profilePointData.constData();

    for(quint32 i = 0; i < numItems; i++) {
        unsigned int len = *(unsigned int*)data;
		data += sizeof(len);

		// TODO: String table ftw
		ProfilePoint point;
		point.tag = QString::fromAscii(data, len);
        data += len;
		
        point.threadId = *(unsigned int*)data;
		data += sizeof(point.threadId);

        point.startTime = *(qint64*)data;
		data += sizeof(point.startTime);

        point.endTime = *(qint64*)data;
		data += sizeof(point.endTime);
        
		threadsData[point.threadId].profilePoints.push_back(point);
	}

	quint64 startTime = 0xFFFFFFFFFFFFFFFF;
	quint64 endTime = 0;
	for(ThreadsData::const_iterator it = threadsData.begin(); it != threadsData.end(); ++it) {

		if(it->second.profilePoints.empty()) {
			continue;
		}

		if(startTime > it->second.profilePoints[0].startTime) {
			startTime = it->second.profilePoints[0].startTime;
		}

		if(endTime < it->second.profilePoints.back().endTime) {
			endTime = it->second.profilePoints.back().endTime;
		}
	}

	const qint32 NUM_UNITS = 6;
	const char*  UNIT_NAME[NUM_UNITS] = { "us", "ms", "s", "m", "h", "d" };
	const qint64 UNIT_NEXT[NUM_UNITS-1] = { 1000, 1000,  60,  60,  24  };	
	const quint32 ITEM_HEIGHT = 15;
	const quint32 THREAD_SPACING = 50;

	// reverse iterate
	int threadIdx = 0;
	int nextPos = 0;

	QGraphicsHud* hud = new QGraphicsHud();
	hud->setPos(0, 0);
	view->scene->addItem(hud);
	
	for(ThreadsData::iterator it = threadsData.begin(); it != threadsData.end(); ++it, threadIdx++) {
		ThreadData& threadData = it->second;

		//threadData.group = new QGraphicsItemGroup();
		//threadData.group->setPos(0, nextPos);

		QGraphicsSimpleTextItem* threadLabel = new QGraphicsSimpleTextItem(threadData.info.threadName);
		threadLabel->setFlag(QGraphicsItem::ItemIgnoresTransformations);
		threadLabel->setParentItem(threadData.group);

		for(std::vector<ProfilePoint>::const_reverse_iterator it = threadData.profilePoints.rbegin(); it != threadData.profilePoints.rend(); ++it) {

			std::vector<qint64>& threadStack = threadData.currentStack;
			while(!threadStack.empty()) {
				if(it->endTime <= threadStack.back()) {
					threadStack.pop_back();
				} else {
					break;
				}
			}
			qreal offset = threadIdx * 200 + ITEM_HEIGHT * threadStack.size();

			threadData.currentStack.push_back(it->startTime);
			if(threadData.currentStack.size() > threadData.maxDepth)
				threadData.maxDepth = threadData.currentStack.size();

			double duration = it->endTime - it->startTime;
			int unitIdx = 0;
			const char* unitName = UNIT_NAME[unitIdx];
			while(duration > UNIT_NEXT[unitIdx]) {
				duration /= UNIT_NEXT[unitIdx];
			
				unitIdx++;
				unitName = UNIT_NAME[unitIdx];

				if(unitIdx == NUM_UNITS)
					break;
			}

			// Round to get 2 decimals of precision
			duration = (int)(duration * 100);
			duration /= 100;

			QGraphicsProfilePoint* profilePoint = new QGraphicsProfilePoint(it->startTime - startTime, offset, it->endTime - it->startTime, ITEM_HEIGHT);

			QString text = QString("%3 (%1 %2)").arg(duration).arg(unitName).arg(it->tag);
			profilePoint->setText(text);
			profilePoint->setToolTip(text);

			qsrand(qHash(it->tag));
			QColor tagColor;
			tagColor.setRed(100 + qrand() % 155);
			tagColor.setGreen(100 + qrand() % 155);
			tagColor.setBlue(100 + qrand() % 155);
			profilePoint->setBrush(QBrush(tagColor));
			
			//profilePoint->setParentItem(threadData.group);
			view->scene->addItem(profilePoint);
		}

		//view->scene->addItem(threadData.group);

		nextPos += threadData.maxDepth * ITEM_HEIGHT + THREAD_SPACING;
	}
}

/////////////////

ProfilerView::ProfilerView(QWidget* parent) : QGraphicsView(parent) {

	setBackgroundBrush(QBrush(QColor(160, 160, 160)));

    //Set-up the scene
    scene = new QGraphicsScene(this);
    setScene(scene);

	setRenderHint(QPainter::HighQualityAntialiasing, true);
	
	//setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	setOptimizationFlags(QGraphicsView::DontSavePainterState);
	//setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	//setDragMode(QGraphicsView::RubberBandDrag);

    //Set-up the view
    SetCenter(QPointF(500.0, 500.0)); //A modified version of centerOn(), handles special cases
}
 
/**
  * Sets the current centerpoint.  Also updates the scene's center point.
  * Unlike centerOn, which has no way of getting the floating point center
  * back, SetCenter() stores the center point.  It also handles the special
  * sidebar case.  This function will claim the centerPoint to sceneRec ie.
  * the centerPoint must be within the sceneRec.
  */
//Set the current centerpoint in the
void ProfilerView::SetCenter(const QPointF& centerPoint) {
    //Get the rectangle of the visible area in scene coords
    QRectF visibleArea = mapToScene(rect()).boundingRect();
 
    //Get the scene area
    QRectF sceneBounds = sceneRect();
 
    double boundX = visibleArea.width() / 2.0;
    double boundY = visibleArea.height() / 2.0;
    double boundWidth = sceneBounds.width() - 2.0 * boundX;
    double boundHeight = sceneBounds.height() - 2.0 * boundY;
 
    //The max boundary that the centerPoint can be to
    QRectF bounds(boundX, boundY, boundWidth, boundHeight);
 
    if(bounds.contains(centerPoint)) {
        //We are within the bounds
        CurrentCenterPoint = centerPoint;
    } else {
        //We need to clamp or use the center of the screen
        if(visibleArea.contains(sceneBounds)) {
            //Use the center of scene ie. we can see the whole scene
            CurrentCenterPoint = sceneBounds.center();
        } else {
 
            CurrentCenterPoint = centerPoint;
 
            //We need to clamp the center. The centerPoint is too large
            if(centerPoint.x() > bounds.x() + bounds.width()) {
                CurrentCenterPoint.setX(bounds.x() + bounds.width());
            } else if(centerPoint.x() < bounds.x()) {
                CurrentCenterPoint.setX(bounds.x());
            }
 
            if(centerPoint.y() > bounds.y() + bounds.height()) {
                CurrentCenterPoint.setY(bounds.y() + bounds.height());
            } else if(centerPoint.y() < bounds.y()) {
                CurrentCenterPoint.setY(bounds.y());
            }
 
        }
    }
 
    //Update the scrollbars
    centerOn(CurrentCenterPoint);
}
 
/**
  * Handles when the mouse button is pressed
  */
void ProfilerView::mousePressEvent(QMouseEvent* event) {
	//QGraphicsView::mousePressEvent(event);
    //For panning the view
    LastPanPoint = event->pos();
    setCursor(Qt::ClosedHandCursor);
}
 
/**
  * Handles when the mouse button is released
  */
void ProfilerView::mouseReleaseEvent(QMouseEvent* event) {
	//QGraphicsView::mouseReleaseEvent(event);
	setCursor(Qt::ArrowCursor);
    LastPanPoint = QPoint();
}
 
/**
*Handles the mouse move event
*/
void ProfilerView::mouseMoveEvent(QMouseEvent* event) {
	//QGraphicsView::mouseMoveEvent(event);
    if(!LastPanPoint.isNull()) {
        //Get how much we panned
        QPointF delta = mapToScene(LastPanPoint) - mapToScene(event->pos());
        LastPanPoint = event->pos();
 
        //Update the center ie. do the pan
        SetCenter(GetCenter() + delta);
    }
}
 
void ProfilerView::zoomEvent(QPoint mousePos, bool zoomIn) {
	//Get the position of the mouse before scaling, in scene coords
    QPointF pointBeforeScale(mapToScene(mousePos));
 
    //Get the original screen centerpoint
    QPointF screenCenter = GetCenter(); //CurrentCenterPoint; //(visRect.center());
 
    //Scale the view ie. do the zoom
    double scaleFactor = 1.15; //How fast we zoom
    if(zoomIn) {
        //Zoom in
        scale(scaleFactor, 1.0f);
    } else {
        //Zooming out
        scale(1.0 / scaleFactor, 1.0f);
    }
 
    //Get the position after scaling, in scene coords
    QPointF pointAfterScale(mapToScene(mousePos));
 
    //Get the offset of how the screen moved
    QPointF offset = pointBeforeScale - pointAfterScale;
 
    //Adjust to the new center for correct zooming
    QPointF newCenter = screenCenter + offset;
    SetCenter(newCenter);
}
/**
  * Zoom the view in and out.
  */
void ProfilerView::wheelEvent(QWheelEvent* event) {
 	zoomEvent(event->pos(), event->delta() > 0);
}

void ProfilerView::keyPressEvent(QKeyEvent* event)
{
	if(!underMouse())
		return;

	if(event->key() == Qt::Key_Plus) {
		zoomEvent(QCursor::pos(), true);
	} else if(event->key() == Qt::Key_Minus) {
		zoomEvent(QCursor::pos(), false);
	}
}
 
/**
  * Need to update the center so there is no jolt in the
  * interaction after resizing the widget.
  */
void ProfilerView::resizeEvent(QResizeEvent* event) {
    //Get the rectangle of the visible area in scene coords
    QRectF visibleArea = mapToScene(rect()).boundingRect();
    SetCenter(visibleArea.center());
 
    //Call the subclass resize so the scrollbars are updated correctly
    QGraphicsView::resizeEvent(event);
}

