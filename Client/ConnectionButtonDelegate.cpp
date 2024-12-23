#include "ConnectionButtonDelegate.h"

#include <QPushButton>

ConnectionButtonDelegate::ConnectionButtonDelegate(QObject* parent)
	: QStyledItemDelegate(parent)
{
}

ConnectionButtonDelegate::~ConnectionButtonDelegate()
{
}

QWidget* ConnectionButtonDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QPushButton* btn = new QPushButton(parent);

	connect(btn, &QPushButton::clicked, this, [this, btn]() {
		const_cast<ConnectionButtonDelegate*>(this)->onButtonClicked(btn);
		});

	return btn;
}

void ConnectionButtonDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	QPushButton* btn = static_cast<QPushButton*>(editor);
	m_status = index.data(Qt::ItemDataRole::UserRole).value<CONNECTION_STATUS>();
	btn->setText(index.data(Qt::ItemDataRole::DisplayRole).toString());
}

void ConnectionButtonDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	QPushButton* btn = static_cast<QPushButton*>(editor);
	model->setData(index, QVariant::fromValue(m_status), Qt::ItemDataRole::UserRole);
	model->setData(index, btn->text(), Qt::ItemDataRole::EditRole);
}

void ConnectionButtonDelegate::onButtonClicked(QWidget* editor)
{
	switch (m_status)
	{
	case CONNECTION_STATUS::CONNECT:
		m_status = CONNECTION_STATUS::DISCONNECT;
		break;
	case CONNECTION_STATUS::DISCONNECT:
		m_status = CONNECTION_STATUS::CONNECT;
		break;
	}

	QPushButton* btn = static_cast<QPushButton*>(editor);
	btn->setText(STATUS_TO_STRING[m_status]);

	emit commitData(editor);
	emit closeEditor(editor);
}
