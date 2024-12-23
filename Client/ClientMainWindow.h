#pragma once

#include <QtWidgets/QMainWindow>

#include "Client.h"
#include "ui_ClientMainWindow.h"

class ClientMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	ClientMainWindow(QWidget* parent = nullptr);
	~ClientMainWindow();

private:
	Ui::ClientMainWindowClass m_ui;

	Client m_client;

};