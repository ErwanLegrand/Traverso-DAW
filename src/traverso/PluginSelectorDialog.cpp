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
#include "ui_PluginSelectorDialog.h"

#include <QHeaderView>

#if defined (LV2_SUPPORT)
#include <LV2Plugin.h>
#endif
#include <Plugin.h>
#include <PluginManager.h>
#include <Information.h>
#include <Utils.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

PluginSelectorDialog* PluginSelectorDialog::m_instance = 0;

PluginSelectorDialog::PluginSelectorDialog( QWidget * p )
	: QDialog(p)
{
	setupUi(this);

	pluginTreeWidget->header()->setResizeMode(0, QHeaderView::ResizeToContents);
	

#if defined (LV2_SUPPORT)
	SLV2Plugins pluginList = PluginManager::instance()->get_slv2_plugin_list();

	for (uint i=0; i < slv2_plugins_size(pluginList); ++i) {
		const SLV2Plugin p = slv2_plugins_get_at(pluginList, i);

		QTreeWidgetItem* item = new QTreeWidgetItem(pluginTreeWidget);
		item->setText(0, QString( (char*) slv2_plugin_get_name(p)));
		item->setData(0, Qt::UserRole, QString( (char*) slv2_plugin_get_uri(p)));
		
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
#if defined (LV2_SUPPORT)
	LV2Plugin* plugin = 0;


	QList<QTreeWidgetItem *> list = pluginTreeWidget->selectedItems();
	
	if ( ! list.size()) {
		printf("No plugin selected\n");
		reject();
		return;
	}
	
	QTreeWidgetItem* item =  list.first();
	
	QString uri = item->data(0, Qt::UserRole).toString();

	plugin = new LV2Plugin(QS_C(uri));
	
	if (plugin->init() > 0) {
		m_plugin = plugin;
	} else {
		printf("Plugin init failed!");
		info().warning(tr("Plugin initialization failed!"));
		delete plugin;
		plugin = 0;
		reject();
	}

	m_plugin = plugin;
	accept();
#endif
}


void PluginSelectorDialog::plugin_double_clicked()
{
	on_okButton_clicked();
}

PluginSelectorDialog* PluginSelectorDialog::instance()
{
	if (m_instance == 0) {
		m_instance = new PluginSelectorDialog();
	}

	return m_instance;
}

Plugin* PluginSelectorDialog::get_selected_plugin( )
{
	Plugin* plugin = m_plugin;
	m_plugin = 0;

	return plugin;
}

//eof
