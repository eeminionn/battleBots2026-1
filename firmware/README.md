# Firmware

Esta carpeta contiene el firmware Arduino para el robot BattleBots 2026-1.

## Sketch principal

- [`battlebot_controller/battlebot_controller.ino`](./battlebot_controller/battlebot_controller.ino)

## Plataforma

- Board: ESP32-S3
- Framework: Arduino
- Interfaz de control: servidor web embebido
- Red: Wi-Fi Access Point

## Librerías requeridas

```cpp
#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
```

## Nota de operación

El sketch crea una red Wi-Fi local para controlar el robot desde un navegador móvil. Las credenciales de operación se definen en el firmware y deben ajustarse según el kit o taller donde se despliegue.
