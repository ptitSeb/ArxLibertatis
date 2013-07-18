#ifndef ARXPROFILER_H
#define ARXPROFILER_H

#include <QtGui/QMainWindow>
#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsRectItem>

#include "ui_arxprofiler.h"

class ProfilerView : public QGraphicsView {
	Q_OBJECT

public:
    ProfilerView(QWidget* parent = NULL);
 
    QGraphicsScene* scene;

protected:
    //Holds the current centerpoint for the view, used for panning and zooming
    QPointF CurrentCenterPoint;
 
    //From panning the view
    QPoint LastPanPoint;
 
    //Set the current centerpoint in the
    void SetCenter(const QPointF& centerPoint);
    QPointF GetCenter() { return CurrentCenterPoint; }
 
    //Take over the interaction
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void wheelEvent(QWheelEvent* event);
    virtual void resizeEvent(QResizeEvent* event);
	virtual void keyPressEvent(QKeyEvent* event);

private:
	void zoomEvent(QPoint mousePos, bool zoomIn);
};

class ArxProfiler : public QMainWindow {
	Q_OBJECT

public:
	ArxProfiler(QWidget *parent = 0, Qt::WFlags flags = 0);
	~ArxProfiler();

private slots:
    void openFile();

private:
	Ui::ArxProfilerClass ui;
    ProfilerView* view;
};



#endif // ARXPROFILER_H
