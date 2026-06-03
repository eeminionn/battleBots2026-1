# Pinout

Referencia de pines para el robot BattleBots 2026-1.

## Motores

| Motor | Posición | Driver | IN1 | IN2 | PWM |
| --- | --- | --- | ---: | ---: | ---: |
| M1 | Adelante izquierda | TB6612FNG #1 | GPIO 5 | GPIO 4 | GPIO 6 |
| M2 | Adelante derecha | TB6612FNG #1 | GPIO 15 | GPIO 7 | GPIO 16 |
| M3 | Atrás izquierda | TB6612FNG #2 | GPIO 17 | GPIO 18 | GPIO 38 |
| M4 | Atrás derecha | TB6612FNG #2 | GPIO 39 | GPIO 40 | GPIO 41 |

## Driver standby

| Señal | GPIO | Función |
| --- | ---: | --- |
| `MOTOR_STBY` | 42 | Standby compartido para ambos TB6612FNG |

## Servo

| Señal | GPIO | Función |
| --- | ---: | --- |
| `SERVO_ATTACK1_PIN` | 11 | Señal del servo principal de ataque |

## Recomendación eléctrica

- No alimentar el servo desde el pin 3.3V de la ESP32-S3.
- Usar una fuente o riel estable de 5V para el servo.
- Compartir GND entre ESP32-S3, TB6612FNG, fuente de motores y fuente del servo.
