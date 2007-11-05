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

#include "PluginSelectorDialog.h"

#include <QHeaderView>

#if defined (LV2_SUPPORT)
#include <LV2Plugin.h>
#endif

#include "Interface.h"
#include <Plugin.h>
#include <PluginManager.h>
#include <Information.h>
#include <Utils.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

PluginSelectorDialog* PluginSelectorDialog::m_instance = 0;

PluginSelectorDialog::PluginSelectorDialog(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);

	pluginTreeWidget->header()->resizeSection(0, 250);
	pluginTreeWidget->header()->setResizeMode(1, QHeaderView::ResizeToContents);
	pluginTreeWidget->header()->resizeSection(2, 60);
	

#if defined (LV2_SUPPORT)
	SLV2Plugins pluginList = PluginManager::instance()->get_slv2_plugin_list();

	QMap<QString, PluginInfo> plugins;
	
	for (uint i=0; i < slv2_plugins_size(pluginList); ++i) {
		const SLV2Plugin plugin = slv2_plugins_get_at(pluginList, i);
		PluginInfo pinfo = LV2Plugin::get_plugin_info(plugin);
		plugins.insertMulti(pinfo.type, pinfo);
	}
	
	foreach(PluginInfo pinfo, plugins) {
		
		if ( (pinfo.audioPortInCount == 1 && pinfo.audioPortOutCount ==  1) ||
		     (pinfo.audioPortInCount == 2 && pinfo.audioPortOutCount ==  2) ) {
			
			QString inoutcount = pinfo.audioPortInCount == 1 ? "Mono" : "Stereo";
			
			QTreeWidgetItem* item = new QTreeWidgetItem(pluginTreeWidget);
			item->setText(0, pinfo.name);
			item->setText(1, pinfo.type.remove("Plugin"));
			item->setText(2, inoutcount);
			item->setData(0, Qt::UserRole, QString(pinfo.uri));
			item->setToolTip(0, pinfo.name);
		}
	}
#endif

	connect(pluginTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(plugin_double_clicked()));
}

PluginSelectorDialog::~PluginSelectorDialog( )
{}

void PluginSelectorDialog::on_cancelButton_clicked( )
{
	reject();
}

void PluginSelectorDialog::on_okButton_clicked( )
{
	Plugin* plugin = 0;

#if defined (LV2_SUPPORT)
	QList<QTreeWidgetItem *> list = pluginTreeWidget->selectedItems();
	
	if ( ! list.size()) {
		printf("No plugin selected\n");
		reject();
		return;
	}
	
	QTreeWidgetItem* item =  list.first();
	
	QString uri = item->data(0, Qt::UserRole).toString();

 	plugin = PluginManager::instance()->create_lv2_plugin(uri);
#endif

	if (!plugin) {
		reject();
	}

	m_plugin = plugin;
	
	accept();
}


void PluginSelectorDialog::plugin_double_clicked()
{
	on_okButton_clicked();
}

PluginSelectorDialog* PluginSelectorDialog::instance()
{
	if (m_instance == 0) {
		m_instance = new PluginSelectorDialog(Interface::instance());
	}

	return m_instance;
}

Plugin* PluginSelectorDialog::get_selected_plugin( )
{
	Plugin* plugin = m_plugin;
	m_plugin = 0;

	return plugin;
}

void PluginSelectorDialog::set_description(const QString & des)
{
	objectToAddPluginTooLabel->setText(des);
}

