#pragma once

#include <QStateMachine>
#include <QtWidgets/QMainWindow>

#include "Server.h"
#include "ui_ServerMainWindow.h"

class ServerMainWindow final : public QMainWindow
{
	Q_OBJECT

public:
	ServerMainWindow(QWidget* parent = nullptr);
	~ServerMainWindow();

private:
	void generateStateMachine();

	void start();
	void stop();

	void onPortChanged(QString text);

	void onPing(const std::string& address);

private:
	Ui::ServerMainWindowClass m_ui;

	QStateMachine m_stateMachine;

	Server m_server;

};