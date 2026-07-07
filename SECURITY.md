# Security Policy

## Secrets

This project uses a local `secrets.h` file for:

- WiFi SSID
- WiFi password
- ThingsBoard host
- ThingsBoard device token

`secrets.h` must never be committed to GitHub.

Use `secrets.example.h` as the public template.

## If a token is exposed

If a ThingsBoard token is accidentally exposed:

1. Regenerate the device token in ThingsBoard.
2. Update local `secrets.h`.
3. Re-upload firmware to the ESP32.
4. Check GitHub history and remove the leaked secret if needed.

## Repository Rule

Only commit:

```text
HP12.ino
config.h
secrets.example.h
README.md
CHANGELOG.md
SECURITY.md
.gitignore
