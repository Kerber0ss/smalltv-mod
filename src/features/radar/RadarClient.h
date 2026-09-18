// RadarClient.h — low-memory client for radar.syslog.pp.ua air alerts.
#pragma once
#include <Arduino.h>
#include "Settings.h"

enum RadarStage : uint8_t {
  RADAR_IDLE = 0,
  RADAR_NO_REGION,
  RADAR_LOW_HEAP,
  RADAR_CONNECT_FAIL,
  RADAR_HTTP_ERROR,
  RADAR_PARSE_FAIL,
  RADAR_OK,
};

struct RadarSituation {
  char     regionName[48];
  char     placeName[48];
  uint16_t drones;
  uint16_t missiles;       // cruise + ballistic missiles
  uint8_t  level;          // 0 = green, 1 = yellow, 2 = red
  bool     valid;
};

void radarInit(const Settings& s);
void radarService(const Settings& s);
void radarForceRefresh();
const RadarSituation& radarSituation();
uint32_t radarLastOkMs();
bool radarError();
RadarStage radarStage();
const char* radarStageName();
int radarLastHttp();
uint32_t radarLastTryMs();
