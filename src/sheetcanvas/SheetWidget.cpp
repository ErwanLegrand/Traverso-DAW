/*
    Copyright (C) 2006-2007 Remon Sijrier 
 
    This file is part of Traverso
 
    Traverso is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.
 
*/

		
#include "SheetWidget.h"
#include "TrackPanelViewPort.h"
#include "ClipsViewPort.h"
#include "TimeLineViewPort.h"
#include "SheetView.h"
#include "Themer.h"
#include "Config.h"

#include <Sheet.h>
#include "Utils.h"
#include "ContextPointer.h"
#include "Mixer.h"

// #if defined (QT_OPENGL_SUPPORT)
// #include <QtOpenGL>
// #endif

#include <QGridLayout>
#include <QScrollBar>

#include <Debugger.h>

SheetPanelView::SheetPanelView(QGraphicsScene* scene, Sheet* sheet)
	: ViewItem(0, 0)
	, m_sheet(sheet)
{
	scene->addItem(this);
	m_boundingRect = QRectF(0, 0, 200, TIMELINE_HEIGHT);
}


void SheetPanelView::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
	int xstart = (int)option->exposedRect.x();
	int pixelcount = (int)option->exposedRect.width();
	
	QColor color = QColor(Qt::darkGray);//themer()->get_color("Track:cliptopoffset");
	painter->setPen(color);
	painter->fillRect(xstart, TIMELINE_HEIGHT - 2, pixelcount, 2, color);
	painter->fillRect(199, 0, 1, TIMELINE_HEIGHT, color);
}

class SheetPanelViewPort : public ViewPort
{
public:
	SheetPanelViewPort(QGraphicsScene* scene, SheetWidget* sw);
	~SheetPanelViewPort() {};

	void get_pointed_context_items(QList<ContextItem* > &list)
	{
		QList<QGraphicsItem *> itemlist = items(cpointer().on_first_input_event_x(), cpointer().on_first_input_event_y());
		foreach(QGraphicsItem* item, itemlist) {
			if (ViewItem::is_viewitem(item)) {
				list.append((ViewItem*)item);
			}
		}
		list.removeAll(m_spv);
		list.append(m_sv);
	}

        void set_sheet_view(SheetView* view) { m_sv = view;}

private:
        Sheet*          m_sheet;
        SheetView*      m_sv;
	SheetPanelView* m_spv;
};


SheetPanelViewPort::SheetPanelViewPort(QGraphicsScene * scene, SheetWidget * sw)
	: ViewPort(scene, sw)
{
	setSceneRect(-200, -TIMELINE_HEIGHT, 200, 0);
	setMaximumHeight(TIMELINE_HEIGHT);
	setMinimumHeight(TIMELINE_HEIGHT);
	setMinimumWidth(200);
	setMaximumWidth(200);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setBackgroundBrush(themer()->get_color("SheetPanel:background"));
}


SheetWidget::SheetWidget(Sheet* sheet, QWidget* parent)
	: QFrame(parent)
	, m_sheet(sheet)
{
	if (!m_sheet) {
		return;
	}
	m_scene = new QGraphicsScene();
	m_vScrollBar = new QScrollBar(this);
	m_hScrollBar = new QScrollBar(this);
	m_hScrollBar->setOrientation(Qt::Horizontal);

	m_trackPanel = new TrackPanelViewPort(m_scene, this);
	m_clipsViewPort = new ClipsViewPort(m_scene, this);
	m_timeLine = new TimeLineViewPort(m_scene, this);
	m_sheetPanelVP = new SheetPanelViewPort(m_scene, this);
	
	m_mainLayout = new QGridLayout(this);
	m_mainLayout->addWidget(m_sheetPanelVP, 0, 0);
	m_mainLayout->addWidget(m_timeLine, 0, 1);
	m_mainLayout->addWidget(m_trackPanel, 1, 0);
	m_mainLayout->addWidget(m_clipsViewPort, 1, 1);
	m_mainLayout->addWidget(m_hScrollBar, 2, 1);
	m_mainLayout->addWidget(m_vScrollBar, 1, 2);
	
	m_mainLayout->setMargin(0);
	m_mainLayout->setSpacing(0);

	setLayout(m_mainLayout);
	
	m_sv = new SheetView(this, m_clipsViewPort, m_trackPanel, m_timeLine, sheet);
	m_timeLine->set_sheetview(m_sv);
	m_sheetPanelVP->set_sheet_view(m_sv);
	
	connect(m_clipsViewPort->horizontalScrollBar(), 
		SIGNAL(valueChanged(int)),
		m_timeLine->horizontalScrollBar(), 
		SLOT(setValue(int)));
	
	connect(m_timeLine->horizontalScrollBar(), 
		SIGNAL(valueChanged(int)),
		m_clipsViewPort->horizontalScrollBar(), 
		SLOT(setValue(int)));
	
	connect(m_clipsViewPort->verticalScrollBar(), 
		SIGNAL(valueChanged(int)),
		m_trackPanel->verticalScrollBar(), 
		SLOT(setValue(int)));
	
	connect(m_trackPanel->verticalScrollBar(), 
		SIGNAL(valueChanged(int)),
		m_clipsViewPort->verticalScrollBar(), 
		SLOT(setValue(int)));
	
	connect(themer(), SIGNAL(themeLoaded()), this, SLOT(load_theme_data()), Qt::QueuedConnection);
	
// 	m_usingOpenGL  = false;
// 	set_use_opengl(config().get_property("Interface", "OpenGL", false).toBool());

	setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

	cpointer().set_current_viewport(m_clipsViewPort);
	m_clipsViewPort->setFocus();
}


SheetWidget::~ SheetWidget()
{
	PENTERDES;
	if (!m_sheet) {
		return;
	}

	delete m_scene;
}


QSize SheetWidget::minimumSizeHint() const
{
	return QSize(400, 200);
}

QSize SheetWidget::sizeHint() const
{
	return QSize(700, 600);
}

// void SheetWidget::set_use_opengl( bool useOpenGL )
// {
// 	if (!m_sheet) {
// 		return;
// 	}
// 	
// 	if (useOpenGL != m_usingOpenGL) {
// #if defined (QT_OPENGL_SUPPORT)
// 		m_clipsViewPort->setViewport(useOpenGL ? new QGLWidget(QGLFormat(QGL::SampleBuffers)) : new QWidget);
// 		m_trackPanel->setViewport(useOpenGL ? new QGLWidget(QGLFormat(QGL::SampleBuffers)) : new QWidget);
// #endif
// 	}
// 	m_usingOpenGL = useOpenGL;
// }


void SheetWidget::load_theme_data()
{
	QList<QGraphicsItem*> list = m_scene->items();
	
	for (int i = 0; i < list.size(); ++i) {
		ViewItem* item = qgraphicsitem_cast<ViewItem*>(list.at(i));
		if (item) {
			item->load_theme_data();
		}
	}
	
}

Sheet * SheetWidget::get_sheet() const
{
	return m_sheet;
}

SheetView * SheetWidget::get_sheetview() const
{
	return m_sv;
}

