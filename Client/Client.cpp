#include "Client.h"

#include <iostream>
#include <QDebug>
#include <QEventLoop>
#include <QNetworkDatagram>
#include <QNetworkInterface>
#include <QRegularExpression>
#include <QThreadPool>

namespace
{
	constexpr quint16 PORT = 10001;

	const QRegularExpression MESSAGE_REGEXP("Qt GRPC Server:(?<ip>\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}):(?<port>[0-9]*)");
	const QString IP_TEMPLATE = "%1:%2";

	QString getIp()
	{
		for (const QNetworkInterface& netInterface : QNetworkInterface::allInterfaces())
		{
			QNetworkInterface::InterfaceFlags flags = netInterface.flags();
			if ((bool)(flags & QNetworkInterface::IsRunning) && !(bool)(flags & QNetworkInterface::IsLoopBack))
			{
				for (const QNetworkAddressEntry& address : netInterface.addressEntries())
					if (address.ip().protocol() == QAbstractSocket::IPv4Protocol)
						return address.ip().toString();
			}
		}
	}
}

PingWorker::PingWorker(const std::string& address, grpc::CompletionQueue& queue, QObject* parent)
	: QObject(parent)
	, m_address(address)
	, m_stub(MaintainingApi::NewStub(grpc::CreateChannel(m_address, grpc::InsecureChannelCredentials())))
	, m_queue(queue)
	, m_stop(false)
{
}

PingWorker::~PingWorker()
{
}

void PingWorker::run()
{
	QTimer timer(this);
	timer.setInterval(5000);
	timer.setSingleShot(true);
	m_lastSuccess = QDateTime::currentDateTime();

	while (!m_stop.load(std::memory_order_relaxed))
	{
		if (timer.remainingTimeAsDuration().count() <= 0)
		{
			this->ping();
			timer.start();
		}
	}

	timer.stop();
}

void PingWorker::stop()
{
	m_stop.store(true, std::memory_order_relaxed);
}

void PingWorker::ping()
{
	PingRequest request;
	request.set_clientip(m_address);

	PingResponse response;
	grpc::ClientContext context;
	grpc::Status status;
	status = m_stub->Ping(&context, request, &response);

	if (status.ok())
	{
		m_lastSuccess = QDateTime::currentDateTime();
		emit success(QString::fromStdString(m_address));
	}
	else
	{
		if (m_lastSuccess.secsTo(QDateTime::currentDateTime()) >= 15)
			emit failure(QString::fromStdString(m_address));
	}
}

Client::Client(QObject* parent)
	: QObject(parent)
	, m_ip(getIp())
	, m_listenerSocket(this)
{
	connect(&m_listenerSocket, &QUdpSocket::readyRead, this, &Client::onReadyRead);
	m_listenerSocket.bind(10001);
}

Client::~Client()
{
}

void Client::startPing(const QString& address)
{
	std::string serverAddress = address.toStdString();

	PingWorker* worker = new PingWorker(serverAddress, m_queue);
	worker->setAutoDelete(true);
	m_workers[serverAddress] = worker;

	connect(worker, &PingWorker::success, this, &Client::pingSuccess, Qt::ConnectionType::BlockingQueuedConnection);
	connect(worker, &PingWorker::failure, this, &Client::pingFailure, Qt::ConnectionType::BlockingQueuedConnection);
	connect(worker, &PingWorker::failure, this, &Client::stopPing, Qt::ConnectionType::BlockingQueuedConnection);

	QThreadPool::globalInstance()->start(worker);
}

void Client::stopPing(const QString& address)
{
	std::string serverAddress = address.toStdString();

	if (m_workers.contains(serverAddress))
	{
		m_workers[serverAddress]->stop();
		m_workers.remove(serverAddress);
	}
}

void Client::onReadyRead()
{
	while (m_listenerSocket.hasPendingDatagrams())
	{
		QNetworkDatagram data = m_listenerSocket.receiveDatagram();
		QRegularExpressionMatch match = MESSAGE_REGEXP.match(data.data());
		if (match.hasMatch())
		{
			const QString address = IP_TEMPLATE.arg(match.captured("ip"), match.captured("port"));
			emit serverFound(address);
		}
	}
}