# Safety guide (for analysis only)

This repository contains malware-capable code. Use this guide to review it safely.

## Safe analysis checklist

- Use a **disposable virtual machine** with snapshots enabled.
- Ensure the VM contains **no sensitive files** and no mapped host folders.
- Prefer **no network** (or a tightly controlled, instrumented network) during dynamic analysis.
- Do not execute unknown files from `dist/` or `build/`.
- Keep backups and restore points **outside** the VM if you must test anything.

## High-level behaviors to watch (defender view)

From reading `ransomware.py`, the following behaviors are present:

- **File encryption + deletion**: reads file bytes, writes encrypted bytes to a new file name, then deletes the original.
- **Recursive traversal**: walks a directory tree and processes all files found.
- **User-impact actions**: writes a ransom note and changes desktop wallpaper.
- **Time-based notification**: delays before a warning dialog (social pressure).
- **Process/memory execution technique**: allocates memory, copies a byte payload into it, marks it executable, and starts a new thread (Windows API).

These are common detection anchors for EDR/SIEM rules (file rename patterns, high-volume file IO, entropy spikes, WinAPI `VirtualAlloc`/`VirtualProtect`/`CreateThread` call chains).

## Indicators of compromise (IOCs) in this workspace

The script references:

- **Ransom note name**: `READ_ME.txt` (on the Desktop)
- **Encrypted extension**: adds `.locked` to file names
- **Wallpaper file name**: `wallpaper.jpg`

Treat these as **workspace-specific strings** useful for lab detection exercises, not as exhaustive ransomware signatures.

## What this documentation will not include

To avoid enabling harm, this project documentation intentionally does **not** include:

- steps to execute the ransomware script
- steps to package it into an executable
- guidance on persistence, spreading, evasion, or payload delivery

If you want, I can help you rewrite this repository so it’s a **purely defensive lab** (e.g., keep only harmless “simulation mode” that logs what *would* happen without touching files).

