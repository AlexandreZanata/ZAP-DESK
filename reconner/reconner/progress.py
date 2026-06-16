"""Progress visualization for Reconner.

Uses the ``rich`` library to render live progress bars, spinners, and
result tables. Gracefully degrades to plain print statements when
``rich`` is not installed.
"""

import time
import threading
from pathlib import Path
from typing import Any, Dict, List, Optional

try:
    from rich.console import Console
    from rich.progress import (
        BarColumn,
        Progress,
        SpinnerColumn,
        TaskID,
        TextColumn,
        TimeElapsedColumn,
    )
    from rich.table import Table
    from rich import box

    RICH_AVAILABLE = True
except ImportError:
    RICH_AVAILABLE = False

# Emoji prefix per tool for visual clarity
_TOOL_EMOJI: Dict[str, str] = {
    "subfinder": "🔍",
    "httpx": "🌐",
    "nmap": "🗺️",
    "whatweb": "🔧",
    "gobuster": "📁",
    "nuclei": "🔬",
}


class ProgressManager:
    """Manages progress visualization across all scan phases.

    When ``quiet=True`` or ``rich`` is unavailable, all methods become
    no-ops so the rest of the codebase never needs to guard against
    ``None`` checks.
    """

    def __init__(self, quiet: bool = False) -> None:
        self.quiet = quiet
        if RICH_AVAILABLE and not quiet:
            self.console: Optional[Console] = Console()
            self.progress: Optional[Progress] = Progress(
                SpinnerColumn(),
                TextColumn("[progress.description]{task.description}"),
                BarColumn(),
                TextColumn("[progress.percentage]{task.percentage:>3.0f}%"),
                TimeElapsedColumn(),
                console=self.console,
            )
        else:
            self.console = None
            self.progress = None

    # ------------------------------------------------------------------
    # Phase lifecycle helpers
    # ------------------------------------------------------------------

    def show_tool_start(
        self,
        tool_name: str,
        step: int,
        total: int,
        description: str = "",
    ) -> None:
        """Print a phase-start banner for *tool_name*.

        Args:
            tool_name: Name of the tool being started.
            step: Current step number (1-based).
            total: Total number of steps.
            description: Optional one-line description shown below the banner.
        """
        if not self.console or self.quiet:
            return

        emoji = _TOOL_EMOJI.get(tool_name, "⚙️")
        self.console.print(
            f"\n[bold cyan]{emoji} [{step}/{total}] Starting {tool_name}...[/bold cyan]"
        )
        if description:
            self.console.print(f"[dim]{description}[/dim]")

    def show_tool_progress(
        self,
        tool_name: str,
        current: int,
        total: Optional[int] = None,
        status: str = "",
    ) -> None:
        """Print an inline progress update (overwrites the current line).

        Args:
            tool_name: Name of the running tool.
            current: Number of items found so far.
            total: Total expected items (``None`` for unknown).
            status: Optional extra status string appended to the line.
        """
        if not self.console or self.quiet:
            return

        if total:
            pct = (current / total) * 100
            self.console.print(
                f"[yellow]  {tool_name}: {current}/{total} ({pct:.1f}%)[/yellow] {status}",
                end="\r",
            )
        else:
            self.console.print(
                f"[yellow]  {tool_name}: {current} items found...[/yellow] {status}",
                end="\r",
            )

    def show_tool_complete(
        self,
        tool_name: str,
        results: Any,
        result_type: str = "items",
    ) -> None:
        """Print a completion summary for *tool_name*.

        Args:
            tool_name: Name of the completed tool.
            results: The result collection (used to derive a count).
            result_type: Human-readable label for the result unit.
        """
        if not self.console or self.quiet:
            return

        count = len(results) if isinstance(results, (list, dict)) else results
        self.console.print(
            f"\n[bold green]✅ {tool_name} completed:[/bold green] "
            f"[bold]{count} {result_type}[/bold]"
        )

    # ------------------------------------------------------------------
    # Live result tables
    # ------------------------------------------------------------------

    def show_live_results(
        self,
        tool_name: str,
        results: List[Dict[str, Any]],
        max_display: int = 10,
    ) -> None:
        """Render a Rich table with the most recent results.

        Supported tools: ``subfinder``, ``httpx``, ``nuclei``.
        Other tools are silently ignored.

        Args:
            tool_name: Name of the tool whose results are displayed.
            results: List of normalized result dicts.
            max_display: Maximum number of rows to show.
        """
        if not self.console or self.quiet or not results:
            return

        try:
            if tool_name == "subfinder":
                self._table_subfinder(results, max_display)
            elif tool_name == "httpx":
                self._table_httpx(results, max_display)
            elif tool_name == "nuclei":
                self._table_nuclei(results, max_display)
        except Exception:
            # Never crash the scan because of a display error
            pass

    def _table_subfinder(self, results: List[Dict[str, Any]], max_display: int) -> None:
        shown = min(len(results), max_display)
        table = Table(
            title=f"🔍 Discovered Subdomains (showing {shown}/{len(results)})",
            box=box.SIMPLE,
        )
        table.add_column("#", style="dim", width=4)
        table.add_column("Subdomain", style="cyan")
        for idx, item in enumerate(results[:max_display], 1):
            subdomain = item.get("subdomain", str(item)) if isinstance(item, dict) else str(item)
            table.add_row(str(idx), subdomain)
        if len(results) > max_display:
            table.add_row("...", f"[dim]... and {len(results) - max_display} more[/dim]")
        self.console.print(table)  # type: ignore[union-attr]

    def _table_httpx(self, results: List[Dict[str, Any]], max_display: int) -> None:
        shown = min(len(results), max_display)
        table = Table(
            title=f"🌐 Live Hosts (showing {shown}/{len(results)})",
            box=box.SIMPLE,
        )
        table.add_column("#", style="dim", width=4)
        table.add_column("URL", style="green", overflow="fold")
        table.add_column("Status", style="yellow", width=6)
        table.add_column("Title", style="blue", overflow="fold")
        for idx, result in enumerate(results[:max_display], 1):
            table.add_row(
                str(idx),
                result.get("url", "")[:60],
                str(result.get("status_code", "")),
                result.get("title", "")[:40],
            )
        if len(results) > max_display:
            table.add_row("...", f"[dim]... and {len(results) - max_display} more[/dim]", "", "")
        self.console.print(table)  # type: ignore[union-attr]

    def _table_nuclei(self, results: List[Dict[str, Any]], max_display: int) -> None:
        shown = min(len(results), max_display)
        table = Table(
            title=f"🔬 Vulnerabilities Found (showing {shown}/{len(results)})",
            box=box.SIMPLE,
        )
        table.add_column("#", style="dim", width=4)
        table.add_column("Severity", style="red")
        table.add_column("Name", style="cyan", overflow="fold")
        table.add_column("URL", style="green", overflow="fold")
        for idx, result in enumerate(results[:max_display], 1):
            table.add_row(
                str(idx),
                result.get("severity", "info").upper(),
                result.get("name", "Unknown")[:40],
                result.get("url", "")[:50],
            )
        if len(results) > max_display:
            table.add_row("...", f"[dim]... and {len(results) - max_display} more[/dim]", "", "")
        self.console.print(table)  # type: ignore[union-attr]

    # ------------------------------------------------------------------
    # Background file monitor
    # ------------------------------------------------------------------

    def monitor_file_and_show_results(
        self,
        tool_name: str,
        output_file: Path,
        parser_func: Any,
        update_interval: float = 5.0,
        max_display: int = 10,
    ) -> None:
        """Poll *output_file* for growth and stream results as they arrive.

        Intended to be called from a background thread while the tool
        subprocess is running.

        Args:
            tool_name: Name of the tool being monitored.
            output_file: Path to the file being written by the tool.
            parser_func: Callable that accepts raw file content and returns
                         a list of result dicts.
            update_interval: Seconds between file-size checks.
            max_display: Maximum rows to show in the live table.
        """
        if not self.console or self.quiet:
            return

        last_size = 0
        last_count = 0
        start_time = time.time()

        while True:
            try:
                if not output_file.exists():
                    time.sleep(1)
                    continue

                current_size = output_file.stat().st_size
                if current_size > last_size:
                    content = output_file.read_text()
                    if content.strip():
                        parsed = parser_func(content)
                        current_count = len(parsed) if isinstance(parsed, list) else 0
                        elapsed = int(time.time() - start_time)

                        self.show_tool_progress(
                            tool_name,
                            current_count,
                            None,
                            f"[dim]({elapsed}s elapsed)[/dim]",
                        )

                        if current_count > last_count and (
                            current_count - last_count >= 5 or elapsed % 10 == 0
                        ):
                            self.show_live_results(tool_name, parsed, max_display)
                            last_count = current_count

                        last_size = current_size

                time.sleep(update_interval)

            except KeyboardInterrupt:
                break
            except Exception:
                time.sleep(update_interval)
