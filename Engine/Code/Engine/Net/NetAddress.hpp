#pragma once
//#define WIN32_LEAN_AND_MEAN

#include <string.h>
#include <stdio.h>

#include "Engine/Core/Util/StringUtils.hpp"
#include "Engine/Core/Log/LogSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/WindowsCommon.hpp"

struct sNetAddress
{
public:
	sNetAddress() {}
	sNetAddress( const char* charArr ); 
	sNetAddress( const char* charArr, uint16_t port );

	bool ToSockAddr( sockaddr *out, size_t *out_addrlen ) const; 
	bool FromSockAddr( sockaddr const *sa ); 

	std::string ToString() const; 

public:
	uint ipv4_address;
	uint16_t port; 

public: 
	// STATICS
	static bool GetBindableAddr(sNetAddress* saddr, uint16_t port); 

	/* these are optional, but useful helpers
	static uint GetAllForHost( net_address_t *out, 
	uint max_count, 
	char const *host_name, 
	char const *service_name ); 
	static uint GetAllLocal( net_address_t *out, uint max_count ); */
};

bool GetAddressForHost( sockaddr* out, int* out_addrlen, const char* hostname, const char* service = "80");
void GetAddressString();
void GetAddressString(char* out);
void Connect( const sNetAddress& addr, const char* msg );
void TCPConnect(const sNetAddress& addr, const char* msg);
void Host(uint16_t port);
void TCPHost(uint16_t port);


