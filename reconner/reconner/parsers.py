"""Output parsers for each orchestrated security tool.

Each parser accepts the raw stdout content of a tool and returns a
normalized list of dictionaries that the rest of Reconner can consume
without knowing the tool's specific output format.

Supported tools
---------------
- subfinder  (JSONL or plain-text fallback)
- httpx      (JSONL or plain-text fallback)
- whatweb    (JSON array or JSONL)
- gobuster   (JSONL or plain-text)
- nuclei     (JSONL)
- nmap       (XML via ``-oX``)
"""

import json
import logging
import re
import xml.etree.ElementTree as ET
from pathlib import Path
from typing import Any, Dict, List, Optional

logger = logging.getLogger(__name__)


# ---------------------------------------------------------------------------
# Per-tool parsers
# ---------------------------------------------------------------------------


def parse_subfinder_json(content: str) -> List[str]:
    """Parse subfinder JSONL output and return a deduplicated list of hostnames.

    Falls back to a regex domain scan when JSON parsing yields nothing.

    Args:
        content: Raw stdout from subfinder.

    Returns:
        A list of unique subdomain strings.
    """
    subdomains: List[str] = []

    for line in content.strip().splitlines():
        if not line.strip():
            continue
        try:
            data = json.loads(line)
            if isinstance(data, dict):
                host = data.get("host") or data.get("subdomain") or data.get("domain")
                if host:
                    subdomains.append(host)
        except json.JSONDecodeError:
            continue

    # Regex fallback when JSON parsing produced nothing
    if not subdomains:
        pattern = r"([a-zA-Z0-9](?:[a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?\.)+[a-zA-Z]{2,}"
        matches = re.findall(pattern, content)
        subdomains = [m[0] if isinstance(m, tuple) else m for m in matches]

    return list(set(subdomains))


def parse_httpx_json(content: str) -> List[Dict[str, Any]]:
    """Parse httpx JSONL output into a normalized list of host records.

    Falls back to URL/status-code regex extraction when JSON parsing fails.

    Args:
        content: Raw stdout from httpx.

    Returns:
        A list of dicts with keys: url, status_code, title, content_length,
        server, tech, headers, host.
    """
    results: List[Dict[str, Any]] = []

    for line in content.strip().splitlines():
        if not line.strip():
            continue
        try:
            data = json.loads(line)
            if isinstance(data, dict):
                results.append(
                    {
                        "url": data.get("url", ""),
                        "status_code": data.get("status_code", 0),
                        "title": data.get("title", ""),
                        "content_length": data.get("content_length", 0),
                        "server": data.get("server", ""),
                        "tech": data.get("tech", []),
                        "headers": data.get("headers", {}),
                        "host": data.get("host", ""),
                    }
                )
        except json.JSONDecodeError:
            continue

    # Plain-text fallback
    if not results:
        url_pattern = r"https?://[^\s]+"
        status_pattern = r"\[(\d{3})\]"
        urls = re.findall(url_pattern, content)
        statuses = re.findall(status_pattern, content)
        for i, url in enumerate(urls):
            results.append(
                {
                    "url": url,
                    "status_code": int(statuses[i]) if i < len(statuses) else 0,
                    "title": "",
                    "content_length": 0,
                    "server": "",
                    "tech": [],
                    "headers": {},
                    "host": "",
                }
            )

    return results


def parse_whatweb_json(content: str) -> List[Dict[str, Any]]:
    """Parse whatweb JSON or JSONL output.

    WhatWeb can emit either a JSON array or one JSON object per line.

    Args:
        content: Raw stdout from whatweb.

    Returns:
        A list of dicts with keys: url, plugins, http_status.
    """
    results: List[Dict[str, Any]] = []

    def _normalize(item: Dict[str, Any]) -> Dict[str, Any]:
        return {
            "url": item.get("target", ""),
            "plugins": item.get("plugins", {}),
            "http_status": item.get("http_status", 0),
        }

    try:
        data = json.loads(content)
        if isinstance(data, list):
            results = [_normalize(item) for item in data if isinstance(item, dict)]
        elif isinstance(data, dict):
            results = [_normalize(data)]
        return results
    except json.JSONDecodeError:
        pass

    # JSONL fallback
    for line in content.strip().splitlines():
        if not line.strip():
            continue
        try:
            data = json.loads(line)
            if isinstance(data, dict):
                results.append(_normalize(data))
        except json.JSONDecodeError:
            continue

    return results


def parse_gobuster_output(content: str) -> List[Dict[str, Any]]:
    """Parse gobuster output (JSONL or plain-text).

    Plain-text format example::

        /admin (Status: 200) [Size: 1234]

    Args:
        content: Raw stdout from gobuster.

    Returns:
        A list of dicts with keys: path, status, size, url.
    """
    results: List[Dict[str, Any]] = []

    # Try JSONL first
    for line in content.strip().splitlines():
        if not line.strip():
            continue
        try:
            data = json.loads(line)
            if isinstance(data, dict):
                results.append(
                    {
                        "path": data.get("Path", data.get("path", "")),
                        "status": data.get("Status", data.get("status", 0)),
                        "size": data.get("Size", data.get("size", 0)),
                        "url": data.get("URL", data.get("url", "")),
                    }
                )
        except json.JSONDecodeError:
            continue

    # Plain-text fallback
    if not results:
        pattern = r"([/\w\-\.]+)\s+\(Status:\s+(\d+)\)\s+\[Size:\s+(\d+)\]"
        for match in re.findall(pattern, content):
            results.append(
                {
                    "path": match[0],
                    "status": int(match[1]),
                    "size": int(match[2]),
                    "url": "",
                }
            )

    return results


def parse_nuclei_json(content: str) -> List[Dict[str, Any]]:
    """Parse nuclei JSONL output.

    Args:
        content: Raw stdout from nuclei.

    Returns:
        A list of dicts with keys: template_id, name, severity, matched_at,
        url, host, info, extracted_results.
    """
    results: List[Dict[str, Any]] = []

    for line in content.strip().splitlines():
        if not line.strip():
            continue
        try:
            data = json.loads(line)
            if isinstance(data, dict):
                results.append(
                    {
                        "template_id": data.get("template-id", data.get("template_id", "")),
                        "name": data.get("name", ""),
                        "severity": data.get("severity", "info").lower(),
                        "matched_at": data.get("matched-at", data.get("matched_at", "")),
                        "url": data.get("url", ""),
                        "host": data.get("host", ""),
                        "info": data.get("info", {}),
                        "extracted_results": data.get("extracted-results", []),
                    }
                )
        except json.JSONDecodeError:
            continue

    return results


def parse_nmap_xml(content: str) -> List[Dict[str, Any]]:
    """Parse nmap XML output (``-oX``) into one dict per open/filtered port.

    Args:
        content: Raw XML from nmap.

    Returns:
        Rows with host, port, protocol, state, service, product, version, etc.
    """
    results: List[Dict[str, Any]] = []
    if not content or not content.strip():
        return results

    try:
        root = ET.fromstring(content)
    except ET.ParseError as exc:
        logger.warning("nmap XML parse error: %s", exc)
        return results

    for host in root.findall("host"):
        hostnames: List[str] = []
        for hn in host.findall("hostnames/hostname"):
            name = hn.get("name")
            if name:
                hostnames.append(name)

        addresses: List[Dict[str, str]] = []
        for addr in host.findall("address"):
            addresses.append(
                {
                    "addr": addr.get("addr", "") or "",
                    "addrtype": addr.get("addrtype", "") or "",
                }
            )

        status_el = host.find("status")
        host_state = status_el.get("state", "") if status_el is not None else ""

        primary = ""
        if hostnames:
            primary = hostnames[0]
        elif addresses:
            primary = addresses[0]["addr"]

        for port in host.findall(".//port"):
            portid = port.get("portid", "") or ""
            protocol = port.get("protocol", "tcp") or "tcp"
            state_el = port.find("state")
            state = state_el.get("state", "") if state_el is not None else ""

            service_name = product = version = extrainfo = ""
            svc = port.find("service")
            if svc is not None:
                service_name = svc.get("name", "") or ""
                product = svc.get("product", "") or ""
                version = svc.get("version", "") or ""
                extrainfo = svc.get("extrainfo", "") or ""

            try:
                port_num = int(portid) if portid.isdigit() else 0
            except ValueError:
                port_num = 0

            results.append(
                {
                    "host": primary,
                    "hostnames": hostnames,
                    "addresses": addresses,
                    "host_state": host_state,
                    "port": port_num,
                    "protocol": protocol,
                    "state": state,
                    "service": service_name,
                    "product": product,
                    "version": version,
                    "extrainfo": extrainfo,
                }
            )

    return results


# ---------------------------------------------------------------------------
# Unified dispatcher
# ---------------------------------------------------------------------------

_PARSERS = {
    "subfinder": lambda c: [{"subdomain": s} for s in parse_subfinder_json(c)],
    "httpx": parse_httpx_json,
    "whatweb": parse_whatweb_json,
    "gobuster": parse_gobuster_output,
    "nuclei": parse_nuclei_json,
    "nmap": parse_nmap_xml,
}


def parse_tool_output(
    tool_name: str,
    content: str = "",
    file_path: Optional[str] = None,
) -> List[Dict[str, Any]]:
    """Dispatch raw tool output to the appropriate parser.

    Either *content* or *file_path* must be provided. When *file_path* is
    given it takes precedence over *content*.

    Args:
        tool_name: One of ``subfinder``, ``httpx``, ``nmap``, ``whatweb``,
                   ``gobuster``, or ``nuclei``.
        content: Raw stdout string.
        file_path: Path to a file containing the raw output.

    Returns:
        A normalized list of result dicts, or an empty list for unknown tools.
    """
    if file_path:
        content = Path(file_path).read_text()

    parser = _PARSERS.get(tool_name)
    if parser is None:
        logger.warning("No parser registered for tool: %s", tool_name)
        return []

    return parser(content)
