#pragma once

#include <QStyledItemDelegate>

enum class CONNECTION_STATUS
{
	CONNECT,
	DISCONNECT
};

Q_DECLARE_METATYPE(CONNECTION_STATUS);

const QMap<CONNECTION_STATUS, QString> STATUS_TO_STRING = {
	{CONNECTION_STATUS::CONNECT, "Disconnect"},
	{CONNECTION_STATUS::DISCONNECT, "Connect"},
};

class ConnectionButtonDelegate final : public QStyledItemDelegate
{
	Q_OBJECT

public:
	explicit ConnectionButtonDelegate(QObject* parent = nullptr);
	~ConnectionButtonDelegate();

	virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

	virtual void setEditorData(QWidget* editor, const QModelIndex& index) const override;
	virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

private:
	void onButtonClicked(QWidget* editor);

private:
	mutable CONNECTION_STATUS m_status = CONNECTION_STATUS::DISCONNECT;

};

