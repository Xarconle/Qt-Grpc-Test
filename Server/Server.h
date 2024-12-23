#pragma once

#include <QObject>
#include <QTimer>
#include <QUdpSocket>

#include "generated/api.grpc.pb.h"


class Server  final : public QObject
{
	Q_OBJECT;

	enum class CALL_STATUS
	{
		CREATE,
		PROCESS,
		FINISH
	};

	class CallData
	{
	public:
		CallData(MaintainingApi::AsyncService* service, grpc::ServerCompletionQueue* queue);

		CALL_STATUS status() const;

		void process();

		std::string senderAddress() const;

	private:
		MaintainingApi::AsyncService* m_service;

		grpc::ServerCompletionQueue* m_queue;
		grpc::ServerContext m_context;

		PingRequest m_request;
		PingResponse m_response;

		grpc::ServerAsyncResponseWriter<PingResponse> m_responder;

		CALL_STATUS m_status;

		std::string m_senderAddress;

	};

public:
	explicit Server(QObject* parent = nullptr);
	~Server();

	void start(quint16 port);
	void stop();

signals:
	void gotPing(const std::string& address);

private:
	void onBroadcastTimeOut();
	void handle();

private:
	bool m_isRunning = false;

	const QString m_ip;
	QString m_port;

	QTimer m_broadcastTimer;
	QUdpSocket m_broadcastSocket;

	QTimer m_timeoutTimer;

	MaintainingApi::AsyncService* m_service;
	std::unique_ptr<grpc::ServerCompletionQueue> m_queue;
	std::unique_ptr<grpc::Server> m_server;

};