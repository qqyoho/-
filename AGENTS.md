Always respond in Chinese.

- The global AGENTS.md is the default baseline for all tasks.
- Project or workspace specific instructions supplement or override only conflicting parts.
- When describing active instruction sources, present them as: 1. global baseline 2. workspace or project additions or overrides.

## Windows BMS Source Workflow

- In this Windows BMS firmware project, when reading source, searching source, or modifying GB2312/GBK/GB18030 C/H files, use the `bms-windows-source-workflow` skill.
- Source searches should use `rg` first. If `rg` is unavailable, blocked, or reports `Access is denied`, fall back to PowerShell `Get-ChildItem -Recurse -Include *.c,*.h -File | Select-String`.
- Source searches should exclude generated/build/cache/review directories such as `EWARM`, `Debug`, `Release`, `Use_SI`, `CodexReviewDiff`, `BrowseInfo`, `cache`, and `tmp` unless explicitly needed.
- For paths with spaces, parentheses, Chinese characters, or special characters, use quoted absolute paths and PowerShell `-LiteralPath`.
- Source files should target UTF-8 without BOM. If a legacy GB2312/GBK/GB18030 source file is encountered, convert it to UTF-8 before normal editing and verify strict UTF-8 decoding. Do not proactively generate `gb2312-review-diff` or `CodexReviewDiff` artifacts unless explicitly requested.
