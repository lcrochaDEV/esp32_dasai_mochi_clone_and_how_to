#include "WirelessConnection.h"


// WirelesConnection.cpp
WirelessConnection::WirelessConnection(const char* ssid, const char* password, Animations* animationPtr)
     : WifiConnect(ssid, password, animationPtr) 
{}