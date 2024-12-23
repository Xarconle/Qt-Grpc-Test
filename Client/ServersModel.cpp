#include "ServersModel.h"

#include <QTime>

ServersModel::ServersModel(QObject* parent) : QAbstractItemModel(parent)
{
}

ServersModel::~ServersModel()
{
}

int ServersModel::columnCount(const QModelIndex& parent) const
{
	return HEADER.size();
}

int ServersModel::rowCount(const QModelIndex& parent) const
{
	return m_data.size();
}

QModelIndex ServersModel::index(int row, int column, const QModelIndex& parent) const
{
	if (parent.isValid())
		return QModelIndex();

	return this->createIndex(row, column);
}

QModelIndex ServersModel::parent(const QModelIndex& index) const
{
	return QModelIndex();
}

Qt::ItemFlags ServersModel::flags(const QModelIndex& index) const
{
	Qt::ItemFlags flags = {};

	if (!index.isValid())
		return flags;

	flags |= Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;

	if (index.column() == colToInt(COLUMNS::CONNECT_COL))
		flags |= Qt::ItemFlag::ItemIsEditable;

	return flags;
}

QVariant ServersModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation != Qt::Orientation::Horizontal)
		return QVariant();

	if (role == Qt::ItemDataRole::DisplayRole)
		return HEADER[intToCol(section)];

	return QVariant();
}

QVariant ServersModel::data(const QModelIndex& index, int role) const
{
	int row = index.row();
	int col = index.column();

	if (role == Qt::ItemDataRole::TextAlignmentRole)
		return Qt::AlignmentFlag::AlignCenter;
	else if (role == Qt::ItemDataRole::DisplayRole)
		return m_data[row][col];
	else if (role == Qt::ItemDataRole::ForegroundRole && index.column() == colToInt(COLUMNS::STATUS_COL))
		return m_connectionResult[row] == CONNECTION_RESULT::ONLINE ? QColor(Qt::GlobalColor::green) : QColor(Qt::GlobalColor::red);
	else if (role == Qt::ItemDataRole::UserRole)
		return QVariant::fromValue(m_connectionStatus[row]);

	return QVariant();
}

bool ServersModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	int row = index.row();
	int col = index.column();

	if (role == Qt::ItemDataRole::EditRole)
	{
		m_data[row][col] = value.toString();
		emit dataChanged(index, index, { role });
		return true;
	}
	else if (role == Qt::ItemDataRole::UserRole)
	{
		const QString address = m_data[row][colToInt(COLUMNS::ADDRESS_COL)];

		switch (m_connectionStatus[row])
		{
		case CONNECTION_STATUS::CONNECT:
			emit disconnectServer(address);
			m_connectionResult[row] = CONNECTION_RESULT::OFFLINE;
			m_data[row][colToInt(COLUMNS::STATUS_COL)] = RESULT_TO_STRING[CONNECTION_RESULT::OFFLINE];
			emit dataChanged(this->index(row, colToInt(COLUMNS::STATUS_COL)), this->index(row, colToInt(COLUMNS::STATUS_COL)), { Qt::ItemDataRole::DisplayRole, Qt::ItemDataRole::ForegroundRole });
			break;
		case CONNECTION_STATUS::DISCONNECT:
			emit connectServer(address);
			break;
		}

		m_connectionStatus[row] = value.value<CONNECTION_STATUS>();


		return true;
	}

	return false;
}

void ServersModel::addRow(const QString& address)
{
	for (const QList<QString>& row : m_data)
		if (row[0] == address)
			return;

	this->beginInsertRows(QModelIndex(), m_data.size(), m_data.size());
	m_data.append({ address, "-", RESULT_TO_STRING[CONNECTION_RESULT::OFFLINE], STATUS_TO_STRING[CONNECTION_STATUS::DISCONNECT] });
	m_connectionStatus.append(CONNECTION_STATUS::DISCONNECT);
	m_connectionResult.append(CONNECTION_RESULT::OFFLINE);
	this->endInsertRows();
}

void ServersModel::delRow(const QString& address)
{
	int row = getRow(address);

	if (row == m_data.size())
		return;

	this->beginRemoveRows(QModelIndex(), row, row);
	m_data.removeAt(row);
	m_connectionStatus.removeAt(row);
	m_connectionResult.removeAt(row);
	this->endRemoveRows();
}

void ServersModel::pingSuccess(const QString& address)
{
	int row = getRow(address);

	m_data[row][colToInt(COLUMNS::TIME_COL)] = QTime::currentTime().toString();
	m_data[row][colToInt(COLUMNS::STATUS_COL)] = RESULT_TO_STRING[CONNECTION_RESULT::ONLINE];
	m_connectionResult[row] = CONNECTION_RESULT::ONLINE;

	emit dataChanged(this->index(row, colToInt(COLUMNS::TIME_COL)), this->index(row, colToInt(COLUMNS::STATUS_COL)), { Qt::ItemDataRole::DisplayRole, Qt::ItemDataRole::ForegroundRole });
}

void ServersModel::pingFailure(const QString& address)
{
	int row = getRow(address);

	m_data[row][colToInt(COLUMNS::STATUS_COL)] = RESULT_TO_STRING[CONNECTION_RESULT::OFFLINE];
	m_connectionResult[row] = CONNECTION_RESULT::OFFLINE;
	m_data[row][colToInt(COLUMNS::CONNECT_COL)] = STATUS_TO_STRING[CONNECTION_STATUS::DISCONNECT];
	m_connectionStatus[row] = CONNECTION_STATUS::DISCONNECT;

	emit dataChanged(this->index(row, colToInt(COLUMNS::STATUS_COL)), this->index(row, colToInt(COLUMNS::CONNECT_COL)), { Qt::ItemDataRole::DisplayRole, Qt::ItemDataRole::ForegroundRole, Qt::ItemDataRole::UserRole });
}

int ServersModel::getRow(const QString& address)
{
	int rowId = 0;

	for (const QList<QString>& row : m_data)
	{
		if (row[0] == address)
			break;
		++rowId;
	}

	return rowId;
}