#pragma once

#include <atomic>
#include <grpc++/grpc++.h>
#include <QDateTime>
#include <QObject>
#include <QRunnable>
#include <QTimer>
#include <QUdpSocket>

#include "generated/api.grpc.pb.h"

struct AsyncClientCall
{
	PingResponse response;
	grpc::ClientContext context;
	grpc::Status status;
	std::unique_ptr<grpc::ClientAsyncResponseReader<PingResponse>> rpc;
};

class PingWorker final : public QObject, public QRunnable
{
	Q_OBJECT;

public:
	explicit PingWorker(const std::string& address, grpc::CompletionQueue& queue, QObject* parent = nullptr);
	~PingWorker();

	virtual void run() override;

	void stop();

signals:
	void success(const QString& address);
	void failure(const QString& address);

private:
	void ping();

private:
	const std::string m_address;

	std::unique_ptr<MaintainingApi::Stub> m_stub;
	grpc::CompletionQueue& m_queue;

	std::atomic<bool> m_stop;

	QDateTime m_lastSuccess;

};

class Client final : public QObject
{
	Q_OBJECT;

	struct AsyncClientCall
	{
		PingResponse response;
		grpc::ClientContext context;
		grpc::Status status;
		std::unique_ptr<grpc::ClientAsyncResponseReader<PingResponse>> rpc;
	};

public:
	explicit Client(QObject* parent = nullptr);
	~Client();

	void startPing(const QString& address);
	void stopPing(const QString& address);

signals:
	void serverFound(const QString& address);
	void serverLost(const QString& address);

	void pingSuccess(const QString& address);
	void pingFailure(const QString& address);

private:
	void onReadyRead();

private:
	const QString m_ip;

	QUdpSocket m_listenerSocket;

	QMap<std::string, PingWorker*> m_workers;
	grpc::CompletionQueue m_queue;

};