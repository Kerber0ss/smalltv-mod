#!/usr/bin/env python3
"""Send the signed-in macOS Codex account's rolling limits to SmallTV."""

import argparse
import json
import math
import re
import select
import subprocess
import sys
import time
import urllib.error
import urllib.request


class MeterError(RuntimeError):
    pass


SERVICE_TYPE = "_clawdmeter._tcp"


def run_bonjour(command, timeout):
    try:
        process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL,
                                   text=True)
    except OSError as error:
        raise MeterError("Cannot start macOS Bonjour: %s" % error) from error
    try:
        output, _ = process.communicate(timeout=timeout)
    except subprocess.TimeoutExpired:
        process.terminate()
        output, _ = process.communicate(timeout=2)
    return output


def browse_services(output):
    services = []
    for line in output.splitlines():
        match = re.search(r"\sAdd\s+\d+\s+\d+\s+(\S+)\s+_clawdmeter\._tcp\.\s+(.+?)\s*$", line)
        if match:
            services.append((match.group(2), match.group(1)))
    return services


def resolve_service(output):
    match = re.search(r"can be reached at ([^:]+):(\d+)", output)
    if not match:
        return None
    return "%s:%s" % (match.group(1).rstrip("."), match.group(2))


def discover_devices(timeout):
    output = run_bonjour(["dns-sd", "-B", SERVICE_TYPE, "local."], timeout)
    devices = []
    for name, domain in browse_services(output):
        resolved = run_bonjour(["dns-sd", "-L", name, SERVICE_TYPE, domain], timeout)
        device = resolve_service(resolved)
        if device and device not in devices:
            devices.append(device)
    return devices


def minutes_until(timestamp, now):
    if not isinstance(timestamp, (int, float)):
        return 0
    return max(0, math.ceil((timestamp - now) / 60))


def percent(window):
    value = window.get("usedPercent") if isinstance(window, dict) else None
    if not isinstance(value, (int, float)):
        raise MeterError("Codex did not return a usage percentage")
    return max(0, min(100, value))


def codex_bucket(snapshot):
    buckets = snapshot.get("rateLimitsByLimitId")
    if isinstance(buckets, dict):
        bucket = buckets.get("codex")
        if isinstance(bucket, dict):
            return bucket
        if len(buckets) == 1:
            return next(iter(buckets.values()))
    bucket = snapshot.get("rateLimits")
    if isinstance(bucket, dict):
        return bucket
    raise MeterError("Codex did not return a Codex rate-limit bucket")


def usage_payload(snapshot, now=None):
    """Convert Codex App Server's snapshot into SmallTV's existing contract."""
    now = time.time() if now is None else now
    bucket = codex_bucket(snapshot)
    primary = bucket.get("primary") or {}
    secondary = bucket.get("secondary") or {}
    session = percent(primary)
    weekly = percent(secondary) if secondary else 0
    status = "rejected" if snapshot.get("ordinaryUsageAllowed") is False else "normal"
    if status == "normal" and max(session, weekly) >= 75:
        status = "warning"
    return {
        "s": session,
        "sr": minutes_until(primary.get("resetsAt"), now),
        "w": weekly,
        "wr": minutes_until(secondary.get("resetsAt"), now),
        "st": status,
        "src": "codex",
        "ok": True,
    }


class CodexAppServer:
    def __init__(self, executable, timeout):
        self.executable = executable
        self.timeout = timeout
        self.process = None

    def __enter__(self):
        try:
            self.process = subprocess.Popen(
                [self.executable, "app-server"], stdin=subprocess.PIPE,
                stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, bufsize=1,
            )
        except OSError as error:
            raise MeterError("Cannot start Codex App Server: %s" % error) from error
        return self

    def __exit__(self, *_):
        if not self.process:
            return
        self.process.terminate()
        try:
            self.process.wait(timeout=2)
        except subprocess.TimeoutExpired:
            self.process.kill()
            self.process.wait(timeout=2)

    def send(self, message):
        self.process.stdin.write(json.dumps(message) + "\n")
        self.process.stdin.flush()

    def receive(self, request_id):
        deadline = time.monotonic() + self.timeout
        while True:
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                raise MeterError("Timed out waiting for Codex App Server")
            ready, _, _ = select.select([self.process.stdout], [], [], remaining)
            if not ready:
                continue
            line = self.process.stdout.readline()
            if not line:
                detail = self.process.stderr.read().strip()
                raise MeterError("Codex App Server stopped%s" % (": " + detail if detail else ""))
            try:
                response = json.loads(line)
            except json.JSONDecodeError:
                continue
            if response.get("id") != request_id:
                continue
            if "error" in response:
                raise MeterError("Codex App Server: %s" % response["error"])
            return response.get("result")

    def rate_limits(self):
        self.send({
            "jsonrpc": "2.0", "id": 1, "method": "initialize",
            "params": {"clientInfo": {"name": "smalltv-codex-meter", "version": "0.1.0"},
                       "capabilities": {}},
        })
        self.receive(1)
        self.send({"jsonrpc": "2.0", "method": "initialized"})
        self.send({
            "jsonrpc": "2.0", "id": 2, "method": "account/rateLimits/read",
            "params": {"excludeResetCreditDetails": True},
        })
        return self.receive(2)


def post_usage(device, payload, timeout):
    url = device.rstrip("/")
    if not url.startswith(("http://", "https://")):
        url = "http://" + url
    if not url.endswith("/api/usage"):
        url += "/api/usage"
    request = urllib.request.Request(
        url, data=json.dumps(payload).encode(),
        headers={"Content-Type": "application/json", "Accept": "application/json"}, method="POST",
    )
    try:
        with urllib.request.urlopen(request, timeout=timeout) as response:
            if not 200 <= response.status < 300:
                raise MeterError("SmallTV returned HTTP %s" % response.status)
    except urllib.error.URLError as error:
        raise MeterError("Cannot send to %s: %s" % (url, error.reason)) from error


def self_test():
    payload = usage_payload({
        "ordinaryUsageAllowed": True,
        "rateLimitsByLimitId": {"codex": {
            "primary": {"usedPercent": 29, "resetsAt": 1_000_120},
            "secondary": {"usedPercent": 81, "resetsAt": 1_604_800},
        }},
    }, now=1_000_000)
    assert payload == {"s": 29, "sr": 2, "w": 81, "wr": 10080,
                       "st": "warning", "src": "codex", "ok": True}
    assert usage_payload({"ordinaryUsageAllowed": False, "rateLimits": {
        "primary": {"usedPercent": 100, "resetsAt": 0}, "secondary": None,
    }}, now=0)["st"] == "rejected"
    assert browse_services("12:00:00 Add 2 4 local. _clawdmeter._tcp. Small TV\n") == [("Small TV", "local.")]
    assert resolve_service("Small TV._clawdmeter._tcp.local. can be reached at smalltv.local.:80") == "smalltv.local:80"
    print("self-test passed")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--device", action="append", metavar="HOST", help="SmallTV host or URL; repeat for more devices")
    parser.add_argument("--discover", action="store_true", help="find SmallTV automatically with macOS Bonjour")
    parser.add_argument("--discover-timeout", type=int, default=3, help="Bonjour discovery time in seconds (default: 3)")
    parser.add_argument("--interval", type=int, default=60, help="seconds between reads (default: 60)")
    parser.add_argument("--timeout", type=int, default=15, help="per-request timeout in seconds (default: 15)")
    parser.add_argument("--once", action="store_true", help="read and send once, then exit")
    parser.add_argument("--stdout", action="store_true", help="print the SmallTV payload instead of sending it")
    parser.add_argument("--codex-bin", default="codex", help="Codex CLI path (default: codex)")
    parser.add_argument("--self-test", action="store_true", help="run the conversion check without Codex or SmallTV")
    args = parser.parse_args()

    if args.self_test:
        self_test()
        return 0
    if sys.platform != "darwin":
        parser.error("this client currently supports macOS only")
    if not args.device and not args.stdout:
        args.discover = True
    if args.interval < 10:
        parser.error("--interval must be at least 10 seconds")

    while True:
        try:
            with CodexAppServer(args.codex_bin, args.timeout) as server:
                payload = usage_payload(server.rate_limits())
            if args.stdout:
                print(json.dumps(payload, separators=(",", ":")))
            devices = list(args.device or [])
            if args.discover:
                devices.extend(discover_devices(args.discover_timeout))
            devices = list(dict.fromkeys(devices))
            if not devices and not args.stdout:
                raise MeterError("No SmallTV found via Bonjour; keep it online and on the same LAN")
            for device in devices:
                post_usage(device, payload, args.timeout)
                print("sent to %s: 5h %s%%, weekly %s%%" % (device, payload["s"], payload["w"]))
            if args.once:
                return 0
        except MeterError as error:
            print("codex-meter: %s" % error, file=sys.stderr)
            if args.once:
                return 1
        time.sleep(args.interval)


if __name__ == "__main__":
    raise SystemExit(main())
