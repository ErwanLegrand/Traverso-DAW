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
#include "Peak.h"

#include <Sheet.h>
#include "Utils.h"
#include "ContextPointer.h"
#include "Mixer.h"

#include <QGridLayout>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

#include <Debugger.h>

SheetPanelView::SheetPanelView(QGraphicsScene* scene, TSession* sheet)
	: ViewItem(0, 0)
	, m_sheet(sheet)
{
        scene->addItem(this);
        m_boundingRect = QRectF(0, 0, 200, TIMELINE_HEIGHT);
}


void SheetPanelView::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
        painter->fillRect(-3, 0, 3, -TIMELINE_HEIGHT - 1, themer()->get_color("TrackPanel:trackseparation"));
}

class SheetPanelViewPort : public ViewPort
{
public:
	SheetPanelViewPort(QGraphicsScene* scene, SheetWidget* sw);
        ~SheetPanelViewPort() {}

        void set_sheet_view(SheetView* view) { m_sv = view;}

private:
        Sheet*          m_sheet;
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
        setBackgroundBrush(themer()->get_color("Timeline:background"));

        m_spv = new SheetPanelView(scene, sw->get_sheet());

        QHBoxLayout* m_mainLayout = new QHBoxLayout(this);
        m_mainLayout->addWidget(new TTimeLabel(this, sw->get_sheet()));
        setLayout(m_mainLayout);

}

TTimeLabel::TTimeLabel(QWidget* parent, TSession* session)
        : QPushButton(parent)
        , m_session(session)
{
        setEnabled(false);

        setMaximumWidth(120);
        setFocusPolicy(Qt::NoFocus);
        setStyleSheet(  "color: lime;"
                        "background-color: black;"
                        "font: 17px;"
                        "border-radius: 5px;"
                        "padding: 0 4 0 4;");


        connect(m_session, SIGNAL(transportStarted()), this, SLOT(transport_started()));
        connect(m_session, SIGNAL(transportStopped()), this, SLOT(transport_stopped()));
        connect(m_session, SIGNAL(transportPosSet()), this, SLOT(update_label()));
        connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(update_label()));

        update_label();
}

void TTimeLabel::transport_started()
{
    // use an odd number for the update interval, because
        // a round number (e.g. 100) lets the last digit stay
        // the same most of the time, but not always, which
        // looks jerky
        m_updateTimer.start(123);
}

void TTimeLabel::transport_stopped()
{
        m_updateTimer.stop();
}


void TTimeLabel::update_label()
{
        QString currentTime;

        if (!m_session) {
                currentTime = "";
        } else {
                currentTime = timeref_to_ms_2(m_session->get_transport_location());
        }
        setText(currentTime);
}


SheetWidget::SheetWidget(TSession* sheet, QWidget* parent)
	: QFrame(parent)
	, m_session(sheet)
{
	if (!m_session) {
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


        QWidget* zoomWidget = new QWidget(this);
        zoomWidget->setMaximumWidth(200);
        QHBoxLayout* zoomLayout = new QHBoxLayout(zoomWidget);
        QLabel* zoomLabel = new QLabel(this);
        zoomLabel->setText("Zoom");
        m_zoomSlider = new QSlider(this);
        m_zoomSlider->setMaximum(Peak::ZOOM_LEVELS);
        m_zoomSlider->setMinimum(0);
        m_zoomSlider->setPageStep(4);
        m_zoomSlider->setSingleStep(1);
        m_zoomSlider->setOrientation(Qt::Horizontal);
        sheet_zoom_level_changed();

        connect(m_zoomSlider, SIGNAL(sliderMoved(int)), this, SLOT(zoom_slider_value_changed(int)));
        connect(m_session, SIGNAL(hzoomChanged()), this, SLOT(sheet_zoom_level_changed()));

        zoomLayout->addWidget(zoomLabel);
        zoomLayout->addWidget(m_zoomSlider);

        m_mainLayout = new QGridLayout(this);
        m_mainLayout->addWidget(m_sheetPanelVP, 0, 0);
        m_mainLayout->addWidget(zoomWidget, 2, 0);
	m_mainLayout->addWidget(m_timeLine, 0, 1);
	m_mainLayout->addWidget(m_trackPanel, 1, 0);
	m_mainLayout->addWidget(m_clipsViewPort, 1, 1);
	m_mainLayout->addWidget(m_hScrollBar, 2, 1);
	m_mainLayout->addWidget(m_vScrollBar, 1, 2);
	
	m_mainLayout->setMargin(0);
	m_mainLayout->setSpacing(0);

	setLayout(m_mainLayout);
	
	m_sv = new SheetView(this, m_clipsViewPort, m_trackPanel, m_timeLine, sheet);
        m_clipsViewPort->set_sheetview(m_sv);
        m_trackPanel->set_sheetview(m_sv);
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
	
	setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

	cpointer().set_current_viewport(m_clipsViewPort);
	m_clipsViewPort->setFocus();
}


SheetWidget::~ SheetWidget()
{
	PENTERDES;
	if (!m_session) {
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

TSession * SheetWidget::get_sheet() const
{
	return m_session;
}

SheetView * SheetWidget::get_sheetview() const
{
	return m_sv;
}

void SheetWidget::zoom_slider_value_changed(int value)
{
        m_session->set_hzoom(Peak::zoomStep[value]);
}

void SheetWidget::sheet_zoom_level_changed()
{
        int level = m_session->get_hzoom();
        for (int i=0; i<Peak::ZOOM_LEVELS; ++i) {
                if (level == Peak::zoomStep[i]) {
                        m_zoomSlider->setValue(i);
                        return;
                }
        }
}
