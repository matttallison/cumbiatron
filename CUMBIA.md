# Cumbia (a Felucca fork for the M-VAVE FM-1)

Felucca's four-part groovebox, set up for cumbia: villera and editada, the Colombian roots, sonidera, santafesina and
the Peruvian Amazon. Everything Felucca does is still here; this fork adds sounds, two drum kits and the phrases and
grooves to play them with. GPL-3.0-only, as Felucca.

## Sounds

ANALOG, after Felucca's own twelve (PRESETS > ANALOG):

| Preset | What |
| --- | --- |
| BUSY BOY, BUSY BOY 2 | the cumbia villera lead (the Korg N364 / X5D "Busy Boy" family): detuned saws / a nasal pulse, a touch of glide, a late vibrato |
| MINI LEAD, SAW LEAD 4, Q DUAL SAW | the other synth leads of the keyboard players |
| FLAUTA | QUADRO's flute character: triangle, breath, glide, a late vibrato (no trill) |
| ZAMPONA | pan flute: sine and breath |
| ACORDEON, BANDONEON | two detuned reeds beating, a slow bellows swell |
| SYNTH BRSS, DETUN BRSS, MAXI STAB | the brasses and the stab |
| CUMBIA BAS | the bass: a sub under it, a little drive, a held body |
| BAJO PESAD | the heavy bass (a Model D's weight): full sub, more drive |
| FARFISA, VOX ORG | the combo organs of Peruvian cumbia: thin, bright, a quick vibrato |
| GUITARRA | the reverb-drenched tremolo guitar of cumbia amazónica |
| SIRENA | the editada siren |

WHEEL: CUMBIA ORG (bright, clicky, fast rotor), ORG SUAVE, VILLERO (driven).

A preset can now set the track's LFO, glide and voice too (`preset_t.xp`); that is how the leads get their vibrato
and the guitar its tremolo. Felucca's own presets set none of it and sound exactly as before.

## Drum kits (DRUM engine, KIT)

| Kit | KICK | SNARE | CLAP | HAT CL | HAT OP | TOM | RIM | BELL |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| CUMB (synthesized) | KICK | TIMBL timbal | CLAP | GUIRO | RASPA long scrape | TUMBA conga | QUINT conga | CAMPN campana |
| HANAN (recorded) | BOMBO | TIMBL | MARAC maracas | GUIRO | CABAS cabasa | CONGA | QUINT | CAMPN |

HANAN plays recordings: Hanan Cumbia's bombo, güiro, conga and campana (Oficina de Sonido, the Bleep Drum based
cumbia machine, CC BY-SA 3.0) and the TR-727 timbal, maracas, cabasa and quinto recorded for Compas (GPL-3.0).
TUNE pitches every lane +-12 semitones (the Hanan conga and cowbell as on the original), DECAY below 64 fades a hit
shorter, TONE below 64 darkens it, SNAP shapes the attack. Sources and licences: `assets/samples-cumbia/README.md`.

## Phrases and grooves (SAVE > PHRASES)

After Felucca's 13 patterns:

- 14 CUMB BAS: a held quarter on 1 and 3, two short eighths on 2 and 4 (i - V). The held note against the short
  ones, and the release on the downbeat, is the weight. It sets GATE 56 (the eighths short; the quarters are ties).
- 15 CUMB ORG: the organ's chucks on the offbeats (GATE 40).
- 16 CUMB LD: an arpeggio hook with a chromatic step, short notes against held ones (GATE 72).

Drum grooves (any kit; on CUMB / HANAN the lanes are the instruments above):

| Groove | What | SWG |
| --- | --- | --- |
| 17 BOMBON | the villera way: kick heavy on 1 and 3, the shaker (MARAC; CLAP on CUMB) only on 2 and 4, against the bass | 0 |
| 18 VILLERA | the same, busier: güiro, quinto, campana | 0 |
| 19 COLOMBIA | the root: tambora, llamador on the offbeats, alegre, the guache's sixteenths, the long scrape | 22 |
| 20 SANTAFE | santafesina: congas driving | 15 |
| 21 SONIDERA | sonidera | 35 |
| 22 AMAZONIA | Peru: the cowbell anchoring the offbeats, the güiro the subdivisions, simple drums | 12 |
| 23 RASPA | güiro and kick, HananCumbia style | 18 |
| 24 EDITADA | claps on 2 and 4 and a timbal fill | 0 |

A groove sets the **song's** swing (GLO SWG), not its track's: one feel for every track, as a groove extracted from a
recording is applied to all of them. Keep it small (100 is 70:30; 12..40 is the 52..58 % range producers use).
The feel itself is in the playing: NUDGE a step early or late (SEQ > DETAIL), and play along with the shaker or the
cowbell until it sits.

Tempos: Colombian cumbia around 100..115 BPM, sonidera 100..125, villera around 110..130, amazónica 90..110.

Performance (FX held, Felucca's): ECHO THROW, REV THROW, REPEAT, TAPE STOP, OCT DN, the filters: the editada tricks.

## Building

As Felucca (`BUILDING.md`). The build also runs `tools/gen_cumbia_pcm.py` (the HANAN samples, about 39 KB of flash).
The engines are no longer a submodule: `firmware/engines/` is Felucca-Engines (MIT) with the Cumbia presets added.

Host demos of the sounds and grooves: `tests/cumbia_demo.c` (see its top) renders WAVs with the firmware's own DSP.

## Credits

Felucca: Leo Kuroshita, Hügelton Instruments, and its contributors (CREDITS.md). Hanan Cumbia: Oficina de Sonido
(https://hanancumbia.oficinadesonido.org/). The TR-727 style recordings: Joe David Walters, Compas
(https://github.com/TexasRADIO/compas-firmware).
