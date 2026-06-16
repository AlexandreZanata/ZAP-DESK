# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [2.0.0] - 2026-05-08

### Added
- Full project restructure following professional open-source standards
- `docs/` directory with extended documentation (installation, usage, contributing)
- `.github/` directory with CI workflow and issue templates
- `CONTRIBUTING.md` — contribution guidelines
- `CODE_OF_CONDUCT.md` — community standards
- `SECURITY.md` — vulnerability disclosure policy
- `CHANGELOG.md` — version history (this file)
- `requirements-dev.txt` — development dependencies
- Report regeneration via `--export-only` flag
- Stealth mode with rate-limited, randomized timing
- Multi-target bulk scanning via `--input-file`
- Live progress bars and real-time result streaming (Rich)
- Organized output directory: `domain - DD_MM_YYYY - HH:MMam`
- `discoveries.txt` output file with key findings summary
- PDF report generation via ReportLab (with weasyprint/pandoc fallback)

### Changed
- Version bumped to `2.0.0`
- All comments, docstrings, and documentation rewritten in English
- README completely rewritten following the new professional template
- `setup.py` updated with new version and metadata
- `install-tools.sh` rewritten in English
- `example-run.sh` rewritten in English
- Wordlists `README.md` rewritten in English

### Fixed
- Gobuster parallel execution stability
- PDF generation HTML tag stripping edge cases
- Tool version detection fallback logic

---

## [1.0.0] - 2025-12-25

### Added
- Initial release
- Subfinder, httpx, whatweb, gobuster, nuclei orchestration
- JSON, Markdown, and PDF report generation
- Rich-based progress visualization
- Proxy support
- Fast mode and stealth mode flags
