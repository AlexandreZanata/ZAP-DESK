"""Tests for ZAP-DESK integration hooks in the reconner runner."""

import os
import sys
from pathlib import Path
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).parent.parent))

from reconner.runner import ToolRunner


class TestZapDeskProgress:
    def test_desk_phase_emits_machine_readable_line(self, capsys):
        runner = ToolRunner(output_dir="/tmp/reconner-test")
        runner.quiet = True

        with patch.dict(os.environ, {"ZAP_DESK_MODE": "1"}):
            runner._desk_phase("subfinder", 1, 6, "Discovering subdomains")

        captured = capsys.readouterr().out.strip()
        assert captured.startswith("@@ZAP-DESK@@PHASE|")
        assert "|subfinder|" in captured

    def test_desk_phase_silent_without_env(self, capsys):
        runner = ToolRunner(output_dir="/tmp/reconner-test")
        runner.quiet = True

        env = os.environ.copy()
        env.pop("ZAP_DESK_MODE", None)
        with patch.dict(os.environ, env, clear=True):
            runner._desk_phase("httpx", 2, 6, "Probing targets")

        assert capsys.readouterr().out == ""
