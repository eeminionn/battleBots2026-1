# BattleBots firmware instructions

- Target board: ESP32-S3 using Arduino. Preserve the documented motor safety behavior: invalid network configuration or access-point startup failure must leave the motor driver in standby.
- Keep `firmware/battlebot_controller/robot_config.h` local and ignored. Only change `robot_config.example.h` with placeholders and never commit Wi-Fi passwords or private control endpoints.
- Treat remote control commands as untrusted input. Keep command validation explicit, bounded, and fail-safe.
- Document any pinout, library, or wiring change in `docs/pinout.md` or the firmware README, and validate compilation for the ESP32-S3 target.
