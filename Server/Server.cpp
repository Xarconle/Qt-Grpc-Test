#include "Server.h"

#include <grpc++/server_builder.h>
#include <QNetworkInterface>
#include <QThreadPool>

namespace
{
	const QString ADDRESS_TEMPLATE = "%1:%2";
	const QString SEND_MESSAGE = "Qt GRPC Server:%1:%2";

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

Server::CallData::CallData(MaintainingApi::AsyncService* service, grpc::ServerCompletionQueue* queue)
	: m_service(service)
	, m_queue(queue)
	, m_responder(&m_context)
	, m_status(CALL_STATUS::CREATE)
{
	process();
}

Server::CALL_STATUS Server::CallData::status() const
{
	return m_status;
}

void Server::CallData::process()
{
	switch (m_status)
	{
	case CALL_STATUS::CREATE:
	{
		m_status = CALL_STATUS::PROCESS;
		m_service->RequestPing(&m_context, &m_request, &m_responder, m_queue, m_queue, this);
		break;
	}
	case CALL_STATUS::PROCESS:
	{
		new CallData{ m_service, m_queue };
		m_senderAddress = m_request.clientip();
		m_status = CALL_STATUS::FINISH;
		m_responder.Finish(m_response, grpc::Status::OK, this);
		break;
	}
	default:
	{
		delete this;
	}
	}
}

std::string Server::CallData::senderAddress() const
{
	return m_senderAddress;
}

Server::Server(QObject* parent)
	: QObject(parent)
	, m_ip(getIp())
	, m_broadcastTimer(this)
	, m_broadcastSocket(this)
{
	m_broadcastTimer.setInterval(5000);
	m_timeoutTimer.setInterval(15000);

	connect(&m_broadcastTimer, &QTimer::timeout, this, &Server::onBroadcastTimeOut);
	connect(&m_timeoutTimer, &QTimer::timeout, &m_broadcastTimer, QOverload<>::of(&QTimer::start));

	connect(this, &Server::gotPing, &m_broadcastTimer, &QTimer::stop, Qt::QueuedConnection);
	connect(this, &Server::gotPing, &m_timeoutTimer, QOverload<>::of(&QTimer::start), Qt::QueuedConnection);
}

Server::~Server()
{
	this->stop();
}

void Server::start(quint16 port)
{
	m_isRunning = true;

	m_port = QString::number(port);
	this->onBroadcastTimeOut();

	std::string serverAddress = ADDRESS_TEMPLATE.arg(m_ip, m_port).toStdString();
	m_service = new MaintainingApi::AsyncService;

	grpc::ServerBuilder builder;
	builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());
	builder.RegisterService(m_service);
	m_queue = builder.AddCompletionQueue();
	m_server = builder.BuildAndStart();

	QThreadPool::globalInstance()->start([this]() {
		this->handle();
		});

	m_broadcastTimer.start();
}

void Server::stop()
{
	if (!m_isRunning)
		return;

	m_isRunning = false;
	m_timeoutTimer.stop();
	m_broadcastTimer.stop();

	m_server->Shutdown();
	m_queue->Shutdown();

	m_server.reset();
	m_queue.reset();

	delete m_service;
}

void Server::onBroadcastTimeOut()
{
	QByteArray data = SEND_MESSAGE.arg(m_ip, m_port).toUtf8();
	m_broadcastSocket.writeDatagram(data, QHostAddress::Broadcast, 10001);
}

void Server::handle()
{
	new CallData(m_service, m_queue.get());
	void* tag;
	bool ok;
	while (m_isRunning)
	{
		if (m_queue->Next(&tag, &ok) && ok)
		{
			CallData* data = static_cast<CallData*>(tag);

			if (data->status() == CALL_STATUS::FINISH)
				emit gotPing(data->senderAddress());

			data->process();
		}
	}
}