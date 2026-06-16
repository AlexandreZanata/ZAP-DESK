"""Tool orchestration engine for Reconner.

The :class:`ToolRunner` class is responsible for executing each external
security tool in the correct dependency order, collecting their outputs,
and storing normalized results for the reporter.

Pipeline
--------
1. subfinder  — subdomain discovery
2. httpx      — live host probing
3. nmap       — TCP port scan and service detection (default profile)
4. whatweb    — technology fingerprinting
5. gobuster   — directory & file enumeration (parallel, per host)
6. nuclei     — vulnerability scanning
"""

import logging
import subprocess
import threading
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from datetime import datetime
from pathlib import Path
from typing import Any, Dict, List, Optional
from urllib.parse import urlparse

try:
    from rich.console import Console
    from rich.table import Table
    from rich import box

    RICH_AVAILABLE = True
except ImportError:
    RICH_AVAILABLE = False

from .parsers import parse_tool_output
from .utils import (
    check_tool_exists,
    detect_json_support,
    get_timestamp,
    get_tool_version,
    normalize_url,
)

logger = logging.getLogger(__name__)


class ToolRunner:
    """Orchestrates the execution of all security tools in the scan pipeline.

    Args:
        output_dir: Directory where raw outputs and logs are written.
        threads: Thread count passed to tools that support concurrency.
        proxy: Optional proxy URL (e.g. ``http://127.0.0.1:8080``).
        stealth: When ``True``, enables rate-limiting and low-noise flags.
        only_live: When ``True``, filters httpx results to HTTP 2xx/3xx only.
        skip_nuclei: When ``True``, skips the nuclei phase entirely.
        wordlists_dir: Base directory for SecLists (or custom wordlists).
        fast_mode: When ``True``, uses smaller wordlists and fewer nuclei templates.
    """

    def __init__(
        self,
        output_dir: str,
        threads: int = 20,
        proxy: Optional[str] = None,
        stealth: bool = False,
        only_live: bool = False,
        skip_nuclei: bool = False,
        wordlists_dir: Optional[str] = None,
        fast_mode: bool = False,
    ) -> None:
        self.output_dir = Path(output_dir)
        self.raw_dir = self.output_dir / "raw"
        self.raw_dir.mkdir(parents=True, exist_ok=True)

        self.threads = threads
        self.proxy = proxy
        self.stealth = stealth
        self.only_live = only_live
        self.skip_nuclei = skip_nuclei
        self.wordlists_dir = wordlists_dir or "/usr/share/seclists"
        self.fast_mode = fast_mode
        self.quiet = False  # Overridden by CLI after instantiation

        self.tool_versions: Dict[str, Any] = {}
        self.results: Dict[str, Any] = {
            "subdomains": [],
            "live_hosts": [],
            "httpx_results": [],
            "nmap_results": [],
            "whatweb_results": [],
            "gobuster_results": [],
            "nuclei_results": [],
            "errors": [],
        }

        # Initialize Rich console for inline output
        self._console: Optional[Console] = Console() if RICH_AVAILABLE else None

        # Initialize progress manager
        try:
            from .progress import ProgressManager

            self.progress = ProgressManager(quiet=self.quiet)
        except ImportError:
            self.progress = None

        self._check_tools()

    # ------------------------------------------------------------------
    # Internal helpers
    # ------------------------------------------------------------------

    def _check_tools(self) -> None:
        """Verify that all required tools are present and record their versions."""
        for tool_name in ["subfinder", "httpx", "nmap", "whatweb", "gobuster", "nuclei"]:
            exists, path = check_tool_exists(tool_name)
            if not exists:
                logger.warning("Tool not found: %s", tool_name)
                self._record_error(tool_name, "Tool not found")
            else:
                version = get_tool_version(tool_name)
                self.tool_versions[tool_name] = {"path": path, "version": version}
                logger.info("Found %s at %s (version: %s)", tool_name, path, version)

    def _record_error(self, tool: str, message: str) -> None:
        """Append an error entry to the results dict."""
        self.results["errors"].append(
            {
                "tool": tool,
                "error": message,
                "timestamp": datetime.now().isoformat(),
            }
        )

    def _print(self, message: str) -> None:
        """Print *message* unless quiet mode is active."""
        if not self.quiet:
            print(message, flush=True)

    # ------------------------------------------------------------------
    # Phase 1 — Subdomain discovery
    # ------------------------------------------------------------------

    def run_subfinder(self, target: str) -> List[str]:
        """Run subfinder against *target* and return discovered subdomains.

        Args:
            target: Bare domain name (no scheme).

        Returns:
            A deduplicated list of subdomain strings including *target* itself.
        """
        logger.info("Running subfinder for %s", target)

        exists, path = check_tool_exists("subfinder")
        if not exists:
            logger.error("subfinder not found")
            self._print("❌ subfinder not found!")
            return []

        json_flag = detect_json_support("subfinder")
        timestamp = get_timestamp()
        output_file = self.raw_dir / f"subfinder-{timestamp}.json"

        cmd = [path, "-d", target]
        if json_flag:
            cmd.append(json_flag)
        if self.proxy:
            cmd.extend(["-proxy", self.proxy])
        if self.stealth:
            cmd.append("-silent")

        if self.progress:
            self.progress.show_tool_start("subfinder", 1, 6, f"Discovering subdomains for {target}")

        # Background monitor thread for live progress
        monitor_thread: Optional[threading.Thread] = None
        if self.progress and not self.quiet:

            def _monitor() -> None:
                last_count = 0
                while True:
                    try:
                        if output_file.exists():
                            content = output_file.read_text()
                            if content.strip():
                                parsed = parse_tool_output("subfinder", content)
                                current_count = len(parsed)
                                if current_count > last_count:
                                    self.progress.show_tool_progress(
                                        "subfinder", current_count, None, "subdomains found"
                                    )
                                    if current_count - last_count >= 10:
                                        self.progress.show_live_results("subfinder", parsed[:20])
                                    last_count = current_count
                        time.sleep(3)
                    except (KeyboardInterrupt, Exception):
                        break

            monitor_thread = threading.Thread(target=_monitor, daemon=True)
            monitor_thread.start()

        try:
            with open(output_file, "w") as fh:
                result = subprocess.run(
                    cmd,
                    stdout=fh,
                    stderr=subprocess.PIPE,
                    text=True,
                    timeout=300,
                )

            if monitor_thread:
                time.sleep(2)  # Let the monitor catch the final output

            if result.returncode != 0:
                logger.warning("subfinder exited non-zero: %s", result.stderr)

            parsed = parse_tool_output("subfinder", output_file.read_text())
            subdomains = list({item["subdomain"] for item in parsed if "subdomain" in item})
            subdomains.append(target)
            subdomains = list(set(subdomains))

            self.results["subdomains"] = subdomains
            logger.info("subfinder found %d subdomains", len(subdomains))

            if self.progress:
                self.progress.show_tool_complete("subfinder", subdomains, "subdomains")
                self.progress.show_live_results(
                    "subfinder", [{"subdomain": s} for s in subdomains[:20]]
                )
            else:
                self._print(f"✅ [1/6] subfinder completed: {len(subdomains)} subdomains found")

            return subdomains

        except subprocess.TimeoutExpired:
            logger.error("subfinder timed out")
            self._print("⏱️  subfinder timed out")
            self._record_error("subfinder", "Timeout")
            return []
        except Exception as exc:
            logger.error("subfinder failed: %s", exc)
            self._print(f"❌ subfinder failed: {exc}")
            self._record_error("subfinder", str(exc))
            return []

    # ------------------------------------------------------------------
    # Phase 2 — Live host probing
    # ------------------------------------------------------------------

    def run_httpx(self, targets: List[str]) -> List[Dict[str, Any]]:
        """Run httpx against *targets* and return live host records.

        Args:
            targets: List of normalized URLs to probe.

        Returns:
            A list of normalized httpx result dicts.
        """
        logger.info("Running httpx for %d targets", len(targets))

        if self.progress:
            self.progress.show_tool_start(
                "httpx", 2, 6, f"Probing {len(targets)} targets for live hosts"
            )
        else:
            self._print(f"🌐 [2/6] Running httpx for {len(targets)} targets...")

        exists, path = check_tool_exists("httpx")
        if not exists:
            logger.error("httpx not found")
            self._print("❌ httpx not found!")
            return []

        json_flag = detect_json_support("httpx")
        timestamp = get_timestamp()
        output_file = self.raw_dir / f"httpx-{timestamp}.json"
        targets_file = self.raw_dir / f"httpx-targets-{timestamp}.txt"
        targets_file.write_text("\n".join(targets))

        cmd = [path, "-l", str(targets_file)]
        if json_flag:
            cmd.append(json_flag)
        cmd.extend(["-title", "-status-code", "-tech-detect", "-content-length"])
        if self.proxy:
            cmd.extend(["-proxy", self.proxy])
        if self.stealth:
            cmd.extend(["-silent", "-rate-limit", "10"])

        try:
            with open(output_file, "w") as fh:
                result = subprocess.run(
                    cmd,
                    stdout=fh,
                    stderr=subprocess.PIPE,
                    text=True,
                    timeout=600,
                )

            if result.returncode != 0:
                logger.warning("httpx exited non-zero: %s", result.stderr)

            parsed = parse_tool_output("httpx", output_file.read_text())

            if self.only_live:
                parsed = [
                    r for r in parsed
                    if r.get("status_code", 0) in {200, 201, 202, 204, 301, 302, 307, 308}
                ]

            self.results["httpx_results"] = parsed
            self.results["live_hosts"] = [r["url"] for r in parsed]
            logger.info("httpx found %d live hosts", len(parsed))

            if self.progress:
                self.progress.show_tool_complete("httpx", parsed, "live hosts")
                self.progress.show_live_results("httpx", parsed[:20])
            else:
                self._print(f"✅ [2/6] httpx completed: {len(parsed)} live hosts found")

            return parsed

        except subprocess.TimeoutExpired:
            logger.error("httpx timed out")
            self._record_error("httpx", "Timeout")
            return []
        except Exception as exc:
            logger.error("httpx failed: %s", exc)
            self._record_error("httpx", str(exc))
            return []

    # ------------------------------------------------------------------
    # Phase 3 — Port scan (nmap)
    # ------------------------------------------------------------------

    def _hosts_from_live_urls(self, live_urls: List[str]) -> List[str]:
        """Extract unique hostnames from live URL list."""
        seen: set = set()
        ordered: List[str] = []
        for url in live_urls:
            parsed = urlparse(url)
            host = (parsed.hostname or "").strip().lower()
            if host and host not in seen:
                seen.add(host)
                ordered.append(parsed.hostname or host)
        return ordered

    def run_nmap(self, host: str) -> List[Dict[str, Any]]:
        """Run nmap default-style scan (``-Pn -sV`` on top TCP ports) against *host*.

        Uses XML output (``-oX``) for structured parsing. Skips if nmap is missing.

        Args:
            host: Hostname or IP to scan.

        Returns:
            Normalized port rows for this host (may be empty).
        """
        logger.info("Running nmap for %s", host)

        exists, path = check_tool_exists("nmap")
        if not exists:
            logger.warning("nmap not found — skipping host %s", host)
            return []

        timestamp = get_timestamp()
        host_safe = (
            host.replace(".", "_")
            .replace(":", "_")
            .replace("/", "_")
        )
        output_file = self.raw_dir / f"nmap-{host_safe}-{timestamp}.xml"

        top_ports = "100" if self.fast_mode else "1000"
        timing = "T2" if self.stealth else "T4"
        cmd = [
            path,
            "-Pn",
            "-sV",
            f"-{timing}",
            "--top-ports",
            top_ports,
            "-oX",
            str(output_file),
            host,
        ]

        try:
            subprocess.run(
                cmd,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.PIPE,
                text=True,
                timeout=1200,
            )
            if not output_file.exists():
                return []
            content = output_file.read_text()
            parsed = parse_tool_output("nmap", content)
            for row in parsed:
                row["scanned_target"] = host
            return parsed
        except subprocess.TimeoutExpired:
            logger.warning("nmap timed out for %s", host)
            self._record_error("nmap", f"Timeout: {host}")
            return []
        except Exception as exc:
            logger.error("nmap failed for %s: %s", host, exc)
            self._record_error("nmap", f"{host}: {exc}")
            return []

    def run_nmap_parallel(self, live_urls: List[str]) -> List[Dict[str, Any]]:
        """Run nmap across deduplicated hosts from *live_urls* (limited concurrency).

        Args:
            live_urls: Live host URLs from httpx.

        Returns:
            Aggregated nmap port/service rows.
        """
        hosts = self._hosts_from_live_urls(live_urls)
        if not hosts:
            return []

        exists, _ = check_tool_exists("nmap")
        if not exists:
            logger.warning("nmap not found — skipping port scan phase")
            self._print("❌ nmap not found — skipping port scan")
            return []

        max_hosts = 5 if self.fast_mode else 15
        hosts = hosts[:max_hosts]

        if self.progress:
            self.progress.show_tool_start(
                "nmap",
                3,
                6,
                f"Port scan ({len(hosts)} host(s), --top-ports {'100' if self.fast_mode else '1000'})",
            )
        else:
            self._print(f"🗺️  [3/6] Running nmap for {len(hosts)} host(s)...")

        all_rows: List[Dict[str, Any]] = []
        workers = 2 if self.stealth else 3

        with ThreadPoolExecutor(max_workers=min(workers, len(hosts))) as executor:
            futures = {executor.submit(self.run_nmap, h): h for h in hosts}
            for future in as_completed(futures):
                h = futures[future]
                try:
                    all_rows.extend(future.result())
                except Exception as exc:
                    logger.error("nmap future failed for %s: %s", h, exc)

        self.results["nmap_results"] = all_rows
        open_ports = sum(1 for r in all_rows if r.get("state") == "open")

        if self.progress:
            self.progress.show_tool_complete("nmap", all_rows, "port rows")
        else:
            self._print(f"✅ [3/6] nmap completed: {open_ports} open port(s) across {len(hosts)} host(s)")

        return all_rows

    # ------------------------------------------------------------------
    # Phase 4 — Technology fingerprinting
    # ------------------------------------------------------------------

    def run_whatweb(self, urls: List[str]) -> List[Dict[str, Any]]:
        """Run whatweb against *urls* for technology fingerprinting.

        Args:
            urls: List of live host URLs to fingerprint.

        Returns:
            A list of normalized whatweb result dicts.
        """
        if not urls:
            return []

        logger.info("Running whatweb for %d URLs", len(urls))

        if self.progress:
            self.progress.show_tool_start(
                "whatweb", 4, 6, f"Fingerprinting technologies on {len(urls)} URLs"
            )
        else:
            self._print(f"🔧 [4/6] Running whatweb for {len(urls)} URLs...")

        exists, path = check_tool_exists("whatweb")
        if not exists:
            logger.warning("whatweb not found — skipping phase 4")
            return []

        json_flag = detect_json_support("whatweb")
        timestamp = get_timestamp()
        output_file = self.raw_dir / f"whatweb-{timestamp}.json"

        cmd = [path]
        if json_flag:
            cmd.append(json_flag)
        if self.proxy:
            cmd.extend(["--proxy", self.proxy])
        cmd.extend(urls[:50])  # Cap at 50 URLs per invocation

        try:
            with open(output_file, "w") as fh:
                subprocess.run(
                    cmd,
                    stdout=fh,
                    stderr=subprocess.PIPE,
                    text=True,
                    timeout=300,
                )

            parsed = parse_tool_output("whatweb", output_file.read_text())
            self.results["whatweb_results"] = parsed
            logger.info("whatweb processed %d URLs", len(parsed))

            if self.progress:
                self.progress.show_tool_complete("whatweb", parsed, "URLs processed")
            else:
                self._print(f"✅ [4/6] whatweb completed: {len(parsed)} URLs processed")

            return parsed

        except Exception as exc:
            logger.error("whatweb failed: %s", exc)
            self._record_error("whatweb", str(exc))
            return []

    # ------------------------------------------------------------------
    # Phase 5 — Directory enumeration
    # ------------------------------------------------------------------

    def run_gobuster(self, url: str, wordlist: str = "common.txt") -> List[Dict[str, Any]]:
        """Run gobuster against a single *url*.

        Args:
            url: Target URL to enumerate.
            wordlist: Wordlist filename relative to the SecLists web-content dir.

        Returns:
            A list of normalized gobuster result dicts.
        """
        logger.info("Running gobuster for %s", url)
        self._print(f"📁 Running gobuster for {url}...")

        exists, path = check_tool_exists("gobuster")
        if not exists:
            logger.warning("gobuster not found — skipping")
            return []

        # Resolve wordlist path
        wordlist_path = Path(self.wordlists_dir) / "Discovery" / "Web-Content" / wordlist
        if not wordlist_path.exists():
            for alt in [
                Path("/usr/share/wordlists") / wordlist,
                Path(self.wordlists_dir) / wordlist,
            ]:
                if alt.exists():
                    wordlist_path = alt
                    break
            else:
                logger.warning("Wordlist %s not found — skipping gobuster for %s", wordlist, url)
                return []

        timestamp = get_timestamp()
        host_safe = (
            url.replace("https://", "")
            .replace("http://", "")
            .replace("/", "_")
            .replace(":", "_")
        )
        output_file = self.raw_dir / f"gobuster-{host_safe}-{timestamp}.txt"

        extensions = "php,html,txt" if self.fast_mode else "php,html,txt,js,bak,old,zip,tar.gz,sql"
        cmd = [
            path, "dir",
            "-u", url,
            "-w", str(wordlist_path),
            "-t", str(min(self.threads, 50)),
            "-x", extensions,
        ]
        if self.proxy:
            cmd.extend(["--proxy", self.proxy])
        if self.stealth:
            cmd.extend(["-t", "5", "-r", "-k"])

        try:
            with open(output_file, "w") as fh:
                subprocess.run(
                    cmd,
                    stdout=fh,
                    stderr=subprocess.PIPE,
                    text=True,
                    timeout=600,
                )

            parsed = parse_tool_output("gobuster", output_file.read_text())
            logger.info("gobuster found %d paths for %s", len(parsed), url)
            self._print(f"✅ gobuster completed for {url}: {len(parsed)} paths found")
            return parsed

        except subprocess.TimeoutExpired:
            logger.warning("gobuster timed out for %s", url)
            return []
        except Exception as exc:
            logger.error("gobuster failed for %s: %s", url, exc)
            return []

    def run_gobuster_parallel(self, urls: List[str]) -> List[Dict[str, Any]]:
        """Run gobuster concurrently across multiple *urls*.

        Args:
            urls: List of live host URLs to enumerate.

        Returns:
            Aggregated list of gobuster result dicts (with ``target_url`` injected).
        """
        if not urls:
            return []

        self._print(f"📁 [5/6] Running gobuster for {len(urls)} hosts...")
        wordlist = "common.txt" if self.fast_mode else "directory-list-2.3-medium.txt"
        all_results: List[Dict[str, Any]] = []

        with ThreadPoolExecutor(max_workers=min(self.threads, 5)) as executor:
            futures = {executor.submit(self.run_gobuster, url, wordlist): url for url in urls}
            for future in as_completed(futures):
                url = futures[future]
                try:
                    results = future.result()
                    for r in results:
                        r["target_url"] = url
                    all_results.extend(results)
                except Exception as exc:
                    logger.error("gobuster future failed for %s: %s", url, exc)

        self.results["gobuster_results"] = all_results
        self._print(f"✅ [5/6] gobuster completed: {len(all_results)} total paths found")
        return all_results

    # ------------------------------------------------------------------
    # Phase 6 — Vulnerability scanning
    # ------------------------------------------------------------------

    def run_nuclei(self, targets: List[str]) -> List[Dict[str, Any]]:
        """Run nuclei against *targets* for vulnerability detection.

        Skipped entirely when ``skip_nuclei=True``.

        Args:
            targets: List of live host URLs to scan.

        Returns:
            A list of normalized nuclei finding dicts.
        """
        if self.skip_nuclei:
            logger.info("Skipping nuclei (--skip-nuclei)")
            self._print("⏭️  [6/6] Skipping nuclei (--skip-nuclei)")
            return []

        logger.info("Running nuclei for %d targets", len(targets))

        if self.progress:
            self.progress.show_tool_start(
                "nuclei", 6, 6, f"Scanning {len(targets)} targets for vulnerabilities"
            )
        else:
            self._print(f"🔬 [6/6] Running nuclei for {len(targets)} targets...")

        exists, path = check_tool_exists("nuclei")
        if not exists:
            logger.warning("nuclei not found — skipping phase 6")
            return []

        json_flag = detect_json_support("nuclei")
        timestamp = get_timestamp()
        output_file = self.raw_dir / f"nuclei-{timestamp}.json"
        targets_file = self.raw_dir / f"nuclei-targets-{timestamp}.txt"
        targets_file.write_text("\n".join(targets))

        severity = "critical,high" if self.fast_mode else "critical,high,medium"
        cmd = [path, "-l", str(targets_file), "-severity", severity, "-t", str(self.threads)]
        if json_flag:
            cmd.append(json_flag)
        if self.proxy:
            cmd.extend(["-proxy", self.proxy])
        if self.stealth:
            cmd.extend(["-rate-limit", "10"])

        try:
            with open(output_file, "w") as fh:
                subprocess.run(
                    cmd,
                    stdout=fh,
                    stderr=subprocess.PIPE,
                    text=True,
                    timeout=1800,  # 30-minute hard cap
                )

            parsed = parse_tool_output("nuclei", output_file.read_text())
            self.results["nuclei_results"] = parsed
            logger.info("nuclei found %d findings", len(parsed))

            if self.progress:
                self.progress.show_tool_complete("nuclei", parsed, "findings")
                if parsed:
                    self.progress.show_live_results("nuclei", parsed[:20])
            else:
                self._print(f"✅ [6/6] nuclei completed: {len(parsed)} findings")

            return parsed

        except subprocess.TimeoutExpired:
            logger.warning("nuclei timed out")
            self._record_error("nuclei", "Timeout")
            return []
        except Exception as exc:
            logger.error("nuclei failed: %s", exc)
            self._record_error("nuclei", str(exc))
            return []

    # ------------------------------------------------------------------
    # Full pipeline
    # ------------------------------------------------------------------

    def run_full_scan(self, targets: List[str]) -> Dict[str, Any]:
        """Execute the complete six-phase reconnaissance pipeline.

        Args:
            targets: One or more normalized target URLs.

        Returns:
            The populated ``self.results`` dictionary.
        """
        logger.info("Starting full reconnaissance scan")
        self._print("\n" + "=" * 60)
        self._print("🚀 Starting Full Reconnaissance Scan")
        self._print("=" * 60 + "\n")

        # Phase 1 — Subdomain discovery
        all_targets: set = set()
        for target in targets:
            bare = (
                target.replace("https://", "")
                .replace("http://", "")
                .split("/")[0]
            )
            subdomains = self.run_subfinder(bare)
            all_targets.update(subdomains)

        normalized = [normalize_url(t) for t in all_targets]

        # Phase 2 — Live host probing
        httpx_results = self.run_httpx(normalized)
        live_urls = [r["url"] for r in httpx_results]

        if not live_urls:
            logger.warning("No live hosts found — skipping remaining phases")
            return self.results

        # Phase 3 — Port scan (nmap)
        self.run_nmap_parallel(live_urls)

        # Phase 4 — Technology fingerprinting
        self.run_whatweb(live_urls[:50])

        # Phase 5 — Directory enumeration
        if not self.fast_mode:
            self.run_gobuster_parallel(live_urls[:10])
        else:
            self._print("⏭️  [5/6] Skipping gobuster (fast mode)")

        # Phase 6 — Vulnerability scanning
        self.run_nuclei(live_urls)

        logger.info("Full scan completed")
        self._print("\n" + "=" * 60)
        self._print("✅ Full Scan Completed!")
        self._print("=" * 60 + "\n")

        return self.results
