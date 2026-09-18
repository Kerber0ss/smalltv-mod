---
title: Codex usage meter
description: Show your Codex 5-hour and weekly usage limits on SmallTV, from a Mac.
---

This uses the **Usage metrics** display mode. It is a separate **CODEX** screen, with its own latest reading; it no longer overwrites the Claude screen.

The client uses the locally signed-in Codex CLI's App Server. It reads the same rate-limit snapshot used by Codex; it does not read, copy, or send an OAuth token. Only the percentages, reset times, and status are sent to the SmallTV over your LAN.

## Setup

1. Flash this firmware, open **Display → Mode**, and select **Usage metrics**. In **Usage metrics**, enable **Show CODEX metrics**. Leave **Claude daemon URL** blank because the Mac pushes directly to the device. Enable both checkboxes and set **Switch screens every** to alternate Claude and CODEX.
2. On the Mac where Codex is signed in, run:

   ```sh
   cd codex-meter
   python3 codex_meter.py --self-test
   python3 codex_meter.py --once --stdout
   python3 codex_meter.py
   ```

3. To keep it running after login, use the `launchd` example in the client README.

The client finds every online SmallTV with macOS Bonjour and polls Codex every 60 seconds. Use `--device <smalltv-ip-or-hostname>` only when Bonjour cannot cross your subnet or VLAN.
