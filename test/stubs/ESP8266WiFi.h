#pragma once
#include "IPAddress.h"

struct FakeWifi {
  int RSSI() { return -60; }
  bool isConnected() { return true; }
  IPAddress localIP() { return {}; }
};
static FakeWifi WiFi;
