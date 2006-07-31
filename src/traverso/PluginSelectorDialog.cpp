/*
Copyright (C) 2006 Remon Sijrier 

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

$Id: PluginSelectorDialog.cpp,v 1.1 2006/07/31 13:27:59 r_sijrier Exp $
*/

#include "PluginSelectorDialog.h"
#include "ui_PluginSelectorDialog.h"

#include <QStandardItemModel>
#include <QHeaderView>

#include <LV2Plugin.h>
#include <Plugin.h>
#include <PluginManager.h>
#include <Information.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

PluginSelectorDialog* PluginSelectorDialog::m_instance = 0;

PluginSelectorDialog::PluginSelectorDialog( QWidget * p )
		: QDialog(p)
{
	setupUi(this);

	QStandardItemModel *model = new QStandardItemModel();
	QModelIndex parent;

	model->insertColumns(0, 2, parent);
	
	SLV2List pluginList = PluginManager::instance()->get_slv2_plugin_list();
	
	for (uint i=0; i < slv2_list_get_length(pluginList); ++i) {
		const SLV2Plugin* const p = slv2_list_get_plugin_by_index(pluginList, i);
		
		model->insertRows(i, 1, parent);
		QModelIndex index = model->index(i, 0, parent);
		model->setData(index, QString( (char*) slv2_plugin_get_name(p)) );
		index = model->index(i, 1, parent);
		model->setData(index, QString( (char*) slv2_plugin_get_uri(p)) );
	}
	
	
	pluginTreeView->setModel(model);
	
	QStringList stringList;
	stringList << "Name" << "Uri";
	
	pluginTreeView->header()->resizeSection(0, 200);
	
	connect(pluginTreeView, SIGNAL(doubleClicked(QModelIndex& index )), this, SLOT(model_double_clicked( const QModelIndex& index )));
}

PluginSelectorDialog::~PluginSelectorDialog( )
{}

void PluginSelectorDialog::on_cancelButton_clicked( )
{
}

void PluginSelectorDialog::on_okButton_clicked( )
{
	LV2Plugin* plugin = 0;
	
	
	QModelIndex index = pluginTreeView->currentIndex();
	if (index.column() == 0)
		index = index.model()->index(index.row(), 1);
		
	if (index.isValid()) {
		QByteArray uri(index.data().toString().toAscii().data());
		
		plugin = new LV2Plugin(uri.data());
		if (plugin->init() > 0) {
			m_plugin = plugin;
		} else {
			printf("Plugin init failed!");
			info().warning(tr("Plugin initialization failed!"));
			delete plugin;
			plugin = 0;
		}
	} else {
		printf("no index selected\n");
	}
	
	m_plugin = plugin;
}


void PluginSelectorDialog::model_double_clicked( const QModelIndex& index )
{
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
