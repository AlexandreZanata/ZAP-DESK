# Contributing

See [CONTRIBUTING.md](../CONTRIBUTING.md) for the full contribution guide.

## Quick Reference

```bash
# Fork, clone, and set up
git clone https://github.com/yourusername/reconner.git
cd reconner
pip install -e .
pip install -r requirements-dev.txt
pre-commit install

# Create a branch
git checkout -b feat/your-feature

# Run tests
pytest

# Format and lint
black reconner/
ruff reconner/

# Submit PR
git push origin feat/your-feature
```

## Areas Where Help is Needed

- [ ] Windows support
- [ ] Docker image
- [ ] Additional parsers (amass, nmap, ffuf)
- [ ] Web dashboard UI
- [ ] CI/CD pipeline hardening
- [ ] More nuclei template filters
