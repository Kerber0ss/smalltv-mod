# SmallTV Codex meter (macOS)

`codex_meter.py` reads the signed-in local Codex account through `codex app-server` and POSTs the five-hour and weekly usage windows to one or more SmallTV devices. It uses only Python's standard library. The device receives percentages, reset countdowns, and a status; no Codex token leaves the Mac.

1. Flash this firmware and select **Display → Mode → Clawdmeter** on the SmallTV. Leave **Usage daemon URL** empty: this client pushes directly to the device.
2. Confirm the local conversion without sending anything:

   ```sh
   python3 codex_meter.py --self-test
   python3 codex_meter.py --once --stdout
   ```

3. Start the meter. It finds every online SmallTV automatically with Bonjour:

   ```sh
   python3 codex_meter.py
   ```

It polls Codex every 60 seconds. `--device 192.168.1.50` remains available when the device is on another subnet or Bonjour is blocked. Use `--once` for one update.

## Run after login

Save this as `~/Library/LaunchAgents/com.smalltv.codex-meter.plist`, replacing both placeholders with absolute paths. Then load it with `launchctl bootstrap gui/$(id -u) ~/Library/LaunchAgents/com.smalltv.codex-meter.plist`.

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0"><dict>
  <key>Label</key><string>com.smalltv.codex-meter</string>
  <key>ProgramArguments</key><array>
    <string>/usr/bin/python3</string>
    <string>ABSOLUTE_PATH_TO/codex_meter.py</string>
  </array>
  <key>RunAtLoad</key><true/>
  <key>KeepAlive</key><true/>
  <key>StandardOutPath</key><string>/tmp/smalltv-codex-meter.log</string>
  <key>StandardErrorPath</key><string>/tmp/smalltv-codex-meter.error.log</string>
</dict></plist>
```
