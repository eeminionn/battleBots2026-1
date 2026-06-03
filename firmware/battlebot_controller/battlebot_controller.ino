// =============================
// ESP32-S3 WIFI MECANUM ROBOT CONTROL
// 2x TB6612FNG + 4 motors + joystick + rotation buttons + servo attack
// =============================
//
// M1 = front left
// M2 = front right
// M3 = rear left
// M4 = rear right
//
// Web:
// /           -> menu
// /movement   -> joystick + rotate left / rotate right
// /attack     -> Attack 1 servo button
//
// Servo:
// GPIO 11
// Attack 1 pressed  -> 120 degrees
// Attack 1 released -> 0 degrees
// =============================

#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// -----------------------------
// WIFI
// -----------------------------

const char* ssid = "Robot_Emilio";
const char* password = "12345678";

WebServer server(80);

// -----------------------------
// MOTOR PINS
// -----------------------------

#define M1_IN1 5
#define M1_IN2 4
#define M1_PWM 6

#define M2_IN1 15
#define M2_IN2 7
#define M2_PWM 16

#define M3_IN1 17
#define M3_IN2 18
#define M3_PWM 38

#define M4_IN1 39
#define M4_IN2 40
#define M4_PWM 41

#define MOTOR_STBY 42

// -----------------------------
// ATTACK SERVO
// -----------------------------

#define SERVO_ATTACK1_PIN 11

Servo servoAttack1;

const int SERVO_REPOSO = 0;
const int SERVO_ATAQUE = 120;

// -----------------------------
// MOVEMENT SETTINGS
// -----------------------------

const int MAX_PWM = 220;
const int MIN_PWM = 55;
const int DEADZONE = 12;

const float LATERAL_GAIN = 1.15;
const float ROTATION_GAIN = 0.85;

unsigned long lastControlCommand = 0;
const unsigned long CONTROL_TIMEOUT = 350;

int joyX = 0;
int joyY = 0;
int rotZ = 0;

// If a motor spins backwards relative to forward, change 1 to -1.
int motorDir[4] = {
  1,  // M1 front left
  1,  // M2 front right
  1,  // M3 rear left
  1   // M4 rear right
};

float motorTrim[4] = {
  1.00,
  1.00,
  1.00,
  1.00
};

// -----------------------------
// MOTOR STRUCTURE
// -----------------------------

struct Motor {
  int in1;
  int in2;
  int pwm;
};

Motor motores[4] = {
  { M1_IN1, M1_IN2, M1_PWM },
  { M2_IN1, M2_IN2, M2_PWM },
  { M3_IN1, M3_IN2, M3_PWM },
  { M4_IN1, M4_IN2, M4_PWM }
};

// -----------------------------
// MOTOR FUNCTIONS
// -----------------------------

void moverMotor(int motor, int velocidad) {
  velocidad = velocidad * motorDir[motor];
  velocidad = velocidad * motorTrim[motor];

  velocidad = constrain(velocidad, -255, 255);

  if (velocidad > 0) {
    digitalWrite(motores[motor].in1, HIGH);
    digitalWrite(motores[motor].in2, LOW);
    analogWrite(motores[motor].pwm, velocidad);
  } else if (velocidad < 0) {
    digitalWrite(motores[motor].in1, LOW);
    digitalWrite(motores[motor].in2, HIGH);
    analogWrite(motores[motor].pwm, abs(velocidad));
  } else {
    digitalWrite(motores[motor].in1, LOW);
    digitalWrite(motores[motor].in2, LOW);
    analogWrite(motores[motor].pwm, 0);
  }
}

void detenerRobot() {
  for (int i = 0; i < 4; i++) {
    moverMotor(i, 0);
  }
}

void configurarMotores() {
  for (int i = 0; i < 4; i++) {
    pinMode(motores[i].in1, OUTPUT);
    pinMode(motores[i].in2, OUTPUT);
    pinMode(motores[i].pwm, OUTPUT);
  }

  pinMode(MOTOR_STBY, OUTPUT);
  digitalWrite(MOTOR_STBY, HIGH);

  detenerRobot();
}

int aplicarPWMMinimo(float valorNormalizado) {
  if (valorNormalizado == 0) {
    return 0;
  }

  int signo = valorNormalizado > 0 ? 1 : -1;
  float magnitud = abs(valorNormalizado);

  int pwm = magnitud * MAX_PWM;

  if (pwm > 0 && pwm < MIN_PWM) {
    pwm = MIN_PWM;
  }

  return signo * constrain(pwm, 0, MAX_PWM);
}

// -----------------------------
// MECANUM / OMNI KINEMATICS
// -----------------------------
//
// M1 = Y + X + R
// M2 = Y - X - R
// M3 = Y - X + R
// M4 = Y + X - R

void moverConControl(int x, int y, int r) {
  if (abs(x) < DEADZONE) x = 0;
  if (abs(y) < DEADZONE) y = 0;
  if (abs(r) < DEADZONE) r = 0;

  if (x == 0 && y == 0 && r == 0) {
    detenerRobot();
    return;
  }

  float xNorm = x / 100.0;
  float yNorm = y / 100.0;
  float rNorm = r / 100.0;

  xNorm = xNorm * LATERAL_GAIN;
  rNorm = rNorm * ROTATION_GAIN;

  float m1 = yNorm + xNorm + rNorm;
  float m2 = yNorm - xNorm - rNorm;
  float m3 = yNorm - xNorm + rNorm;
  float m4 = yNorm + xNorm - rNorm;

  float maxValor = max(max(abs(m1), abs(m2)), max(abs(m3), abs(m4)));

  if (maxValor > 1.0) {
    m1 = m1 / maxValor;
    m2 = m2 / maxValor;
    m3 = m3 / maxValor;
    m4 = m4 / maxValor;
  }

  int pwm1 = aplicarPWMMinimo(m1);
  int pwm2 = aplicarPWMMinimo(m2);
  int pwm3 = aplicarPWMMinimo(m3);
  int pwm4 = aplicarPWMMinimo(m4);

  moverMotor(0, pwm1);
  moverMotor(1, pwm2);
  moverMotor(2, pwm3);
  moverMotor(3, pwm4);

  Serial.print("CTRL | X: ");
  Serial.print(x);
  Serial.print(" | Y: ");
  Serial.print(y);
  Serial.print(" | R: ");
  Serial.println(r);

  Serial.print("PWM | M1: ");
  Serial.print(pwm1);
  Serial.print(" | M2: ");
  Serial.print(pwm2);
  Serial.print(" | M3: ");
  Serial.print(pwm3);
  Serial.print(" | M4: ");
  Serial.println(pwm4);
}

// -----------------------------
// SERVO FUNCTIONS
// -----------------------------

void configurarServoAtaque() {
  servoAttack1.setPeriodHertz(50);
  servoAttack1.attach(SERVO_ATTACK1_PIN, 500, 2400);

  servoAttack1.write(SERVO_REPOSO);

  Serial.println("Servo Attack 1 configured on GPIO 11");
}

void ataque1Activar() {
  servoAttack1.write(SERVO_ATAQUE);

  Serial.print("ATTACK 1 -> Servo to ");
  Serial.print(SERVO_ATAQUE);
  Serial.println(" degrees");
}

void ataque1Detener() {
  servoAttack1.write(SERVO_REPOSO);

  Serial.print("ATTACK 1 STOP -> Servo to ");
  Serial.print(SERVO_REPOSO);
  Serial.println(" degrees");
}

// -----------------------------
// MENU PAGE
// -----------------------------

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>Robot Control</title>

  <style>
    body {
      margin: 0;
      background: #111;
      color: white;
      font-family: Arial, sans-serif;
      text-align: center;
      user-select: none;
    }

    .container {
      padding: 40px 20px;
    }

    h1 {
      margin-top: 40px;
      font-size: 30px;
    }

    h2 {
      margin-top: 10px;
      font-size: 22px;
      color: #ccc;
      font-weight: normal;
    }

    .menu-button {
      display: block;
      width: 85%;
      max-width: 320px;
      height: 80px;
      margin: 25px auto;
      font-size: 26px;
      border: none;
      border-radius: 18px;
      background: #2d89ef;
      color: white;
      font-weight: bold;
    }

    .attack {
      background: #e74c3c;
    }

    .footer {
      margin-top: 40px;
      color: #777;
      font-size: 14px;
    }
  </style>
</head>

<body>
  <div class="container">
    <h1>Robot Control</h1>
    <h2>What controls do you want?</h2>

    <button class="menu-button" onclick="location.href='/movement'">
      Movement
    </button>

    <button class="menu-button attack" onclick="location.href='/attack'">
      Attack
    </button>

    <div class="footer">
      ESP32-S3 Wi-Fi Robot Interface
    </div>
  </div>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}

// -----------------------------
// MOVEMENT PAGE
// -----------------------------

void handleMovementPage() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>Movement Control</title>

  <style>
    body {
      margin: 0;
      background: #111;
      color: white;
      font-family: Arial, sans-serif;
      text-align: center;
      overflow: hidden;
      user-select: none;
    }

    h2 {
      margin-top: 25px;
    }

    .back-button {
      position: absolute;
      top: 15px;
      left: 15px;
      background: #444;
      color: white;
      border: none;
      border-radius: 10px;
      padding: 10px 14px;
      font-size: 16px;
    }

    #joystick {
      width: 220px;
      height: 220px;
      background: #333;
      border-radius: 50%;
      margin: 50px auto 15px auto;
      position: relative;
      touch-action: none;
      border: 4px solid #666;
    }

    #knob {
      width: 84px;
      height: 84px;
      background: #ddd;
      border-radius: 50%;
      position: absolute;
      left: 68px;
      top: 68px;
      touch-action: none;
    }

    #values {
      font-size: 20px;
      margin-top: 15px;
    }

    #status {
      font-size: 17px;
      color: #aaa;
      margin-top: 8px;
    }

    .rotate-container {
      display: flex;
      justify-content: center;
      gap: 18px;
      margin-top: 25px;
    }

    .rotate-button {
      width: 145px;
      height: 65px;
      font-size: 20px;
      border: none;
      border-radius: 16px;
      background: #f39c12;
      color: white;
      font-weight: bold;
      touch-action: none;
    }

    .rotate-button:active {
      background: #d68910;
      transform: scale(0.96);
    }
  </style>
</head>

<body>
  <button class="back-button" onclick="location.href='/'">Menu</button>

  <h2>Movement Control</h2>

  <div id="joystick">
    <div id="knob"></div>
  </div>

  <div id="values">
    X: <span id="xVal">0</span> |
    Y: <span id="yVal">0</span> |
    R: <span id="rVal">0</span>
  </div>

  <div id="status">Robot stopped</div>

  <div class="rotate-container">
    <button id="rotateLeft" class="rotate-button">Rotate Left</button>
    <button id="rotateRight" class="rotate-button">Rotate Right</button>
  </div>

  <script>
    const joystick = document.getElementById("joystick");
    const knob = document.getElementById("knob");
    const xVal = document.getElementById("xVal");
    const yVal = document.getElementById("yVal");
    const rVal = document.getElementById("rVal");
    const statusText = document.getElementById("status");

    const rotateLeft = document.getElementById("rotateLeft");
    const rotateRight = document.getElementById("rotateRight");

    let active = false;
    let currentX = 0;
    let currentY = 0;
    let currentR = 0;

    let lastJoySend = 0;
    let lastRotSend = 0;
    let rotationInterval = null;

    const joystickSize = 220;
    const knobSize = 84;
    const knobCenter = (joystickSize - knobSize) / 2;
    const maxDistance = knobCenter;

    function updateStatus() {
      if (currentX === 0 && currentY === 0 && currentR === 0) {
        statusText.textContent = "Robot stopped";
      } else if (currentR > 0) {
        statusText.textContent = "Rotating right";
      } else if (currentR < 0) {
        statusText.textContent = "Rotating left";
      } else {
        statusText.textContent = "Moving robot";
      }
    }

    function sendJoystick(force = false) {
      const now = Date.now();

      if (!force && now - lastJoySend < 70) {
        return;
      }

      lastJoySend = now;

      fetch("/joy?x=" + currentX + "&y=" + currentY)
        .catch(error => {
          console.log("Joystick send error:", error);
        });
    }

    function sendRotation(force = false) {
      const now = Date.now();

      if (!force && now - lastRotSend < 70) {
        return;
      }

      lastRotSend = now;

      fetch("/rot?r=" + currentR)
        .catch(error => {
          console.log("Rotation send error:", error);
        });
    }

    function updateJoystick(clientX, clientY) {
      const rect = joystick.getBoundingClientRect();

      const centerX = rect.left + joystickSize / 2;
      const centerY = rect.top + joystickSize / 2;

      let dx = clientX - centerX;
      let dy = clientY - centerY;

      const distance = Math.sqrt(dx * dx + dy * dy);

      if (distance > maxDistance) {
        dx = dx / distance * maxDistance;
        dy = dy / distance * maxDistance;
      }

      knob.style.left = (knobCenter + dx) + "px";
      knob.style.top = (knobCenter + dy) + "px";

      currentX = Math.round((dx / maxDistance) * 100);
      currentY = Math.round((-dy / maxDistance) * 100);

      xVal.textContent = currentX;
      yVal.textContent = currentY;

      updateStatus();
      sendJoystick();
    }

    function centerJoystick() {
      currentX = 0;
      currentY = 0;

      knob.style.left = knobCenter + "px";
      knob.style.top = knobCenter + "px";

      xVal.textContent = "0";
      yVal.textContent = "0";

      updateStatus();
      sendJoystick(true);
    }

    function startRotation(value) {
      currentR = value;
      rVal.textContent = currentR;

      updateStatus();
      sendRotation(true);

      if (rotationInterval !== null) {
        clearInterval(rotationInterval);
      }

      rotationInterval = setInterval(function() {
        sendRotation();
      }, 90);
    }

    function stopRotation() {
      currentR = 0;
      rVal.textContent = "0";

      updateStatus();
      sendRotation(true);

      if (rotationInterval !== null) {
        clearInterval(rotationInterval);
        rotationInterval = null;
      }
    }

    joystick.addEventListener("pointerdown", function(event) {
      active = true;
      joystick.setPointerCapture(event.pointerId);
      updateJoystick(event.clientX, event.clientY);
    });

    joystick.addEventListener("pointermove", function(event) {
      if (active) {
        updateJoystick(event.clientX, event.clientY);
      }
    });

    joystick.addEventListener("pointerup", function(event) {
      active = false;
      centerJoystick();
    });

    joystick.addEventListener("pointercancel", function(event) {
      active = false;
      centerJoystick();
    });

    joystick.addEventListener("lostpointercapture", function(event) {
      active = false;
      centerJoystick();
    });

    setInterval(function() {
      if (active) {
        sendJoystick();
      }
    }, 90);

    rotateLeft.addEventListener("pointerdown", function(event) {
      event.preventDefault();
      startRotation(-100);
    });

    rotateLeft.addEventListener("pointerup", function(event) {
      event.preventDefault();
      stopRotation();
    });

    rotateLeft.addEventListener("pointercancel", function(event) {
      event.preventDefault();
      stopRotation();
    });

    rotateLeft.addEventListener("lostpointercapture", function(event) {
      stopRotation();
    });

    rotateRight.addEventListener("pointerdown", function(event) {
      event.preventDefault();
      startRotation(100);
    });

    rotateRight.addEventListener("pointerup", function(event) {
      event.preventDefault();
      stopRotation();
    });

    rotateRight.addEventListener("pointercancel", function(event) {
      event.preventDefault();
      stopRotation();
    });

    rotateRight.addEventListener("lostpointercapture", function(event) {
      stopRotation();
    });
  </script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}

// -----------------------------
// ATTACK PAGE
// -----------------------------

void handleAttackPage() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>Attack Control</title>

  <style>
    body {
      margin: 0;
      background: #111;
      color: white;
      font-family: Arial, sans-serif;
      text-align: center;
      user-select: none;
      touch-action: none;
    }

    h2 {
      margin-top: 25px;
    }

    .back-button {
      position: absolute;
      top: 15px;
      left: 15px;
      background: #444;
      color: white;
      border: none;
      border-radius: 10px;
      padding: 10px 14px;
      font-size: 16px;
    }

    .attack-button {
      display: block;
      width: 85%;
      max-width: 340px;
      height: 90px;
      margin: 90px auto 25px auto;
      font-size: 28px;
      border: none;
      border-radius: 18px;
      background: #e74c3c;
      color: white;
      font-weight: bold;
      touch-action: none;
    }

    .stop-button {
      display: block;
      width: 85%;
      max-width: 340px;
      height: 70px;
      margin: 20px auto;
      font-size: 22px;
      border: none;
      border-radius: 18px;
      background: #555;
      color: white;
      font-weight: bold;
      touch-action: none;
    }

    #status {
      margin-top: 25px;
      font-size: 20px;
      color: #aaa;
    }
  </style>
</head>

<body>
  <button class="back-button" onclick="location.href='/'">Menu</button>

  <h2>Attack Controls</h2>

  <button id="attack1" class="attack-button">
    Attack 1
  </button>

  <button id="stopAttack" class="stop-button">
    Stop Attack
  </button>

  <div id="status">Servo ready</div>

  <script>
    const attack1 = document.getElementById("attack1");
    const stopAttack = document.getElementById("stopAttack");
    const statusText = document.getElementById("status");

    function sendAttack(command) {
      statusText.textContent = "Sending: " + command;

      fetch("/atk?cmd=" + command)
        .catch(error => {
          console.log("Attack send error:", error);
        });
    }

    attack1.addEventListener("pointerdown", function(event) {
      event.preventDefault();
      attack1.setPointerCapture(event.pointerId);
      sendAttack("A1_START");
    });

    attack1.addEventListener("pointerup", function(event) {
      event.preventDefault();
      sendAttack("A1_STOP");
    });

    attack1.addEventListener("pointercancel", function(event) {
      event.preventDefault();
      sendAttack("A1_STOP");
    });

    attack1.addEventListener("lostpointercapture", function(event) {
      sendAttack("A1_STOP");
    });

    stopAttack.addEventListener("pointerdown", function(event) {
      event.preventDefault();
      sendAttack("ASTOP");
    });
  </script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}

// -----------------------------
// RECEIVE JOYSTICK
// -----------------------------

void handleJoystick() {
  if (server.hasArg("x") && server.hasArg("y")) {
    joyX = constrain(server.arg("x").toInt(), -100, 100);
    joyY = constrain(server.arg("y").toInt(), -100, 100);

    lastControlCommand = millis();

    moverConControl(joyX, joyY, rotZ);

    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing x or y");
  }
}

// -----------------------------
// RECEIVE ROTATION
// -----------------------------

void handleRotation() {
  if (server.hasArg("r")) {
    rotZ = constrain(server.arg("r").toInt(), -100, 100);

    lastControlCommand = millis();

    moverConControl(joyX, joyY, rotZ);

    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing r");
  }
}

// -----------------------------
// RECEIVE ATTACK
// -----------------------------

void handleAttackCommand() {
  if (server.hasArg("cmd")) {
    String cmd = server.arg("cmd");

    Serial.print("ATTACK COMMAND: ");
    Serial.println(cmd);

    if (cmd == "A1_START") {
      ataque1Activar();
    } else if (cmd == "A1_STOP") {
      ataque1Detener();
    } else if (cmd == "ASTOP") {
      ataque1Detener();
    }

    server.send(200, "text/plain", "Attack command received: " + cmd);
  } else {
    server.send(400, "text/plain", "Missing attack command");
  }
}

// -----------------------------
// SETUP
// -----------------------------

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Starting ESP32-S3 Wi-Fi mecanum robot control...");

  configurarMotores();
  configurarServoAtaque();

  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();

  Serial.print("Wi-Fi name: ");
  Serial.println(ssid);

  Serial.print("Password: ");
  Serial.println(password);

  Serial.print("Open in phone browser: ");
  Serial.println(IP);

  server.on("/", handleRoot);
  server.on("/movement", handleMovementPage);
  server.on("/attack", handleAttackPage);
  server.on("/joy", handleJoystick);
  server.on("/rot", handleRotation);
  server.on("/atk", handleAttackCommand);

  server.begin();

  Serial.println("Web server started");
  Serial.println("Robot ready.");
}

// -----------------------------
// LOOP
// -----------------------------

void loop() {
  server.handleClient();

  if ((joyX != 0 || joyY != 0 || rotZ != 0) && millis() - lastControlCommand > CONTROL_TIMEOUT) {
    joyX = 0;
    joyY = 0;
    rotZ = 0;

    detenerRobot();

    Serial.println("CONTROL TIMEOUT -> ROBOT STOPPED");
  }
}
