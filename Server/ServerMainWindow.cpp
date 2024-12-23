#include "ServerMainWindow.h"

#include <limits>
#include <QDateTime>
#include <QIntValidator>

namespace
{
	constexpr int MIN_PORT = 1025;
	constexpr int MAX_PORT = std::numeric_limits<uint16_t>().max();

	const QString MESSAGE_TEMPLATE = "%1:\tping from\t%2\n";
}

ServerMainWindow::ServerMainWindow(QWidget* parent)
	: QMainWindow(parent)
	, m_stateMachine(this)
	, m_server(this)
{
	m_ui.setupUi(this);
	this->generateStateMachine();

	m_ui.portLine->setValidator(new QIntValidator(MIN_PORT, MAX_PORT, m_ui.portLine));

	connect(m_ui.portLine, &QLineEdit::textChanged, this, &ServerMainWindow::onPortChanged);
	connect(&m_server, &Server::gotPing, this, &ServerMainWindow::onPing);
}

ServerMainWindow::~ServerMainWindow()
{
}

void ServerMainWindow::generateStateMachine()
{
	QState* wait = new QState();
	QState* broadcast = new QState();

	wait->addTransition(m_ui.startButton, &QPushButton::clicked, broadcast);
	broadcast->addTransition(m_ui.startButton, &QPushButton::clicked, wait);

	connect(wait, &QState::entered, this, &ServerMainWindow::stop);
	connect(broadcast, &QState::entered, this, &ServerMainWindow::start);

	m_stateMachine.addState(wait);
	m_stateMachine.addState(broadcast);

	m_stateMachine.setInitialState(wait);
	m_stateMachine.start();
}

void ServerMainWindow::start()
{
	m_ui.startButton->setText("Stop");
	m_ui.portLine->setEnabled(false);
	m_server.start(m_ui.portLine->text().toUShort());
}

void ServerMainWindow::stop()
{
	m_server.stop();
	m_ui.portLine->setEnabled(true);
	m_ui.startButton->setText("Start");
}

void ServerMainWindow::onPortChanged(QString text)
{
	int placeholder = 0;
	bool isEnabled = m_ui.portLine->validator()->validate(text, placeholder) == QValidator::State::Acceptable;
	m_ui.startButton->setEnabled(isEnabled);
}

void ServerMainWindow::onPing(const std::string& address)
{
	QTextCursor cursor = m_ui.logWidget->textCursor();
	cursor.insertText(MESSAGE_TEMPLATE.arg(QDateTime::currentDateTime().toString(), QString::fromStdString(address)));
}