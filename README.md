![License: MIT](https://img.shields.io/badge/License-MIT-emerald.svg)
![Platform: Windows](https://img.shields.io/badge/Platform-Windows-blue.svg)
![Language: C++17](https://img.shields.io/badge/Language-C%2B%2B17-00599C.svg)

# NemOS HWID Grabber

Official open-source hardware identification utility for NemOS Client. Designed to generate a unique machine hardware identifier required for license activation.

---

## Instructions

1. Download the latest executable from the Releases section (`GetHWID.exe`).
2. Run `GetHWID.exe` on your system.
3. The generated HWID hash will be automatically copied to your system clipboard.
4. Navigate to your NemOS Client dashboard, paste the identifier into the HWID registration field, and click Activate.

---

## Privacy and Security

* The tool strictly reads public system identifiers (system partition serial number and CPU/motherboard registry strings).
* Raw hardware parameters are hashed locally using the FNV-1a64 algorithm.
* The application makes zero outbound network calls.
* Source code is fully exposed for auditing and custom compilation.

---

## Compilation

To compile the utility manually using Visual Studio Developer Command Prompt:

```cmd
cl /EHsc /O2 GetHWID.cpp /link user32.lib advapi32.lib
