#pragma once

#include "Engine/Net/TCPSocket.hpp"
#include "Engine/Renderer/Mesh.hpp"

#include <string>
#include <vector>
#include <map>

enum eRCSState
{
	RCS_INITIAL,
	RCS_CLIENT,
	RCS_HOST,
	RCS_DELAY,
	NUM_RCS_STATE
};

class RCS
{
public:
	eRCSState m_rcsState = RCS_INITIAL;

	// clients and host
	std::vector<TCPSocket*> m_connections;

	// client
	
	// host
	TCPSocket* m_listenSocket = nullptr;
	static const int MAX_CONNECTIONS = 32;
	static constexpr uint16_t m_port = 29283;

	// delay
	float m_delay = 3.f;

	Mesh* m_titleMesh = nullptr; float m_titleMHeight; float m_titleMRatio = .5f; Vector2 m_titleMMin;
	Mesh* m_addrMesh = nullptr;	float m_addrMHeight; float m_addrMRatio = .5f; Vector2 m_addrMMin;
	Mesh* m_connNumMesh = nullptr; float m_connNumMHeight; float m_connNumMRatio = .5f; Vector2 m_connNumMMin;
	std::vector<Mesh*> m_connectMeshes;
	float m_connMHeight;
	float m_connMRatio = .5f;
	Vector2 m_connMMin;

	static RCS* m_rcsInstance;

	// echo
	uint8_t m_echo = 2;		// non-echo
	uint m_hostEchoIdx;

public:
	RCS();
	~RCS();

	void Update(float deltaTime);
	void UpdateState(float deltaTime);
	void UpdateText();

	void Render(Renderer* renderer);

	void SendMsg(uint idx, uint8_t isEcho, const char* str);
	void ReceiveAndRunMsg(TCPSocket* connection);

	TCPSocket* GetConnectionByIdx(uint idx);

	static RCS* GetInstance();
	static void DestroyInstance();
	void Startup();
	void ResetDelay() { m_delay = 3.f; }
};