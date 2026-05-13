
<div align="center">

# 🐱 CryptoCat — Ransomware Simulation

**A controlled, educational ransomware simulation built in C for Red Team training and cybersecurity research.**

![Platform](https://img.shields.io/badge/Platform-Windows-blue?style=flat-square&logo=windows)
![Language](https://img.shields.io/badge/Language-C%2FC%2B%2B-red?style=flat-square&logo=c)
![Purpose](https://img.shields.io/badge/Purpose-Educational%20%2F%20Red%20Team-orange?style=flat-square)
![License](https://img.shields.io/badge/License-MIT-green?style=flat-square)
![Status](https://img.shields.io/badge/Status-Simulation%20Only-critical?style=flat-square)

</div>

---

> ⚠️ **DISCLAIMER** — Read the [Legal Notice](#-legal-disclaimer) before cloning or running this project.

---

## 📖 Table of Contents

- [About](#-about)
- [Why This Matters](#-why-this-matters)
- [Features](#-features)
- [How It Works](#-how-it-works)
- [MITRE ATT&CK Mapping](#-mitre-attck-mapping)
- [Installation & Build](#-installation--build)
- [Usage](#-usage)
- [Team](#-team)
- [Resources & References](#-resources--references)
- [Legal Disclaimer](#-legal-disclaimer)

---

## 🐱 About

**CryptoCat** is a cybersecurity simulation tool focused on replicating the behavior and lifecycle of modern ransomware. The primary goal is to educate and train cybersecurity professionals by providing a hands-on demonstration of how ransomware is developed, executed, and how it propagates through systems.

Unlike actual malware, CryptoCat is built in a **controlled, ethical, and safe environment** exclusively for **educational and Red Teaming purposes**. It mimics the key stages of a ransomware attack:

- Initial execution of the payload (`.exe`)
- File and directory traversal on the victim's desktop
- File encryption using AES-256 via Windows CryptoAPI
- Desktop wallpaper manipulation to display a simulated ransom warning
- Automated ransom note delivery (`READ_ME.txt`)

---

## 🎯 Why This Matters

Ransomware remains one of the most devastating cyber threats globally. CryptoCat bridges the knowledge gap by enabling:

- **Blue Teams** to practice detection, response, and mitigation in a safe scenario
- **Red Teams** to demonstrate ransomware impact during security assessments
- **Students** to learn how ransomware is structured and detected — step by step
- **Researchers** to study behavioral IOCs tied to real families like REvil, LockBit, and Conti

---

## ⚙️ Features

| Feature | Description |
|---|---|
| 🔐 AES-256 Encryption | Encrypts desktop files using Windows CryptoAPI (`CryptEncrypt`) |
| 🔓 AES-256 Decryption | Fully reverses encryption and restores all original files |
| 🖼️ Wallpaper Manipulation | Sets a ransom warning image as the desktop wallpaper |
| 📝 Ransom Note Drop | Creates `READ_ME.txt` with simulated ransom instructions |
| 📁 File Traversal | Iterates desktop files using `FindFirstFileA` / `FindNextFileA` |
| 🗑️ Original File Deletion | Deletes originals post-encryption, mirrors real ransomware behavior |
| 🏗️ Modular C Code | Clean, documented codebase built for learning and extension |

---

## 🔬 How It Works

**1. File Traversal**
`EncryptFiles()` uses `FindFirstFileA` and `FindNextFileA` to iterate all files on the desktop.

**2. AES-256 Encryption**
`ProcessFile()` acquires a crypto context via `CryptAcquireContext`, derives an AES-256 key from a SHA-256 hash using `CryptDeriveKey`, encrypts each file in chunks via `CryptEncrypt`, appends `.enc`, then deletes the original.

**3. Decryption**
`DecryptFiles()` finds all `*.enc` files, strips the extension, and calls `ProcessFile()` with `encrypt = FALSE` to restore everything.

**4. Wallpaper Takeover**
`downloadAndSetWallpaper()` sets a ransom image as the wallpaper using `SystemParametersInfoA` with `SPI_SETDESKWALLPAPER`.

**5. Ransom Note**
`drop_ransom_note()` writes `READ_ME.txt` to the desktop with a simulated ransom message.

---

## 🗺️ MITRE ATT&CK Mapping

| Tactic | Technique ID | Technique Name | Implementation |
|---|---|---|---|
| Execution | T1204 | User Execution | Payload runs as `.exe` |
| Discovery | T1083 | File and Directory Discovery | `FindFirstFileA` / `FindNextFileA` |
| Impact | T1486 | Data Encrypted for Impact | AES-256 via Windows CryptoAPI |
| Impact | T1491 | Defacement – Internal | Desktop wallpaper replacement |
| Defense Evasion | T1070.004 | File Deletion | Originals deleted post-encryption |
| Command & Control | T1071 | Application Layer Protocol | Simulated C2 wallpaper download |

> Reference: [MITRE ATT&CK Framework](https://attack.mitre.org)

---

## 🛠️ Installation & Build

### Prerequisites

- Windows 10 / 11
- GCC via [MinGW-w64](https://www.mingw-w64.org/) or MSVC (Visual Studio)
- No external libraries — Windows CryptoAPI is used natively

### Clone

```bash
git clone https://github.com/Hassanelsayed14/CryptoCat.git
cd CryptoCat


Build with GCC

gcc cryptocat.c -o cryptocat.exe -ladvapi32 -lshell32 -lurlmon


Build with MSVC

cl cryptocat.c /Fe:cryptocat.exe advapi32.lib shell32.lib urlmon.lib


🚀 Usage
⚠️ Run only inside an isolated VM. Never run on a production machine.

# Encrypt
cryptocat.exe --encrypt

# Decrypt
cryptocat.exe --decrypt


📚 Resources & References
	•	MITRE ATT&CK Framework
	•	VX-Underground
	•	Practical Malware Analysis — Michael Sikorski & Andrew Honig
	•	The Art of Memory Forensics — Ligh, Case, Levy, and Walters
	•	Malware Unicorn Workshops
	•	YouTube: John Hammond · LiveOverflow · HuskyHacks

⚖️ Legal Disclaimer
CryptoCat is developed exclusively for educational, research, and authorized Red Team purposes.
	•	Use only in isolated lab environments you own or have written authorization to test.
	•	Do not deploy against any unauthorized system or individual.
	•	Authors are not responsible for any misuse or damage.
	•	Unauthorized deployment is illegal under computer crime laws worldwide.
Use responsibly. Hack ethically.

<div align="center">Made for learning. Built for defenders.


</div>
```
