# Contributing to Reconner

Thank you for your interest in contributing! This document outlines the process for contributing to Reconner.

---

## Code of Conduct

By participating in this project, you agree to abide by our [Code of Conduct](CODE_OF_CONDUCT.md).

---

## How to Contribute

### Reporting Bugs

1. Check the [existing issues](https://github.com/yourusername/reconner/issues) to avoid duplicates.
2. Open a new issue using the **Bug Report** template.
3. Include as much detail as possible: OS, Python version, tool versions, and steps to reproduce.

### Suggesting Features

1. Open a new issue using the **Feature Request** template.
2. Describe the use case and expected behavior clearly.

### Submitting Pull Requests

```bash
# 1. Fork and clone
git clone https://github.com/yourusername/reconner.git
cd reconner

# 2. Create a feature branch
git checkout -b feat/your-feature-name

# 3. Install dev dependencies
pip install -r requirements-dev.txt
pre-commit install

# 4. Make your changes and write tests
pytest

# 5. Format and lint
black reconner/
ruff reconner/

# 6. Commit and push
git commit -m "feat: describe your change"
git push origin feat/your-feature-name

# 7. Open a Pull Request on GitHub
```

---

## Development Setup

```bash
# Clone the repository
git clone https://github.com/yourusername/reconner.git
cd reconner

# Create a virtual environment
python3 -m venv venv
source venv/bin/activate

# Install in editable mode with dev dependencies
pip install -e .
pip install -r requirements-dev.txt

# Install pre-commit hooks
pre-commit install
```

---

## Code Style

- **Formatter:** [Black](https://black.readthedocs.io/) — `black reconner/`
- **Linter:** [Ruff](https://docs.astral.sh/ruff/) — `ruff reconner/`
- **Type hints:** encouraged for all public functions
- **Docstrings:** Google-style, in English

---

## Testing

```bash
# Run all tests
pytest

# With coverage report
pytest --cov=reconner --cov-report=html

# Run only smoke tests
pytest tests/test_cli_smoke.py -v
```

All new features must include tests. Bug fixes should include a regression test.

---

## Areas Where Help is Needed

- [ ] Windows support
- [ ] Docker image
- [ ] Additional parser modules (amass, nmap, ffuf)
- [ ] Web dashboard UI
- [ ] CI/CD pipeline hardening
- [ ] More nuclei template filters

---

## Commit Message Convention

We follow [Conventional Commits](https://www.conventionalcommits.org/):

```
feat: add new parser for amass output
fix: handle gobuster timeout gracefully
docs: update installation guide
chore: bump dependencies
test: add coverage for parsers module
```

---

## License

By contributing, you agree that your contributions will be licensed under the [MIT License](LICENSE).
