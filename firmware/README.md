# Firmware

Firmware Arduino para el robot BattleBots 2026-1.

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

## Configuracion de red

El sketch crea una red Wi-Fi local y expone una interfaz web para control desde
navegador movil. Para usar credenciales propias, copia
`battlebot_controller/robot_config.example.h` como
`battlebot_controller/robot_config.h` y cambia sus valores. El archivo local
esta ignorado por Git.

La clave debe tener al menos ocho caracteres. Si la configuracion es invalida o
el Access Point no puede iniciarse, el firmware deja el driver de motores en
standby.

Los comandos de ataque admitidos son `A1_START`, `A1_STOP` y `ASTOP`. Cualquier
otro valor responde `400 Bad Request`.
