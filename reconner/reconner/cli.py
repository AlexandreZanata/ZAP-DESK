"""Command-line interface for Reconner.

Entry point: ``reconner`` (installed via ``setup.py`` console_scripts).
"""

import json
import logging
import sys
from pathlib import Path
from typing import List, Optional

import click

from .reporter import Reporter
from .runner import ToolRunner
from .utils import (
    check_tool_exists,
    create_organized_output_dir,
    ensure_output_dir,
    get_all_tool_versions,
    normalize_url,
    read_targets_file,
    setup_logging,
)

# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

VERSION = "2.0.0"

_LEGAL_WARNING = """
╔══════════════════════════════════════════════════════════════════════════════╗
║                          ⚠️  LEGAL WARNING  ⚠️                              ║
╠══════════════════════════════════════════════════════════════════════════════╣
║                                                                              ║
║  This tool is designed for AUTHORIZED security testing ONLY.                 ║
║                                                                              ║
║  ⚠️  ONLY execute against targets you own or have explicit written          ║
║      permission to test.                                                     ║
║                                                                              ║
║  ⚠️  Unauthorized scanning is ILLEGAL and may result in criminal charges.   ║
║                                                                              ║
║  ⚠️  You are RESPONSIBLE for ensuring you have proper authorization         ║
║      before running this tool.                                               ║
║                                                                              ║
║  ⚠️  The authors and contributors are NOT responsible for misuse.           ║
║                                                                              ║
╚══════════════════════════════════════════════════════════════════════════════╝
"""


# ---------------------------------------------------------------------------
# CLI definition
# ---------------------------------------------------------------------------


@click.command()
@click.option(
    "--target", "-t",
    help="Single target domain (e.g. example.com or https://example.com).",
    type=str,
)
@click.option(
    "--input-file", "-i",
    help="File containing targets, one per line.",
    type=click.Path(exists=True),
)
@click.option(
    "--output-dir", "-o",
    default="./results",
    show_default=True,
    help="Base output directory for scan results.",
    type=click.Path(),
)
@click.option(
    "--wordlists-dir", "-w",
    default="/usr/share/seclists",
    show_default=True,
    help="Directory containing SecLists (or custom) wordlists.",
    type=click.Path(),
)
@click.option(
    "--threads",
    default=20,
    show_default=True,
    help="Thread count passed to tools that support concurrency.",
    type=int,
)
@click.option(
    "--proxy",
    help="Proxy URL (e.g. http://127.0.0.1:8080).",
    type=str,
)
@click.option(
    "--stealth",
    is_flag=True,
    help="Low-noise mode: rate-limited, randomized timing.",
)
@click.option(
    "--only-live",
    is_flag=True,
    help="Filter httpx results to HTTP 2xx/3xx responses only.",
)
@click.option(
    "--skip-nuclei",
    is_flag=True,
    help="Skip the nuclei vulnerability scanning phase.",
)
@click.option(
    "--fast",
    is_flag=True,
    help="Fast mode: smaller wordlists and fewer nuclei templates.",
)
@click.option(
    "--export-only",
    is_flag=True,
    help="Re-generate reports from an existing summary.json (no scanning).",
)
@click.option(
    "--verbose", "-v",
    is_flag=True,
    help="Enable DEBUG-level logging.",
)
@click.option(
    "--quiet", "-q",
    is_flag=True,
    help="Suppress all progress output.",
)
@click.version_option(version=VERSION, prog_name="reconner")
def main(
    target: Optional[str],
    input_file: Optional[str],
    output_dir: str,
    wordlists_dir: str,
    threads: int,
    proxy: Optional[str],
    stealth: bool,
    only_live: bool,
    skip_nuclei: bool,
    fast: bool,
    export_only: bool,
    verbose: bool,
    quiet: bool,
) -> None:
    """Reconner — Automated, modular security reconnaissance orchestrator.

    Chains subfinder → httpx → nmap → whatweb → gobuster → nuclei into a single
    workflow and produces JSON, Markdown, and PDF reports.
    """
    # ------------------------------------------------------------------ #
    # Legal confirmation                                                   #
    # ------------------------------------------------------------------ #
    if not quiet:
        click.echo(click.style(_LEGAL_WARNING, fg="red", bold=True))
        if not click.confirm(
            "Do you have explicit written authorization to scan the target(s)?",
            default=False,
        ):
            click.echo(click.style("Scan cancelled. Exiting.", fg="yellow"))
            sys.exit(1)
        click.echo()

    # ------------------------------------------------------------------ #
    # Input validation                                                     #
    # ------------------------------------------------------------------ #
    if not target and not input_file and not export_only:
        click.echo(click.style("Error: --target or --input-file is required.", fg="red"))
        sys.exit(1)

    if target and input_file:
        click.echo(click.style("Error: use --target OR --input-file, not both.", fg="red"))
        sys.exit(1)

    # ------------------------------------------------------------------ #
    # Resolve targets                                                      #
    # ------------------------------------------------------------------ #
    targets: List[str] = []
    primary_domain: Optional[str] = None

    if target:
        normalized = normalize_url(target)
        targets.append(normalized)
        primary_domain = (
            normalized.replace("https://", "").replace("http://", "").split("/")[0].split(":")[0]
        )
    elif input_file:
        targets = read_targets_file(input_file)
        if targets:
            primary_domain = (
                targets[0].replace("https://", "").replace("http://", "").split("/")[0].split(":")[0]
            )

    if not targets and not export_only:
        click.echo(click.style("Error: no valid targets found.", fg="red"))
        sys.exit(1)

    # ------------------------------------------------------------------ #
    # Output directory                                                     #
    # ------------------------------------------------------------------ #
    if not export_only and primary_domain:
        output_path = create_organized_output_dir(output_dir, primary_domain)
    else:
        output_path = ensure_output_dir(output_dir)

    # ------------------------------------------------------------------ #
    # Logging                                                              #
    # ------------------------------------------------------------------ #
    log_file = str(output_path / "reconner.log")
    setup_logging(verbose=verbose, quiet=quiet, log_file=log_file)
    logger = logging.getLogger(__name__)

    if not quiet:
        click.echo(f"📁 Results directory: {output_path}")

    # ------------------------------------------------------------------ #
    # Tool availability check                                              #
    # ------------------------------------------------------------------ #
    if not export_only:
        missing = [
            t for t in ["subfinder", "httpx", "nmap", "whatweb", "gobuster", "nuclei"]
            if not check_tool_exists(t)[0]
        ]
        if missing and not quiet:
            click.echo(
                click.style(
                    f"Warning: tools not found: {', '.join(missing)}",
                    fg="yellow",
                )
            )

    # ------------------------------------------------------------------ #
    # Export-only mode                                                     #
    # ------------------------------------------------------------------ #
    if export_only:
        summary_file = output_path / "summary.json"
        if not summary_file.exists():
            click.echo(
                click.style(
                    "Error: summary.json not found. Run a scan first.", fg="red"
                )
            )
            sys.exit(1)

        with open(summary_file) as fh:
            cached = json.load(fh)

        results = {
            "subdomains": cached.get("subdomains", {}).get("list", []),
            "live_hosts": cached.get("live_hosts", {}).get("list", []),
            "httpx_results": cached.get("httpx_results", []),
            "nmap_results": cached.get("nmap_results", []),
            "whatweb_results": cached.get("whatweb_results", []),
            "gobuster_results": cached.get("gobuster_results", []),
            "nuclei_results": cached.get("nuclei_results", []),
            "errors": cached.get("errors", []),
        }

        tool_versions = get_all_tool_versions()
        reporter = Reporter(str(output_path), tool_versions, target_domain=primary_domain or "Unknown")
        reports = reporter.generate_all_reports(results)

        if not quiet:
            click.echo(click.style("\n✅ Reports regenerated successfully!", fg="green"))
            for name, path in reports.items():
                click.echo(f"  ✓ {name}: {path}")

        sys.exit(0)

    # ------------------------------------------------------------------ #
    # Full scan                                                            #
    # ------------------------------------------------------------------ #
    if not quiet:
        click.echo(click.style("\n🚀 Starting reconnaissance scan...", fg="cyan", bold=True))
        click.echo(f"  Targets  : {len(targets)}")
        click.echo(f"  Output   : {output_path}")
        click.echo(f"  Threads  : {threads}")
        if proxy:
            click.echo(f"  Proxy    : {proxy}")
        if stealth:
            click.echo("  Mode     : Stealth")
        if fast:
            click.echo("  Mode     : Fast")
        click.echo()

    runner = ToolRunner(
        output_dir=str(output_path),
        threads=threads,
        proxy=proxy,
        stealth=stealth,
        only_live=only_live,
        skip_nuclei=skip_nuclei,
        wordlists_dir=wordlists_dir,
        fast_mode=fast,
    )
    runner.quiet = quiet

    try:
        results = runner.run_full_scan(targets)
    except KeyboardInterrupt:
        logger.warning("Scan interrupted by user")
        if not quiet:
            click.echo(click.style("\n⚠️  Scan interrupted by user.", fg="yellow"))
        sys.exit(1)
    except Exception as exc:
        logger.error("Scan failed: %s", exc, exc_info=True)
        if not quiet:
            click.echo(click.style(f"\n❌ Scan failed: {exc}", fg="red"))
        sys.exit(1)

    # ------------------------------------------------------------------ #
    # Report generation                                                    #
    # ------------------------------------------------------------------ #
    if not quiet:
        click.echo(click.style("\n📊 Generating reports...", fg="cyan"))

    reporter = Reporter(
        str(output_path),
        runner.tool_versions,
        target_domain=primary_domain or "Unknown",
    )
    reports = reporter.generate_all_reports(results)

    # ------------------------------------------------------------------ #
    # Final summary                                                        #
    # ------------------------------------------------------------------ #
    if not quiet:
        click.echo(click.style("\n✅ Scan completed successfully!", fg="green", bold=True))
        click.echo("\n📄 Generated Reports:")
        for name, path in reports.items():
            click.echo(f"  ✓ {name}: {path}")

        click.echo("\n📊 Summary:")
        click.echo(f"  Subdomains     : {len(results.get('subdomains', []))}")
        click.echo(f"  Live Hosts     : {len(results.get('live_hosts', []))}")
        open_ports = sum(
            1 for r in results.get("nmap_results", []) if r.get("state") == "open"
        )
        click.echo(f"  Open ports     : {open_ports} (nmap)")
        click.echo(f"  Vulnerabilities: {len(results.get('nuclei_results', []))}")
        click.echo(f"  Paths Found    : {len(results.get('gobuster_results', []))}")

        if results.get("errors"):
            click.echo(
                click.style(f"\n⚠️  Errors during scan: {len(results['errors'])}", fg="yellow")
            )

    logger.info("Scan completed successfully")


if __name__ == "__main__":
    main()
