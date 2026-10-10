# Third-party notices

This repository is MIT-licensed (see `LICENSE`). The following carry third-party provenance that must
be kept in mind when redistributing.

## Crash reporter — GPL-2.0 lineage

`src/Ascension/AscCrashReport.cpp` is transcribed from the original Extensions.dll's crash reporter,
whose implementation derives from TrinityCore's `WheatyExceptionReport` (**GPL-2.0**). The
reconstruction is distributed under this repository's MIT license, but if it is ever treated as a
derivative of the GPL-2.0 work, redistribution terms differ. Resolve before any redistribution that
includes this file (tracked in `docs/IMPROVEMENTS.md`, "crash reporter").

## Vendored libraries (`third_party/`)

| library | license | where |
| --- | --- | --- |
| freetype | FreeType License (FTL) / GPL-2.0 dual | `third_party/freetype/LICENSE.TXT` |
| mapbox | ISC | `third_party/mapbox/LICENSE` |
| nlohmann `json.hpp` | MIT | license block atop `third_party/nlohmann/json.hpp` |

Keep those license files with any redistribution that includes the libraries.
