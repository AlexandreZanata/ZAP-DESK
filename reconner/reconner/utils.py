"""Utility functions for Reconner.

Covers tool detection, version probing, URL normalization,
output directory management, and logging setup.
"""

import os
import subprocess
import shutil
import logging
from pathlib import Path
from typing import Optional, Dict, List, Tuple
from datetime import datetime

logger = logging.getLogger(__name__)

# ---------------------------------------------------------------------------
# Default tool paths (fallback; shutil.which is tried first)
# ---------------------------------------------------------------------------
TOOL_PATHS: Dict[str, str] = {
    "httpx": "/usr/local/bin/httpx",
    "subfinder": "/usr/local/bin/subfinder",
    "nuclei": "/usr/local/bin/nuclei",
    "gobuster": "/usr/local/bin/gobuster",
    "whatweb": "/usr/local/bin/whatweb",
    "nmap": "/usr/bin/nmap",
}

# Known JSON output flags per tool
TOOL_JSON_FLAGS: Dict[str, List[str]] = {
    "httpx": ["-json", "--json"],
    "subfinder": ["-json", "--json", "-oJ"],
    "nuclei": ["-json", "--json", "-jsonl"],
    "gobuster": ["-oJ", "--output-format", "json"],
    "whatweb": ["--log-json"],
}

# Known version flags per tool
TOOL_VERSION_FLAGS: Dict[str, List[str]] = {
    "httpx": ["-version", "--version"],
    "subfinder": ["-version", "--version"],
    "nuclei": ["-version", "--version"],
    "gobuster": ["-version", "--version"],
    "whatweb": ["--version"],
    "nmap": ["--version"],
}


def check_tool_exists(tool_name: str) -> Tuple[bool, Optional[str]]:
    """Check whether a tool is available and return its resolved path.

    Tries the hardcoded default path first, then falls back to PATH lookup.

    Args:
        tool_name: Name of the tool (e.g. ``"httpx"``).

    Returns:
        A tuple ``(exists, path)`` where *path* is ``None`` when not found.
    """
    default_path = TOOL_PATHS.get(tool_name)
    if default_path and os.path.isfile(default_path) and os.access(default_path, os.X_OK):
        return True, default_path

    # Fall back to PATH lookup
    which_path = shutil.which(tool_name)
    if which_path:
        return True, which_path

    return False, None


def get_tool_version(tool_name: str) -> Optional[str]:
    """Probe a tool's version string by running its version flag.

    Args:
        tool_name: Name of the tool.

    Returns:
        The raw version string, or ``None`` if the tool is not found or
        the version flag fails.
    """
    exists, path = check_tool_exists(tool_name)
    if not exists or not path:
        return None

    for flag in TOOL_VERSION_FLAGS.get(tool_name, ["--version"]):
        try:
            result = subprocess.run(
                [path, flag],
                capture_output=True,
                text=True,
                timeout=5,
                stderr=subprocess.STDOUT,
            )
            if result.returncode == 0:
                return result.stdout.strip()
        except Exception as exc:
            logger.debug("Version probe failed for %s with %s: %s", tool_name, flag, exc)

    return None


def detect_json_support(tool_name: str) -> Optional[str]:
    """Detect the JSON output flag supported by a tool.

    Inspects the tool's ``--help`` output and returns the first matching
    flag from ``TOOL_JSON_FLAGS``.

    Args:
        tool_name: Name of the tool.

    Returns:
        The JSON flag string (e.g. ``"-json"``), or ``None``.
    """
    exists, path = check_tool_exists(tool_name)
    if not exists or not path:
        return None

    json_flags = TOOL_JSON_FLAGS.get(tool_name, [])

    try:
        result = subprocess.run(
            [path, "--help"],
            capture_output=True,
            text=True,
            timeout=5,
        )
        help_text = (result.stdout + result.stderr).lower()
        for flag in json_flags:
            if flag.lower() in help_text:
                return flag
    except Exception as exc:
        logger.debug("Could not inspect help for %s: %s", tool_name, exc)

    # Return the first known flag as a best-effort fallback
    return json_flags[0] if json_flags else None


def get_all_tool_versions() -> Dict[str, Optional[str]]:
    """Return a mapping of tool name → version string for all known tools."""
    return {tool: get_tool_version(tool) for tool in TOOL_PATHS}


def ensure_output_dir(output_dir: str) -> Path:
    """Create the output directory and its ``raw/`` subdirectory.

    Args:
        output_dir: Path to the desired output directory.

    Returns:
        A :class:`~pathlib.Path` pointing to the created directory.
    """
    path = Path(output_dir)
    path.mkdir(parents=True, exist_ok=True)
    (path / "raw").mkdir(exist_ok=True)
    return path


def get_timestamp() -> str:
    """Return the current timestamp formatted as ``YYYYMMDD-HHMMSS``."""
    return datetime.now().strftime("%Y%m%d-%H%M%S")


def create_organized_output_dir(base_dir: str, target: str) -> Path:
    """Create a timestamped output directory named after the target domain.

    Directory format: ``<base_dir>/<domain> - DD_MM_YYYY - HH:MMam``

    Example: ``results/example.com - 08_05_2026 - 7:22pm``

    Args:
        base_dir: Parent directory for all scan results.
        target: Raw target string (URL or bare domain).

    Returns:
        A :class:`~pathlib.Path` to the newly created directory.
    """
    # Strip protocol and path components to get the bare domain
    domain = (
        target.replace("https://", "")
        .replace("http://", "")
        .split("/")[0]
        .split(":")[0]
    )

    now = datetime.now()
    date_str = now.strftime("%d_%m_%Y")
    # e.g. "7:22pm"
    time_str = now.strftime("%I:%M%p").lstrip("0").lower()

    dir_name = f"{domain} - {date_str} - {time_str}"
    output_path = Path(base_dir) / dir_name
    output_path.mkdir(parents=True, exist_ok=True)
    (output_path / "raw").mkdir(exist_ok=True)

    return output_path


def normalize_url(url: str) -> str:
    """Ensure a URL has an ``https://`` scheme.

    Args:
        url: Raw URL or bare domain string.

    Returns:
        The URL with a scheme prefix.
    """
    url = url.strip()
    if not url.startswith(("http://", "https://")):
        url = f"https://{url}"
    return url


def read_targets_file(file_path: str) -> List[str]:
    """Read a newline-delimited list of targets from a file.

    Lines starting with ``#`` and blank lines are ignored.
    Each target is normalized via :func:`normalize_url`.

    Args:
        file_path: Path to the targets file.

    Returns:
        A list of normalized target URLs.
    """
    targets: List[str] = []
    with open(file_path, "r") as fh:
        for line in fh:
            line = line.strip()
            if line and not line.startswith("#"):
                targets.append(normalize_url(line))
    return targets


def setup_logging(
    verbose: bool = False,
    quiet: bool = False,
    log_file: Optional[str] = None,
) -> None:
    """Configure the root logger.

    Args:
        verbose: Enable DEBUG-level output.
        quiet: Suppress all output below ERROR level.
        log_file: Optional path to a log file (appended alongside stderr).
    """
    level = logging.DEBUG if verbose else (logging.ERROR if quiet else logging.INFO)

    handlers: List[logging.Handler] = [logging.StreamHandler()]
    if log_file:
        handlers.append(logging.FileHandler(log_file))

    logging.basicConfig(
        level=level,
        format="%(asctime)s - %(name)s - %(levelname)s - %(message)s",
        handlers=handlers,
    )
