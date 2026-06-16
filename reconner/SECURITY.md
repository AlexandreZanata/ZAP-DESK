# Security Policy

## Supported Versions

| Version | Supported          |
| ------- | ------------------ |
| 2.x     | :white_check_mark: |
| 1.x     | :x:                |

## Reporting a Vulnerability

**Please do NOT open a public GitHub issue for security vulnerabilities.**

If you discover a security vulnerability in Reconner itself (not in the tools it orchestrates), please report it responsibly:

1. **Email:** Send a detailed report to the maintainers via the contact listed on the GitHub profile.
2. **Include:**
   - A description of the vulnerability
   - Steps to reproduce
   - Potential impact
   - Suggested fix (if any)

3. **Response time:** We aim to acknowledge reports within 48 hours and provide a fix or mitigation within 14 days for critical issues.

4. **Disclosure:** We follow coordinated disclosure. Please allow us time to patch before public disclosure.

---

## Scope

This policy covers vulnerabilities **in the Reconner codebase itself**, such as:

- Command injection via user-supplied input
- Path traversal in output directory handling
- Insecure deserialization of tool outputs
- Privilege escalation

Vulnerabilities in the external tools orchestrated by Reconner (subfinder, httpx, nuclei, gobuster, whatweb) should be reported to their respective maintainers.

---

## Legal Notice

Reconner is designed for **authorized security testing only**. Any use of this tool against systems without explicit written permission is illegal. The authors assume zero liability for misuse.
