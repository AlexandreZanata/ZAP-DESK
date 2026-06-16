# Wordlists Directory

This directory is a placeholder for custom wordlists used by Reconner
(specifically by the gobuster directory enumeration phase).

By default, Reconner looks for wordlists in `/usr/share/seclists`.
Override this with the `--wordlists-dir` option.

---

## Recommended Layout

```
wordlists/
├── Discovery/
│   ├── Web-Content/
│   │   ├── common.txt                      # Small, fast — used in --fast mode
│   │   ├── directory-list-2.3-medium.txt   # Default for normal scans
│   │   ├── directory-list-2.3-big.txt      # Comprehensive (slow)
│   │   ├── raft-small-files.txt            # Sensitive file names
│   │   └── raft-small-directories.txt      # Common directory names
│   └── DNS/
│       ├── subdomains-top1million-5000.txt
│       └── subdomains-top1million-110000.txt
```

---

## Installing SecLists (Recommended)

```bash
# Debian / Ubuntu
sudo apt install seclists

# Or clone from GitHub (~1 GB)
sudo git clone --depth 1 https://github.com/danielmiessler/SecLists.git /usr/share/seclists
```

---

## Using a Custom Wordlist Directory

```bash
reconner --target example.com --wordlists-dir ./wordlists
```

Place your wordlists under `wordlists/Discovery/Web-Content/` to match
the default path resolution logic, or pass the full path directly.
