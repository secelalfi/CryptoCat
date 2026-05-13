# Project overview

This workspace combines:

1. `dnstwist/` (defensive): a domain-permutation generator and verifier used for **typosquatting/phishing detection**.
2. `ransomware.py` (offensive/malware-capable): a ransomware-like script that performs **local file encryption** and other disruptive actions.

This document focuses on **structure and intent** to support safe review, code auditing, and defensive learning.

## Defensive tooling (`dnstwist/`)

`dnstwist` is a defensive security tool that:

- generates domain permutations using multiple fuzzing algorithms
- checks which permutations are registered / have DNS records
- can optionally enrich results (e.g., GeoIP) and detect phishing similarity signals

Reference documentation is already present at `dnstwist/docs/README.md`.

Notable files:

- `dnstwist/dnstwist.py`: primary CLI/tool implementation
- `dnstwist/webapp/webapp.py`: web UI entrypoint (if used)
- `dnstwist/requirements.txt`: dependency set used in this workspace

## Malware-capable code (`ransomware.py`)

`ransomware.py` contains behaviors commonly associated with ransomware. At a high level, it:

- generates a random AES key at runtime
- recursively processes files in the user’s Desktop directory
- writes encrypted data to new files (adds a new extension)
- deletes the original files after writing encrypted versions
- drops a ransom note to the Desktop
- changes the desktop wallpaper
- uses Windows API calls to allocate memory and create a thread for a byte payload

This code is **dangerous** to execute on a real system containing valuable data.
For defensive analysis guidance, see `docs/SAFETY.md`.

## Build artifacts and packaging

- `ransomware.spec`: a PyInstaller spec referencing `ransomware.py`. Treat anything produced from it (e.g. under `dist/`) as potentially malicious.
- `build/`, `dist/`: commonly used for build outputs; do not trust contents by default.

