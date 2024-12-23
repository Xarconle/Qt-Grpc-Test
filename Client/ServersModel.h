#pragma once

#include "ConnectionButtonDelegate.h"
#include "QAbstractItemModel"

enum class COLUMNS
{
	ADDRESS_COL,
	TIME_COL,
	STATUS_COL,
	CONNECT_COL,
};

inline int colToInt(COLUMNS col)
{
	return static_cast<int>(col);
}

inline COLUMNS intToCol(int num)
{
	switch (num)
	{
	case 0: return COLUMNS::ADDRESS_COL;
	case 1: return COLUMNS::TIME_COL;
	case 2: return COLUMNS::STATUS_COL;
	case 3: return COLUMNS::CONNECT_COL;
	default: throw std::out_of_range("Wrong column num!");
	}
}

const QMap<COLUMNS, QString> HEADER = {
	{COLUMNS::ADDRESS_COL, "IP"},
	{COLUMNS::TIME_COL, "Last ping time"},
	{COLUMNS::STATUS_COL, "Status"},
	{COLUMNS::CONNECT_COL, "Action"},
};

enum class CONNECTION_RESULT {
	ONLINE,
	OFFLINE,
};

const QMap<CONNECTION_RESULT, QString> RESULT_TO_STRING = {
	{CONNECTION_RESULT::ONLINE, "Online"},
	{CONNECTION_RESULT::OFFLINE, "Offline"},
};

class ServersModel final : public QAbstractItemModel
{
	Q_OBJECT

public:
	explicit ServersModel(QObject* parent = nullptr);
	~ServersModel();

	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	virtual QModelIndex parent(const QModelIndex& index) const override;
	virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

	void addRow(const QString& address);
	void delRow(const QString& address);

	void pingSuccess(const QString& address);
	void pingFailure(const QString& address);

signals:
	void connectServer(const QString& address);
	void disconnectServer(const QString& address);

private:
	int getRow(const QString& address);

private:
	QList<QList<QString>> m_data;
	QList<CONNECTION_STATUS> m_connectionStatus;
	QList<CONNECTION_RESULT> m_connectionResult;

};