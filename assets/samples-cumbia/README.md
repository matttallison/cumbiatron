# Cumbia samples (DRUM engine, KIT HANAN)

`tools/gen_cumbia_pcm.py` turns these into `build/gen/cumbia_pcm.h` (8-bit mu-law, mono) for the HANAN kit
(`firmware/src/drum_voice.c`, `DV_KIT`). Each file keeps the licence and credit of its source.

| File | Lane | Source | Licence |
| --- | --- | --- | --- |
| `bombo-kick.wav` | KICK | Hanan Cumbia (Oficina de Sonido), build `hanan26Taller`, based on Bleep Labs' Bleep Drum | CC BY-SA 3.0 |
| `huiro-guiro.wav` | GUIRO | Hanan Cumbia, as above | CC BY-SA 3.0 |
| `conga.wav` | CONGA | Hanan Cumbia, as above | CC BY-SA 3.0 |
| `campana-cowbell.wav` | CAMPN | Hanan Cumbia, as above | CC BY-SA 3.0 |
| `727-high-timbale.wav` | TIMBL | TR-727-style recording by Joe David Walters (Compas), `727-sources.json` | GPL-3.0-only |
| `727-maracas.wav` | MARAC | as above | GPL-3.0-only |
| `727-cabasa.wav` | CABAS | as above | GPL-3.0-only |
| `727-open-high-conga.wav` | QUINT | as above | GPL-3.0-only |

The Hanan Cumbia files and their provenance (`HANAN-SOURCE.txt`) come as Compas
(https://github.com/TexasRADIO/compas-firmware, `release-inputs/hanan/`) carries them; the 727 recordings and
`727-sources.json` from the same repository (`release-inputs/studio/`, `board/727-sources.json`).
Hanan Cumbia: https://hanancumbia.oficinadesonido.org/ (CC BY-SA 3.0,
https://creativecommons.org/licenses/by-sa/3.0/).
