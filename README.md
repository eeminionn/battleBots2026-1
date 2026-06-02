# battleBots2026-1

Proyecto desarrollado por el equipo **EIRI (Equipo Interdisciplinario de Robótica e Innovación)** en conjunto con **Ingeniería Civil Informática** para el ciclo **2026-1**.

El objetivo del proyecto es diseñar e implementar un set de **6 clases/talleres** orientados a estudiantes de **4to medio**, donde los participantes construyen y programan robots tipo **BattleBots / Boombot**, pensados para competir mediante un mecanismo de ataque que permite explotar globos.

Como parte del desarrollo del ciclo, el equipo preparó la planificación de clases, desarrolló los contenidos técnicos y diseñó una **placa electrónica PCB** para facilitar el armado, conexión y uso de los robots durante los talleres.

## Objetivo del Proyecto

El proyecto busca acercar a estudiantes de enseñanza media a la robótica aplicada mediante una experiencia práctica, competitiva y guiada. Durante el ciclo, los estudiantes trabajan con conceptos de electrónica, programación, control de motores, diseño mecánico y resolución de problemas.

El robot base corresponde a un vehículo omnidireccional/mecanum controlado por Wi-Fi desde smartphones. La placa principal utilizada es una **ESP32-S3**, que crea su propia red Wi-Fi en modo Access Point y entrega una interfaz web para controlar movimiento y ataque desde el navegador de un celular.

## Contexto Educativo

- Institución/equipo responsable: EIRI e Ingeniería Civil Informática.
- Ciclo: 2026-1.
- Público objetivo: estudiantes de 4to medio.
- Formato: set de 6 clases prácticas.
- Desafío final: construcción y control de robots BattleBots tipo Boombot.
- Enfoque de competencia: robots con mecanismo de ataque para explotar globos.
- Apoyo de hardware: PCB diseñada para los talleres.

## Descripción Técnica General

El robot utiliza una ESP32-S3 como controlador principal. La ESP32-S3 levanta una red Wi-Fi propia, sirve una interfaz web local y recibe comandos HTTP desde uno o más celulares conectados a esa red.

Desde la interfaz web se contemplan dos modos principales:

- **Movement**: control del desplazamiento mediante joystick virtual y botones de rotación.
- **Attack**: control de mecanismos de ataque, inicialmente mediante un servomotor.

La versión actual evita el uso de Bluetooth clásico y se basa en Wi-Fi porque resulta más compatible con smartphones modernos y permite controlar el robot desde cualquier navegador web.

## Hardware Principal

- Microcontrolador: ESP32-S3.
- Movimiento: 4 motores DC.
- Ruedas: omnidireccionales/mecanum.
- Drivers de motores: 2x TB6612FNG.
- Mecanismo de ataque: servomotor de 180 grados.
- Servo de ataque: GPIO 11.
- Placa electrónica: PCB diseñada para el desarrollo de los talleres.

## Librerías Utilizadas

El proyecto está pensado para Arduino Framework / Arduino IDE y utiliza las siguientes librerías:

```cpp
#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
```

- `WiFi.h`: permite que la ESP32-S3 cree una red Wi-Fi propia en modo Access Point.
- `WebServer.h`: levanta un servidor HTTP local para servir la interfaz web y recibir comandos.
- `ESP32Servo.h`: controla el servomotor asociado al mecanismo de ataque.

## Configuración Wi-Fi

La ESP32-S3 debe crear una red Wi-Fi propia para que los estudiantes puedan conectarse directamente al robot desde sus celulares.

- SSID: `Robot_Emilio`
- Password: `12345678`
- IP esperada: `192.168.4.1`

Flujo de uso:

1. Encender la ESP32-S3.
2. Conectarse desde el celular a la red `Robot_Emilio`.
3. Abrir el navegador.
4. Entrar a `http://192.168.4.1`.
5. Elegir entre `Movement` o `Attack`.

## Interfaz Web

El servidor web de la ESP32-S3 contempla las siguientes rutas:

### `/`

Página principal del robot.

Debe mostrar:

- Título: `Robot Control`.
- Pregunta: `What controls do you want?`.
- Botón `Movement`.
- Botón `Attack`.

### `/movement`

Página de control de movimiento.

Debe mostrar:

- Joystick virtual.
- Valores visibles `X`, `Y` y `R`.
- Botón `Rotate Left`.
- Botón `Rotate Right`.
- Botón `Menu` para volver al inicio.

El joystick debe usar `pointer events` para ser compatible con control táctil desde celular:

- `pointerdown`: activa el control.
- `pointermove`: actualiza los valores X/Y.
- `pointerup`: vuelve al centro.
- `pointercancel`: vuelve al centro.
- `lostpointercapture`: vuelve al centro.

El joystick envía valores entre `-100` y `100`:

- X positivo: desplazamiento lateral a la derecha.
- X negativo: desplazamiento lateral a la izquierda.
- Y positivo: avance.
- Y negativo: retroceso.
- X = 0 e Y = 0: detener desplazamiento.

Los botones de rotación funcionan como controles de mantener presionado:

- `Rotate Left`: envía `R = -100` mientras se mantiene presionado.
- `Rotate Right`: envía `R = 100` mientras se mantiene presionado.
- Al soltar cualquier botón de rotación, se envía `R = 0`.

### `/attack`

Página de control de ataque.

Debe mostrar:

- Botón `Attack 1`.
- Botón `Stop Attack`.
- Botón `Menu`.

Comportamiento esperado:

- Al presionar `Attack 1`, enviar `A1_START`.
- Al soltar `Attack 1`, enviar `A1_STOP`.
- `Stop Attack` debe enviar `ASTOP`.

## Endpoints del Servidor

### `/joy`

Recibe los valores del joystick:

```text
GET /joy?x=50&y=80
```

Parámetros:

- `x`: valor de joystick en X, entre `-100` y `100`.
- `y`: valor de joystick en Y, entre `-100` y `100`.

Al recibir datos, el robot debe actualizar `joyX`, `joyY`, `lastControlCommand`, ejecutar `moverConControl(joyX, joyY, rotZ)` y responder `OK`.

### `/rot`

Recibe el valor de rotación:

```text
GET /rot?r=100
```

Parámetros:

- `r`: valor de rotación, entre `-100` y `100`.

Al recibir datos, el robot debe actualizar `rotZ`, `lastControlCommand`, ejecutar `moverConControl(joyX, joyY, rotZ)` y responder `OK`.

### `/atk`

Recibe comandos de ataque:

```text
GET /atk?cmd=A1_START
```

Comandos esperados:

- `A1_START`: mueve el servo Attack 1 a 120 grados.
- `A1_STOP`: mueve el servo Attack 1 a posición de reposo.
- `ASTOP`: mueve el servo Attack 1 a posición de reposo.

## Pinout de Motores

Distribución de motores:

- M1: adelante izquierda.
- M2: adelante derecha.
- M3: atrás izquierda.
- M4: atrás derecha.

| Motor | Driver | IN1 | IN2 | PWM |
| --- | --- | --- | --- | --- |
| M1 adelante izquierda | TB6612 #1 | GPIO 5 | GPIO 4 | GPIO 6 |
| M2 adelante derecha | TB6612 #1 | GPIO 15 | GPIO 7 | GPIO 16 |
| M3 atrás izquierda | TB6612 #2 | GPIO 17 | GPIO 18 | GPIO 38 |
| M4 atrás derecha | TB6612 #2 | GPIO 39 | GPIO 40 | GPIO 41 |

STBY compartido:

```cpp
MOTOR_STBY = GPIO 42
```

Estos pines no deben cambiarse salvo que se indique explícitamente.

## Configuración de Movimiento

Valores de referencia utilizados:

```cpp
MAX_PWM = 220
MIN_PWM = 55
DEADZONE = 12
LATERAL_GAIN = 1.15
ROTATION_GAIN = 0.85
CONTROL_TIMEOUT = 350 ms
```

- `MAX_PWM`: límite superior de velocidad.
- `MIN_PWM`: PWM mínimo para vencer fricción.
- `DEADZONE`: zona muerta del joystick.
- `LATERAL_GAIN`: compensa la mayor fuerza requerida por el movimiento lateral.
- `ROTATION_GAIN`: reduce la agresividad del giro.
- `CONTROL_TIMEOUT`: detiene el robot si no llegan comandos por más de 350 ms.

## Cinemática Mecanum

El robot utiliza ruedas mecanum/omnidireccionales en orientación tipo X vista desde arriba:

```text
Frente del robot

I - D
D - I
```

La fórmula de movimiento validada es:

```text
M1 = Y + X + R
M2 = Y - X - R
M3 = Y - X + R
M4 = Y + X - R
```

Donde:

- `X`: desplazamiento lateral.
- `Y`: avance o retroceso.
- `R`: rotación.

Esta fórmula permite avanzar, retroceder, desplazarse lateralmente, rotar sobre el propio eje y combinar movimientos.

## Servo de Ataque

El mecanismo `Attack 1` utiliza un servomotor de 180 grados conectado a GPIO 11.

```cpp
SERVO_ATTACK1_PIN = 11
SERVO_REPOSO = 0
SERVO_ATAQUE = 120
```

Comportamiento esperado:

- `Attack 1` presionado: servo a 120 grados.
- `Attack 1` soltado: servo a 0 grados.
- `Stop Attack`: servo a 0 grados.

Conexión recomendada:

- Señal del servo: GPIO 11.
- VCC del servo: fuente externa de 5V estable.
- GND del servo: GND común con ESP32, drivers y fuentes.

El servo no debe alimentarse desde el pin 3.3V de la ESP32.

## Seguridad

El robot debe detenerse automáticamente si deja de recibir comandos del control web. Esto evita que siga moviéndose si se desconecta el celular, se cierra el navegador, se pierde Wi-Fi o se corta el envío de datos.

Variables relevantes:

```cpp
joyX
joyY
rotZ
lastControlCommand
CONTROL_TIMEOUT
```

Lógica esperada en `loop()`:

```cpp
server.handleClient();

if ((joyX != 0 || joyY != 0 || rotZ != 0) && millis() - lastControlCommand > CONTROL_TIMEOUT) {
  joyX = 0;
  joyY = 0;
  rotZ = 0;
  detenerRobot();
}
```

## Reglas Técnicas Importantes

- No cambiar el pinout de motores sin pedirlo.
- No cambiar el SSID ni la contraseña sin pedirlo.
- Mantener la ESP32-S3 en modo Wi-Fi Access Point.
- No depender de un router externo.
- Mantener interfaz compatible con celular.
- Mantener uso de `pointer events` para control táctil.
- Mantener timeout de seguridad.
- Mantener `Attack 1` en GPIO 11.
- Mantener el servo a 120 grados en ataque.
- Mantener el servo a 0 grados al soltar.
- Mantener la fórmula mecanum validada.

## Estructura Pedagógica del Ciclo

El ciclo contempla 6 clases orientadas al desarrollo progresivo del robot. La estructura exacta puede ajustarse según la planificación del equipo, pero el README deja registrada la intención general del proyecto:

1. Introducción a robótica, seguridad y desafío BattleBots/Boombot.
2. Reconocimiento de componentes electrónicos y mecánicos.
3. Armado de estructura, motores, ruedas y conexiones principales.
4. Programación de ESP32-S3, Wi-Fi AP e interfaz web.
5. Integración de movimiento mecanum y mecanismo de ataque.
6. Pruebas, calibración, competencia y cierre del ciclo.

## Mejoras Futuras

Estas mejoras quedan como ideas posibles y no deben implementarse sin decisión previa del equipo:

- Segundo joystick para rotación.
- Slider de velocidad máxima.
- Botón de emergencia `STOP` visible en todas las páginas.
- Indicador visual de conexión.
- Separación del HTML en archivos o estructuras más ordenadas.
- Más servos o mecanismos de ataque.
- Control PID o suavizado de aceleración.
- Uso de WebSocket para menor latencia.
- Modo de calibración de motores desde la web.
- Guardado de ajustes en memoria no volátil.

## Estado del Proyecto

El proyecto se encuentra en desarrollo para el ciclo 2026-1. La versión actual esperada integra:

- Red Wi-Fi propia creada por la ESP32-S3.
- Interfaz web con modos `Movement` y `Attack`.
- Joystick virtual para movimiento X/Y.
- Botones de rotación izquierda/derecha.
- Control de 4 motores DC mediante 2 TB6612FNG.
- Cinemática mecanum validada.
- Servo de ataque en GPIO 11.
- Timeout de seguridad para detener el robot.
- Diseño de PCB para facilitar los talleres.

## Comandos Git Sugeridos

Después de modificar el proyecto, se recomienda registrar los cambios con:

```bash
git add .
git commit -m "Agrega README inicial del proyecto BattleBots 2026-1"
git push
```
