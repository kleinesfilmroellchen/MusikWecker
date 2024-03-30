#include "Debug.h"
#include <umm_malloc/umm_heap_select.h>

std::unique_ptr<DebugManager> DebugManager::instance;

DebugManager& DebugManager::the()
{
	HeapSelectIram iram;

	if (!DebugManager::instance)
		DebugManager::instance = std::make_unique<DebugManager>();

	return *DebugManager::instance.get();
}

DebugManager::DebugManager()
{
	log_server.begin();
	// Disable Nagle to send out all log messages immediately
	log_server.setNoDelay(true);
}

size_t DebugManager::write(uint8_t value)
{
#if USE_SERIAL
	Serial.write(value);
#endif
	size_t last_size = 0;

	for (auto& client : log_clients) {
		if (client && client.connected())
			last_size = client.write(value);
	}
	return last_size;
}

size_t DebugManager::write(const uint8_t* buffer, size_t size)
{
#if USE_SERIAL
	Serial.write(buffer, size);
#endif
	size_t last_size = 0;

	for (auto& client : log_clients) {
		if (client && client.connected())
			last_size = client.write(buffer, size);
	}
	return last_size;
}

void DebugManager::handle()
{
	HeapSelectIram iram;

	while (log_server.hasClient()) {
		log_clients.push_back(log_server.accept());
	}

	for (size_t i = 0; i < log_clients.size(); ++i) {
		auto& client = log_clients[i];
		if (!client || !client.connected()) {
			log_clients.erase(log_clients.begin() + i);
			--i;
			continue;
		}
		client.flush();
		// Echo functionality allows clients to check that their connection is working.
		while (client.available())
			client.write(client.read());
	}
}
