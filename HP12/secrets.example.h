#pragma once

/*
  HP12 - SECRETS EXAMPLE

  Copy file này thành secrets.h
  Sau đó điền WiFi và ThingsBoard thật trong secrets.h.

  QUAN TRỌNG:
  - secrets.h chỉ nằm trên máy cá nhân.
  - Không đưa secrets.h lên GitHub.
  - Chỉ đưa secrets.example.h lên GitHub.
*/

const char* WIFI_SSID     = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const char* TB_HOST       = "thingsboard.cloud";
const int   TB_PORT       = 1883;
const char* TB_TOKEN      = "YOUR_THINGSBOARD_DEVICE_ACCESS_TOKEN";
