"""Report generation for Reconner.

The :class:`Reporter` class reads normalized scan results and produces:

- ``summary.json``  — machine-readable structured data
- ``report.md``     — human-readable Markdown report (Jinja2 template)
- ``report.pdf``    — PDF version (weasyprint → pandoc → reportlab fallback)
- ``highlights.txt`` — quick executive summary
"""

import json
import logging
import re
import subprocess
from datetime import datetime
from pathlib import Path
from typing import Any, Dict, List, Optional

from jinja2 import Template

logger = logging.getLogger(__name__)


class Reporter:
    """Generates all output reports from normalized scan results.

    Args:
        output_dir: Directory where reports are written.
        tool_versions: Mapping of tool name → version info dict.
        target_domain: Primary target domain (used in report headers).
    """

    def __init__(
        self,
        output_dir: str,
        tool_versions: Dict[str, Any],
        target_domain: Optional[str] = None,
    ) -> None:
        self.output_dir = Path(output_dir)
        self.tool_versions = tool_versions
        self.target_domain = target_domain or "Unknown"
        self.summary_data: Dict[str, Any] = {}

    # ------------------------------------------------------------------
    # summary.json
    # ------------------------------------------------------------------

    def generate_summary_json(self, results: Dict[str, Any]) -> Path:
        """Write a structured ``summary.json`` from *results*.

        Args:
            results: Normalized scan results dict from :class:`~reconner.runner.ToolRunner`.

        Returns:
            Path to the written file.
        """
        summary: Dict[str, Any] = {
            "scan_info": {
                "timestamp": datetime.now().isoformat(),
                "target": self.target_domain,
                "tool_versions": self.tool_versions,
            },
            "subdomains": {
                "total": len(results.get("subdomains", [])),
                "list": results.get("subdomains", []),
            },
            "live_hosts": {
                "total": len(results.get("live_hosts", [])),
                "list": results.get("live_hosts", []),
            },
            "httpx_results": results.get("httpx_results", []),
            "nmap_results": results.get("nmap_results", []),
            "whatweb_results": results.get("whatweb_results", []),
            "gobuster_results": results.get("gobuster_results", []),
            "nuclei_results": results.get("nuclei_results", []),
            "errors": results.get("errors", []),
            "statistics": self._build_statistics(results),
        }

        path = self.output_dir / "summary.json"
        path.write_text(json.dumps(summary, indent=2))
        self.summary_data = summary
        logger.info("Generated summary.json at %s", path)
        return path

    def _build_statistics(self, results: Dict[str, Any]) -> Dict[str, int]:
        """Compute aggregate statistics from *results*."""
        nuclei = results.get("nuclei_results", [])
        nmap = results.get("nmap_results", [])
        open_ports = sum(1 for r in nmap if r.get("state") == "open")
        return {
            "total_subdomains": len(results.get("subdomains", [])),
            "total_live_hosts": len(results.get("live_hosts", [])),
            "total_technologies": len(self._extract_technologies(results)),
            "total_paths_found": len(results.get("gobuster_results", [])),
            "total_open_ports": open_ports,
            "total_vulnerabilities": len(nuclei),
            "critical_findings": sum(1 for r in nuclei if r.get("severity") == "critical"),
            "high_findings": sum(1 for r in nuclei if r.get("severity") == "high"),
            "medium_findings": sum(1 for r in nuclei if r.get("severity") == "medium"),
        }

    def _extract_technologies(self, results: Dict[str, Any]) -> List[str]:
        """Collect unique technology names from httpx and whatweb results."""
        techs: set = set()

        for r in results.get("httpx_results", []):
            techs.update(r.get("tech", []))
            if r.get("server"):
                techs.add(r["server"])

        for r in results.get("whatweb_results", []):
            plugins = r.get("plugins", {})
            if isinstance(plugins, dict):
                techs.update(plugins.keys())

        return sorted(techs)

    # ------------------------------------------------------------------
    # highlights.txt
    # ------------------------------------------------------------------

    def generate_highlights(self, results: Dict[str, Any]) -> Path:
        """Write a plain-text executive summary to ``highlights.txt``.

        Args:
            results: Normalized scan results dict.

        Returns:
            Path to the written file.
        """
        stats = self._build_statistics(results)
        lines: List[str] = [
            "=" * 80,
            "RECONNER SCAN HIGHLIGHTS",
            "=" * 80,
            f"Scan Date : {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}",
            f"Target    : {self.target_domain}",
            "",
            "STATISTICS:",
            f"  Subdomains found  : {len(results.get('subdomains', []))}",
            f"  Live hosts        : {len(results.get('live_hosts', []))}",
            f"  Open ports (nmap) : {stats.get('total_open_ports', 0)}",
            f"  Paths discovered  : {len(results.get('gobuster_results', []))}",
            f"  Vulnerabilities   : {len(results.get('nuclei_results', []))}",
            "",
        ]

        # Critical findings
        critical = [r for r in results.get("nuclei_results", []) if r.get("severity") == "critical"]
        if critical:
            lines.append("CRITICAL FINDINGS:")
            for finding in critical[:10]:
                lines.append(f"  - {finding.get('name', 'Unknown')} at {finding.get('url', 'N/A')}")
            lines.append("")

        # High findings
        high = [r for r in results.get("nuclei_results", []) if r.get("severity") == "high"]
        if high:
            lines.append("HIGH SEVERITY FINDINGS:")
            for finding in high[:10]:
                lines.append(f"  - {finding.get('name', 'Unknown')} at {finding.get('url', 'N/A')}")
            lines.append("")

        # Interesting paths
        sensitive_keywords = {"admin", "api", "backup", "config", "test", "dev", "secret"}
        interesting = [
            r for r in results.get("gobuster_results", [])
            if any(kw in r.get("path", "").lower() for kw in sensitive_keywords)
        ]
        if interesting:
            lines.append("INTERESTING PATHS:")
            for p in interesting[:15]:
                lines.append(f"  - {p.get('path', 'N/A')} (Status: {p.get('status', 'N/A')})")
            lines.append("")

        lines += [
            "=" * 80,
            "For full details see summary.json and report.md",
            "=" * 80,
        ]

        path = self.output_dir / "highlights.txt"
        path.write_text("\n".join(lines))
        logger.info("Generated highlights.txt at %s", path)
        return path

    # ------------------------------------------------------------------
    # report.md
    # ------------------------------------------------------------------

    def generate_markdown_report(self, results: Dict[str, Any]) -> Path:
        """Render the Jinja2 Markdown report template and write ``report.md``.

        Args:
            results: Normalized scan results dict.

        Returns:
            Path to the written file.
        """
        template_path = Path(__file__).parent / "templates" / "report.md.j2"
        template_content = (
            template_path.read_text() if template_path.exists() else self._default_template()
        )

        template = Template(template_content)

        _sev_rank = {"critical": 3, "high": 2, "medium": 1, "low": 0, "info": -1}

        data = {
            "scan_date": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
            "target_domain": self.target_domain,
            "tool_versions": self.tool_versions,
            "statistics": {
                "subdomains": len(results.get("subdomains", [])),
                "live_hosts": len(results.get("live_hosts", [])),
                "technologies": len(self._extract_technologies(results)),
                "paths": len(results.get("gobuster_results", [])),
                "open_ports": sum(
                    1 for r in results.get("nmap_results", []) if r.get("state") == "open"
                ),
                "vulnerabilities": len(results.get("nuclei_results", [])),
                "critical": sum(
                    1 for r in results.get("nuclei_results", []) if r.get("severity") == "critical"
                ),
                "high": sum(
                    1 for r in results.get("nuclei_results", []) if r.get("severity") == "high"
                ),
                "medium": sum(
                    1 for r in results.get("nuclei_results", []) if r.get("severity") == "medium"
                ),
            },
            "subdomains": results.get("subdomains", [])[:50],
            "live_hosts": results.get("live_hosts", [])[:50],
            "httpx_results": results.get("httpx_results", [])[:100],
            "nmap_results": sorted(
                [r for r in results.get("nmap_results", []) if r.get("state") == "open"],
                key=lambda x: (x.get("host", ""), x.get("port", 0)),
            )[:200],
            "whatweb_results": results.get("whatweb_results", []),
            "gobuster_results": sorted(
                results.get("gobuster_results", []),
                key=lambda x: x.get("status", 0),
                reverse=True,
            )[:100],
            "nuclei_results": sorted(
                results.get("nuclei_results", []),
                key=lambda x: _sev_rank.get(x.get("severity", "info"), -1),
                reverse=True,
            ),
            "technologies": self._extract_technologies(results),
            "errors": results.get("errors", []),
        }

        content = template.render(**data)
        path = self.output_dir / "report.md"
        path.write_text(content)
        logger.info("Generated report.md at %s", path)
        return path

    def _default_template(self) -> str:
        """Return a minimal fallback Markdown template."""
        return """\
# Reconnaissance Report — {{ target_domain }}

**Scan Date:** {{ scan_date }}

## Executive Summary

| Metric | Count |
|--------|-------|
| Subdomains | {{ statistics.subdomains }} |
| Live Hosts | {{ statistics.live_hosts }} |
| Technologies | {{ statistics.technologies }} |
| Paths Found | {{ statistics.paths }} |
| Open ports (nmap) | {{ statistics.open_ports }} |
| Vulnerabilities | {{ statistics.vulnerabilities }} |
| Critical | {{ statistics.critical }} |
| High | {{ statistics.high }} |
| Medium | {{ statistics.medium }} |

## Tool Versions

{% for tool, info in tool_versions.items() %}
- **{{ tool }}:** {{ info.version or 'Unknown' }}
{% endfor %}

## Live Hosts

{% for host in live_hosts %}
- {{ host }}
{% endfor %}

## Vulnerabilities

{% for finding in nuclei_results %}
### {{ finding.name }} ({{ finding.severity | upper }})
- **URL:** {{ finding.url }}
- **Template:** {{ finding.template_id }}
{% endfor %}

## Interesting Paths

{% for path in gobuster_results[:50] %}
- `{{ path.path }}` — Status: {{ path.status }}, Size: {{ path.size }}
{% endfor %}

## Legal Notice

This scan was performed for authorized security testing purposes only.
"""

    # ------------------------------------------------------------------
    # report.pdf
    # ------------------------------------------------------------------

    def convert_to_pdf(self, markdown_path: Path) -> Optional[Path]:
        """Convert *markdown_path* to a PDF report.

        Tries three backends in order:
        1. **weasyprint** (best quality)
        2. **pandoc** (system package)
        3. **reportlab** (pure-Python fallback)

        Args:
            markdown_path: Path to the Markdown report.

        Returns:
            Path to the generated PDF, or ``None`` if all backends fail.
        """
        pdf_path = self.output_dir / "report.pdf"

        if self._try_weasyprint(markdown_path, pdf_path):
            return pdf_path
        if self._try_pandoc(markdown_path, pdf_path):
            return pdf_path
        if self._try_reportlab(markdown_path, pdf_path):
            return pdf_path

        logger.error("All PDF backends failed — no PDF generated")
        return None

    def _try_weasyprint(self, md_path: Path, pdf_path: Path) -> bool:
        try:
            from weasyprint import HTML
            from markdown import markdown

            html = markdown(md_path.read_text(), extensions=["extra", "codehilite"])
            HTML(string=html).write_pdf(str(pdf_path))
            logger.info("PDF generated via weasyprint at %s", pdf_path)
            return True
        except ImportError:
            logger.debug("weasyprint not available")
        except Exception as exc:
            logger.debug("weasyprint failed: %s", exc)
        return False

    def _try_pandoc(self, md_path: Path, pdf_path: Path) -> bool:
        try:
            result = subprocess.run(
                ["pandoc", str(md_path), "-o", str(pdf_path)],
                capture_output=True,
                timeout=30,
            )
            if result.returncode == 0:
                logger.info("PDF generated via pandoc at %s", pdf_path)
                return True
        except FileNotFoundError:
            logger.debug("pandoc not available")
        except Exception as exc:
            logger.debug("pandoc failed: %s", exc)
        return False

    def _try_reportlab(self, md_path: Path, pdf_path: Path) -> bool:
        """Generate a PDF using ReportLab as a pure-Python fallback."""
        try:
            from reportlab.lib import colors
            from reportlab.lib.enums import TA_CENTER, TA_JUSTIFY
            from reportlab.lib.pagesizes import letter
            from reportlab.lib.styles import ParagraphStyle, getSampleStyleSheet
            from reportlab.lib.units import inch
            from reportlab.platypus import Paragraph, SimpleDocTemplate, Spacer

            md_content = md_path.read_text()
            styles = getSampleStyleSheet()

            title_style = ParagraphStyle(
                "RTitle",
                parent=styles["Heading1"],
                fontSize=22,
                textColor=colors.HexColor("#1a1a1a"),
                spaceAfter=24,
                alignment=TA_CENTER,
                fontName="Helvetica-Bold",
            )
            h1_style = ParagraphStyle(
                "RH1",
                parent=styles["Heading1"],
                fontSize=16,
                textColor=colors.HexColor("#2c3e50"),
                spaceAfter=10,
                spaceBefore=18,
                fontName="Helvetica-Bold",
            )
            h2_style = ParagraphStyle(
                "RH2",
                parent=styles["Heading2"],
                fontSize=13,
                textColor=colors.HexColor("#34495e"),
                spaceAfter=8,
                spaceBefore=14,
                fontName="Helvetica-Bold",
            )
            normal_style = ParagraphStyle(
                "RNormal",
                parent=styles["Normal"],
                fontSize=10,
                textColor=colors.HexColor("#2c3e50"),
                spaceAfter=5,
                leading=14,
                alignment=TA_JUSTIFY,
            )
            list_style = ParagraphStyle(
                "RList",
                parent=styles["Normal"],
                fontSize=10,
                textColor=colors.HexColor("#2c3e50"),
                spaceAfter=3,
                leading=14,
                leftIndent=18,
            )

            def _clean(text: str) -> str:
                """Strip HTML tags and decode common entities."""
                text = re.sub(r"<[^>]*>", "", text, flags=re.DOTALL)
                for entity, char in [
                    ("&amp;", "&"), ("&lt;", "<"), ("&gt;", ">"),
                    ("&quot;", '"'), ("&apos;", "'"), ("&nbsp;", " "),
                ]:
                    text = text.replace(entity, char)
                text = re.sub(r"&[a-zA-Z]+;", "", text)
                return re.sub(r"\s+", " ", text).strip()

            def _escape(text: str) -> str:
                return re.sub(r"&(?![a-zA-Z]+;)", "&amp;", text)

            story = []
            in_code = False
            code_lines: List[str] = []

            for i, raw_line in enumerate(md_content.splitlines()):
                line = raw_line.strip()

                if line.startswith("```"):
                    if in_code:
                        if code_lines:
                            cleaned = [
                                _clean(l).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")
                                for l in code_lines
                            ]
                            story.append(
                                Paragraph(
                                    "<font face='Courier' size='9' color='#27ae60'>"
                                    + "<br/>".join(cleaned)
                                    + "</font>",
                                    normal_style,
                                )
                            )
                            story.append(Spacer(1, 6))
                        code_lines = []
                        in_code = False
                    else:
                        in_code = True
                    continue

                if in_code:
                    code_lines.append(raw_line)
                    continue

                if not line:
                    story.append(Spacer(1, 4))
                    continue

                if line.startswith("# ") and i < 5:
                    story.append(Paragraph(_escape(_clean(line[2:])), title_style))
                    story.append(Spacer(1, 16))
                elif line.startswith("# "):
                    story.append(Paragraph(_escape(_clean(line[2:])), h1_style))
                elif line.startswith("## "):
                    story.append(Paragraph(_escape(_clean(line[3:])), h2_style))
                elif line.startswith("### "):
                    story.append(
                        Paragraph(f"<b>{_escape(_clean(line[4:]))}</b>", normal_style)
                    )
                elif line.startswith(("- ", "* ")):
                    text = _clean(line[2:])
                    text = re.sub(r"\*\*([^*]+)\*\*", r"<b>\1</b>", text)
                    text = re.sub(
                        r"`([^`]+)`",
                        r"<font face='Courier' size='9' color='#27ae60'>\1</font>",
                        text,
                    )
                    story.append(Paragraph(f"• {_escape(text)}", list_style))
                elif re.match(r"^\d+\.\s", line):
                    text = _clean(re.sub(r"^\d+\.\s", "", line))
                    text = re.sub(r"\*\*([^*]+)\*\*", r"<b>\1</b>", text)
                    story.append(Paragraph(f"• {_escape(text)}", list_style))
                elif line.startswith(("---", "===")):
                    story.append(Spacer(1, 8))
                else:
                    text = _clean(line)
                    text = re.sub(r"\*\*([^*]+)\*\*", r"<b>\1</b>", text)
                    text = re.sub(r"\*([^*]+)\*", r"<i>\1</i>", text)
                    text = re.sub(
                        r"`([^`]+)`",
                        r"<font face='Courier' size='9' color='#27ae60'>\1</font>",
                        text,
                    )
                    if text.strip():
                        story.append(Paragraph(_escape(text), normal_style))
                        story.append(Spacer(1, 3))

            doc = SimpleDocTemplate(
                str(pdf_path),
                pagesize=letter,
                rightMargin=0.75 * inch,
                leftMargin=0.75 * inch,
                topMargin=0.75 * inch,
                bottomMargin=0.75 * inch,
            )
            doc.build(story)
            logger.info("PDF generated via reportlab at %s", pdf_path)
            return True

        except Exception as exc:
            logger.error("reportlab PDF generation failed: %s", exc)
            return False

    # ------------------------------------------------------------------
    # Orchestrator
    # ------------------------------------------------------------------

    def generate_all_reports(self, results: Dict[str, Any]) -> Dict[str, Path]:
        """Generate all reports and return a mapping of name → path.

        Args:
            results: Normalized scan results dict.

        Returns:
            Dict with keys ``summary``, ``highlights``, ``markdown``,
            and optionally ``pdf``.
        """
        logger.info("Generating all reports...")
        reports: Dict[str, Path] = {}

        reports["summary"] = self.generate_summary_json(results)
        reports["highlights"] = self.generate_highlights(results)
        reports["markdown"] = self.generate_markdown_report(results)

        pdf = self.convert_to_pdf(reports["markdown"])
        if pdf:
            reports["pdf"] = pdf

        return reports
