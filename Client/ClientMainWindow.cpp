#include "ClientMainWindow.h"

#include "ConnectionButtonDelegate.h"
#include "ServersModel.h"

ClientMainWindow::ClientMainWindow(QWidget* parent)
	: QMainWindow(parent)
	, m_client(this)
{
	m_ui.setupUi(this);

	ServersModel* model = new ServersModel(m_ui.tableView);
	m_ui.tableView->setModel(model);

	m_ui.tableView->horizontalHeader()->setVisible(true);
	m_ui.tableView->setItemDelegateForColumn(colToInt(COLUMNS::CONNECT_COL), new ConnectionButtonDelegate(m_ui.tableView));

	connect(&m_client, &Client::serverFound, model, &ServersModel::addRow);

	connect(model, &ServersModel::connectServer, &m_client, &Client::startPing);
	connect(model, &ServersModel::disconnectServer, &m_client, &Client::stopPing);

	connect(&m_client, &Client::pingSuccess, model, &ServersModel::pingSuccess);
	connect(&m_client, &Client::pingFailure, model, &ServersModel::pingFailure);
}

ClientMainWindow::~ClientMainWindow()
{
}