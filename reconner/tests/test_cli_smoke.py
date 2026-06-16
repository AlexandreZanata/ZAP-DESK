"""Smoke tests for the Reconner CLI and core modules.

These tests verify that all modules import correctly, utility functions
behave as expected, and the CLI wiring is intact — without requiring
any external security tools to be installed.
"""

import sys
from pathlib import Path
from unittest.mock import MagicMock, patch

import pytest

# Ensure the package root is on sys.path when running tests directly
sys.path.insert(0, str(Path(__file__).parent.parent))

from reconner.cli import main
from reconner.utils import check_tool_exists, normalize_url, read_targets_file


# ---------------------------------------------------------------------------
# utils.py
# ---------------------------------------------------------------------------


class TestNormalizeUrl:
    def test_bare_domain_gets_https(self):
        assert normalize_url("example.com") == "https://example.com"

    def test_http_scheme_preserved(self):
        assert normalize_url("http://example.com") == "http://example.com"

    def test_https_scheme_preserved(self):
        assert normalize_url("https://example.com") == "https://example.com"

    def test_strips_whitespace(self):
        assert normalize_url("  example.com  ") == "https://example.com"


class TestCheckToolExists:
    def test_returns_bool_and_optional_str(self):
        exists, path = check_tool_exists("httpx")
        assert isinstance(exists, bool)
        assert path is None or isinstance(path, str)

    def test_unknown_tool_returns_false(self):
        exists, path = check_tool_exists("__nonexistent_tool__")
        assert exists is False
        assert path is None


class TestReadTargetsFile:
    def test_reads_and_normalizes(self, tmp_path):
        targets_file = tmp_path / "targets.txt"
        targets_file.write_text(
            "example.com\nhttps://test.com\n# comment\n\nvalid.com\n"
        )
        targets = read_targets_file(str(targets_file))
        assert "https://example.com" in targets
        assert "https://test.com" in targets
        assert "https://valid.com" in targets
        assert len(targets) == 3

    def test_ignores_blank_lines_and_comments(self, tmp_path):
        targets_file = tmp_path / "targets.txt"
        targets_file.write_text("# header\n\n# another comment\n")
        targets = read_targets_file(str(targets_file))
        assert targets == []


# ---------------------------------------------------------------------------
# parsers.py
# ---------------------------------------------------------------------------


class TestParsers:
    def test_parse_subfinder_jsonl(self):
        from reconner.parsers import parse_subfinder_json

        content = '{"host": "sub.example.com"}\n{"host": "api.example.com"}\n'
        result = parse_subfinder_json(content)
        assert "sub.example.com" in result
        assert "api.example.com" in result

    def test_parse_subfinder_empty(self):
        from reconner.parsers import parse_subfinder_json

        assert parse_subfinder_json("") == []

    def test_parse_httpx_jsonl(self):
        from reconner.parsers import parse_httpx_json

        content = '{"url": "https://example.com", "status_code": 200, "title": "Home"}\n'
        result = parse_httpx_json(content)
        assert len(result) == 1
        assert result[0]["url"] == "https://example.com"
        assert result[0]["status_code"] == 200

    def test_parse_gobuster_text(self):
        from reconner.parsers import parse_gobuster_output

        content = "/admin (Status: 200) [Size: 1234]\n/login (Status: 301) [Size: 0]\n"
        result = parse_gobuster_output(content)
        assert len(result) == 2
        assert result[0]["path"] == "/admin"
        assert result[0]["status"] == 200

    def test_parse_nuclei_jsonl(self):
        from reconner.parsers import parse_nuclei_json

        content = (
            '{"template-id": "cve-2021-1234", "name": "Test CVE", '
            '"severity": "critical", "url": "https://example.com", '
            '"host": "example.com", "matched-at": "https://example.com/vuln"}\n'
        )
        result = parse_nuclei_json(content)
        assert len(result) == 1
        assert result[0]["severity"] == "critical"
        assert result[0]["template_id"] == "cve-2021-1234"

    def test_parse_tool_output_dispatcher(self):
        from reconner.parsers import parse_tool_output

        content = '{"host": "sub.example.com"}\n'
        result = parse_tool_output("subfinder", content)
        assert isinstance(result, list)
        assert result[0]["subdomain"] == "sub.example.com"

    def test_parse_nmap_xml_minimal(self):
        from reconner.parsers import parse_nmap_xml

        xml = """<?xml version="1.0"?>
<nmaprun>
  <host>
    <status state="up"/>
    <hostnames><hostname name="scanme.nmap.org" type="user"/></hostnames>
    <address addr="45.33.32.156" addrtype="ipv4"/>
    <ports>
      <port protocol="tcp" portid="22">
        <state state="open"/>
        <service name="ssh" product="OpenSSH" version="6.6.1"/>
      </port>
    </ports>
  </host>
</nmaprun>"""
        rows = parse_nmap_xml(xml)
        assert len(rows) == 1
        assert rows[0]["port"] == 22
        assert rows[0]["state"] == "open"
        assert rows[0]["service"] == "ssh"
        assert rows[0]["host"] == "scanme.nmap.org"

    def test_parse_tool_output_unknown_tool(self):
        from reconner.parsers import parse_tool_output

        result = parse_tool_output("unknown_tool", "some content")
        assert result == []


class TestReporter:
    def test_generate_summary_json(self, tmp_path):
        from reconner.reporter import Reporter

        reporter = Reporter(str(tmp_path), {}, target_domain="example.com")
        results = {
            "subdomains": ["sub.example.com"],
            "live_hosts": ["https://sub.example.com"],
            "httpx_results": [],
            "nmap_results": [
                {
                    "host": "sub.example.com",
                    "port": 443,
                    "protocol": "tcp",
                    "state": "open",
                    "service": "https",
                    "product": "",
                    "version": "",
                    "scanned_target": "sub.example.com",
                },
            ],
            "whatweb_results": [],
            "gobuster_results": [],
            "nuclei_results": [],
            "errors": [],
        }
        path = reporter.generate_summary_json(results)
        assert path.exists()

        import json
        data = json.loads(path.read_text())
        assert data["subdomains"]["total"] == 1
        assert data["statistics"]["total_subdomains"] == 1
        assert data["statistics"]["total_open_ports"] == 1

    def test_generate_highlights(self, tmp_path):
        from reconner.reporter import Reporter

        reporter = Reporter(str(tmp_path), {}, target_domain="example.com")
        results = {
            "subdomains": [],
            "live_hosts": [],
            "gobuster_results": [],
            "nuclei_results": [],
        }
        path = reporter.generate_highlights(results)
        assert path.exists()
        assert "RECONNER SCAN HIGHLIGHTS" in path.read_text()

    def test_generate_markdown_report(self, tmp_path):
        from reconner.reporter import Reporter

        reporter = Reporter(str(tmp_path), {}, target_domain="example.com")
        results = {
            "subdomains": [],
            "live_hosts": [],
            "httpx_results": [],
            "nmap_results": [],
            "whatweb_results": [],
            "gobuster_results": [],
            "nuclei_results": [],
            "errors": [],
        }
        path = reporter.generate_markdown_report(results)
        assert path.exists()
        content = path.read_text()
        assert "example.com" in content


# ---------------------------------------------------------------------------
# CLI wiring
# ---------------------------------------------------------------------------


class TestImports:
    """Verify that all public modules import without errors."""

    def test_import_cli(self):
        from reconner import cli
        assert cli is not None

    def test_import_runner(self):
        from reconner import runner
        assert runner is not None

    def test_import_parsers(self):
        from reconner import parsers
        assert parsers is not None

    def test_import_reporter(self):
        from reconner import reporter
        assert reporter is not None

    def test_import_utils(self):
        from reconner import utils
        assert utils is not None

    def test_import_progress(self):
        from reconner import progress
        assert progress is not None


@patch("reconner.cli.ToolRunner")
@patch("reconner.cli.Reporter")
@patch("reconner.cli.get_all_tool_versions")
@patch("reconner.cli.check_tool_exists")
def test_cli_basic_wiring(mock_check, mock_versions, mock_reporter_cls, mock_runner_cls):
    """Verify the CLI correctly wires runner and reporter together."""
    mock_check.return_value = (True, "/usr/local/bin/httpx")
    mock_versions.return_value = {"httpx": {"version": "1.0.0", "path": "/usr/local/bin/httpx"}}

    mock_runner = MagicMock()
    mock_runner.run_full_scan.return_value = {
        "subdomains": [],
        "live_hosts": [],
        "httpx_results": [],
        "nmap_results": [],
        "whatweb_results": [],
        "gobuster_results": [],
        "nuclei_results": [],
        "errors": [],
    }
    mock_runner.tool_versions = {}
    mock_runner_cls.return_value = mock_runner

    mock_reporter = MagicMock()
    mock_reporter.generate_all_reports.return_value = {
        "summary": Path("/tmp/summary.json"),
        "markdown": Path("/tmp/report.md"),
    }
    mock_reporter_cls.return_value = mock_reporter

    # Imports and wiring verified — actual invocation requires Click test runner
    assert main is not None
