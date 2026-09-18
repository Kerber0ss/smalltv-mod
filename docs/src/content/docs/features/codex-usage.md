---
title: Codex usage meter
description: Show your Codex 5-hour and weekly usage limits on SmallTV, from a Mac.
---

This reuses the existing **Clawdmeter** display mode. With the macOS client running, its title changes to **CODEX** and shows the current five-hour and weekly windows with countdowns to their resets.

The client uses the locally signed-in Codex CLI's App Server. It reads the same rate-limit snapshot used by Codex; it does not read, copy, or send an OAuth token. Only the percentages, reset times, and status are sent to the SmallTV over your LAN.

## Setup

1. Flash this firmware, open **Display → Mode**, and select **Clawdmeter**. Leave **Usage daemon URL** blank because the Mac pushes directly to the device.
2. On the Mac where Codex is signed in, run:

   ```sh
   cd codex-meter
   python3 codex_meter.py --self-test
   python3 codex_meter.py --once --stdout
   python3 codex_meter.py
   ```

3. To keep it running after login, use the `launchd` example in the client README.

The client finds every online SmallTV with macOS Bonjour and polls Codex every 60 seconds. Use `--device <smalltv-ip-or-hostname>` only when Bonjour cannot cross your subnet or VLAN.
