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

		
#include "SongWidget.h"
#include "TrackPanelViewPort.h"
#include "ClipsViewPort.h"
#include "TimeLineViewPort.h"
#include "SongView.h"
#include "ViewItem.h"
#include "Themer.h"
#include "Config.h"

#include <Song.h>
#include "Utils.h"
#include "ContextPointer.h"
#include "Mixer.h"

#include <QtOpenGL>

#include <Debugger.h>


class SongPanelGain : public ViewItem
{
	Q_OBJECT
public:
	SongPanelGain(ViewItem* parent, Song* song);
	SongPanelGain(){}

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

public slots:
	Command* gain_increment();
	Command* gain_decrement();
	
private slots:
	void update_gain() {update();}

private:
	Song* m_song;
};


SongPanelGain::SongPanelGain(ViewItem* parent, Song* song)
	: ViewItem(parent, song)
	, m_song(song)
{
	m_boundingRect = QRectF(0, 0, 180, 9);
	connect(song, SIGNAL(masterGainChanged()), this, SLOT(update_gain()));
	setAcceptsHoverEvents(true);
}

void SongPanelGain::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	Q_UNUSED(widget);
	const int height = 9;

	int sliderWidth = (int)m_boundingRect.width() - 75;
	float gain = m_song->get_gain();
	QString sgain = coefficient_to_dbstring(gain);
	float db = coefficient_to_dB(gain);

	if (db < -60) {
		db = -60;
	}
	int sliderdbx =  (int) (sliderWidth - (sliderWidth*0.3)) - (int) ( ( (-1 * db) / 60 ) * sliderWidth);
	if (sliderdbx < 0) {
		sliderdbx = 0;
	}
	if (db > 0) {
		sliderdbx =  (int)(sliderWidth*0.7) + (int) ( ( db / 6 ) * (sliderWidth*0.3));
	}

	int cr = (gain >= 1 ? 30 + (int)(100 * gain) : (int)(50 * gain));
	int cb = ( gain < 1 ? 150 + (int)(50 * gain) : abs((int)(10 * gain)) );
	
	painter->setPen(themer()->get_color("TrackPanel:text"));
	painter->setFont(themer()->get_font("TrackPanel:gain"));
	painter->drawText(0, height + 1, "GAIN");
	painter->drawRect(30, 1, sliderWidth, height);
	
	bool mousehover = (option->state & QStyle::State_MouseOver);
	QColor color(cr,0,cb);
	if (mousehover) {
		color = color.light(140);
	}
	painter->fillRect(31, 2, sliderdbx, height-1, color);
	painter->drawText(sliderWidth + 35, height, sgain);
}

Command* SongPanelGain::gain_increment()
{
	m_song->set_gain(m_song->get_gain() + 0.05);
	return 0;
}

Command* SongPanelGain::gain_decrement()
{
	m_song->set_gain(m_song->get_gain() - 0.05);
	return 0;
}


class SongPanelView : public ViewItem
{
	Q_OBJECT
public:
	SongPanelView(QGraphicsScene* scene, Song* song);
	~SongPanelView() {}
	
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	
private:
	SongPanelGain* m_gainview;
	Song* m_song;
};

SongPanelView::SongPanelView(QGraphicsScene* scene, Song* song)
	: ViewItem(0, 0)
	, m_song(song)
{
	scene->addItem(this);
	m_gainview = new SongPanelGain(this, m_song);
	m_gainview->setPos(10, 16);
	scene->addItem(m_gainview);
	m_boundingRect = QRectF(0, 0, 200, TIMELINE_HEIGHT);
}

void SongPanelView::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
	int xstart = (int)option->exposedRect.x();
	int pixelcount = (int)option->exposedRect.width();
	
	painter->setPen(themer()->get_color("TrackPanel:text"));
	painter->setFont(themer()->get_font("TrackPanel:led"));
	painter->drawText(10, 11, "Song: " + m_song->get_title());
	
	QColor color = QColor(Qt::darkGray);//themer()->get_color("Track:cliptopoffset");
	painter->setPen(color);
	painter->fillRect(xstart, TIMELINE_HEIGHT - 2, pixelcount, 2, color);
	painter->fillRect(199, 0, 1, TIMELINE_HEIGHT, color);
}

class SongPanelViewPort : public ViewPort
{
public:
	SongPanelViewPort(QGraphicsScene* scene, SongWidget* sw);
	~SongPanelViewPort() {};

	void get_pointed_context_items(QList<ContextItem* > &list)
	{
		QList<QGraphicsItem *> itemlist = items(cpointer().on_first_input_event_x(), cpointer().on_first_input_event_y());
		foreach(QGraphicsItem* item, itemlist) {
			list.append((ViewItem*)item);
		}
		list.removeAll(m_spv);
		list.append(m_sv);
	}
	
	void set_song_view(SongView* sv)
	{
		m_sv = sv;
		m_spv = new SongPanelView(scene(), m_song);
		m_spv->setPos(-200, -TIMELINE_HEIGHT);
	}

private:
	Song*	m_song;
	SongView* m_sv;
	SongPanelView* m_spv;
};


SongPanelViewPort::SongPanelViewPort(QGraphicsScene * scene, SongWidget * sw)
	: ViewPort(scene, sw)
{
	setSceneRect(-200, -TIMELINE_HEIGHT, 200, 0);
	m_song = sw->get_song();
	
	setMaximumHeight(TIMELINE_HEIGHT);
	setMinimumHeight(TIMELINE_HEIGHT);
	setMinimumWidth(200);
	setMaximumWidth(200);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setBackgroundBrush(themer()->get_color("SongPanel:background"));
}


#include "SongWidget.moc"



SongWidget::SongWidget(Song* song, QWidget* parent)
	: QFrame(parent)
	, m_song(song)
{
	m_scene = new QGraphicsScene();
	m_vScrollBar = new QScrollBar(this);
	m_hScrollBar = new QScrollBar(this);
	m_hScrollBar->setOrientation(Qt::Horizontal);

	m_trackPanel = new TrackPanelViewPort(m_scene, this);
	m_clipsViewPort = new ClipsViewPort(m_scene, this);
	m_timeLine = new TimeLineViewPort(m_scene, this, m_clipsViewPort);
	m_songPanelVP = new SongPanelViewPort(m_scene, this);
	
	m_mainLayout = new QGridLayout(this);
	m_mainLayout->addWidget(m_songPanelVP, 0, 0);
	m_mainLayout->addWidget(m_timeLine, 0, 1);
	m_mainLayout->addWidget(m_trackPanel, 1, 0);
	m_mainLayout->addWidget(m_clipsViewPort, 1, 1);
	m_mainLayout->addWidget(m_hScrollBar, 2, 1);
	m_mainLayout->addWidget(m_vScrollBar, 1, 2);
	
	m_mainLayout->setMargin(0);
	m_mainLayout->setSpacing(0);

	setLayout(m_mainLayout);
	
	m_sv = new SongView(this, m_clipsViewPort, m_trackPanel, m_timeLine, song);
	m_timeLine->set_songview(m_sv);
	m_songPanelVP->set_song_view(m_sv);
	
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
	
	m_usingOpenGL  = false;
	set_use_opengl(config().get_property("Interface", "OpenGL", false).toBool());

	setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
}


SongWidget::~ SongWidget()
{
	delete m_trackPanel;
	delete m_clipsViewPort;
	delete m_timeLine;
	delete m_songPanelVP;
	delete m_scene;
}


QSize SongWidget::minimumSizeHint() const
{
	return QSize(400, 200);
}

QSize SongWidget::sizeHint() const
{
	return QSize(700, 600);
}

void SongWidget::set_use_opengl( bool useOpenGL )
{
	if (useOpenGL != m_usingOpenGL) {
		m_clipsViewPort->setViewport(useOpenGL ? new QGLWidget(QGLFormat(QGL::SampleBuffers)) : new QWidget);
		m_trackPanel->setViewport(useOpenGL ? new QGLWidget(QGLFormat(QGL::SampleBuffers)) : new QWidget);
	}
	m_usingOpenGL = useOpenGL;
}


void SongWidget::load_theme_data()
{
	QList<QGraphicsItem*> list = m_scene->items();
	
	for (int i = 0; i < list.size(); ++i) {
		ViewItem* item = qgraphicsitem_cast<ViewItem*>(list.at(i));
		if (item) {
			item->load_theme_data();
		}
	}
	
}

Song * SongWidget::get_song() const
{
	return m_song;
}

SongView * SongWidget::get_songview() const
{
	return m_sv;
}

//eof


