/*
Copyright (C) 2005-2010 Remon Sijrier

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

#include <QLineEdit>
#include <QInputDialog>
#include <QGraphicsScene>

#include "TrackView.h"
#include "PluginChainView.h"
#include "Themer.h"
#include "TrackPanelViewPort.h"
#include "SheetView.h"
#include "TrackPanelView.h"
#include <Interface.h>

#include <Sheet.h>
#include <Track.h>
#include <Utils.h>

#include <PluginSelectorDialog.h>

#include <Debugger.h>

TrackView::TrackView(SheetView* sv, Track * track)
        : ViewItem(0, track)
{
        PENTERCONS;

        setZValue(sv->zValue() + 1);

        m_sv = sv;
        sv->scene()->addItem(this);

        load_theme_data();

        m_track = track;
        setFlags(ItemIsSelectable | ItemIsMovable);
        setCursor(themer()->get_cursor("Track"));

        m_pluginChainView = new PluginChainView(m_sv, this, m_track->get_plugin_chain());
}

TrackView:: ~ TrackView( )
{
}

void TrackView::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
        Q_UNUSED(widget);

// 	printf("TrackView:: PAINT :: exposed rect is: x=%f, y=%f, w=%f, h=%f\n", option->exposedRect.x(), option->exposedRect.y(), option->exposedRect.width(), option->exposedRect.height());

        int xstart = (int)option->exposedRect.x();
        int pixelcount = (int)option->exposedRect.width();

        if (m_topborderwidth > 0) {
                QColor color = themer()->get_color("Track:cliptopoffset");
                painter->fillRect(xstart, 0, pixelcount+1, m_topborderwidth, color);
        }

        if (m_paintBackground) {
                QColor color = themer()->get_color("Track:background");
                painter->fillRect(xstart, m_topborderwidth, pixelcount+1, m_track->get_height() - m_bottomborderwidth, color);
        }

        if (m_bottomborderwidth > 0) {
                QColor color = themer()->get_color("Track:clipbottomoffset");
                painter->fillRect(xstart, m_track->get_height() - m_bottomborderwidth, pixelcount+1, m_bottomborderwidth, color);
        }
}

int TrackView::get_childview_y_offset() const
{
        return m_topborderwidth + m_cliptopmargin;
}

void TrackView::move_to( int x, int y )
{
        Q_UNUSED(x);
        setPos(0, y);
        m_panel->setPos(-200, y);
}

int TrackView::get_height( )
{
        return m_track->get_height() - (m_topborderwidth + m_bottomborderwidth + m_clipbottommargin + m_cliptopmargin);
}

Command* TrackView::edit_properties( )
{
        bool ok;
        QString text = QInputDialog::getText(m_sv->get_trackpanel_view_port()->viewport(), tr("Edit name"),
                                        tr("Enter new name"),
                                        QLineEdit::Normal, m_track->get_name(), &ok);
        if (ok && !text.isEmpty()) {
                m_track->set_name(text);
        }

        return (Command*) 0;
}

Command* TrackView::add_new_plugin( )
{
        PluginSelectorDialog::instance()->set_description(tr("Track %1:  %2")
                        .arg(m_track->get_sort_index()+1).arg(m_track->get_name()));

        if (PluginSelectorDialog::instance()->exec() == QDialog::Accepted) {
                Plugin* plugin = PluginSelectorDialog::instance()->get_selected_plugin();
                if (plugin) {
                        // Force showing into effects mode, just in case the user adds
                        // a plugin in edit mode, which means it won't show up!
                        m_sv->get_sheet()->set_effects_mode();
                        return m_track->add_plugin(plugin);
                }
        }

        return 0;
}

void TrackView::set_height( int height )
{
        m_height = height;
}

void TrackView::calculate_bounding_rect()
{
        prepareGeometryChange();
        m_boundingRect = QRectF(0, 0, MAX_CANVAS_WIDTH, m_track->get_height());
        m_panel->calculate_bounding_rect();
        ViewItem::calculate_bounding_rect();
}

void TrackView::load_theme_data()
{
        m_paintBackground = themer()->get_property("Track:paintbackground").toInt();
        m_topborderwidth = themer()->get_property("Track:topborderwidth").toInt();
        m_bottomborderwidth = themer()->get_property("Track:bottomborderwidth").toInt();

        m_cliptopmargin = themer()->get_property("Track:cliptopmargin").toInt();
        m_clipbottommargin = themer()->get_property("Track:clipbottommargin").toInt();
}


Command* TrackView::select_bus()
{
        Interface::instance()->show_busselector((AudioTrack*)m_track);
        return 0;
}

