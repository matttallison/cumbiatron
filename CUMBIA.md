# Cumbia (a Felucca fork for the M-VAVE FM-1)

Felucca's four-part groovebox, set up for cumbia: villera and editada, the Colombian roots, sonidera, santafesina and
the Peruvian Amazon. Everything Felucca does is still here; this fork adds sounds, two drum kits and the phrases and
grooves to play them with. GPL-3.0-only, as Felucca.

## Sounds

ANALOG, after Felucca's own twelve (PRESETS > ANALOG):

| Preset | What |
| --- | --- |
| PIBE LEAD, PIBE LEAD2 | the cumbia villera lead, on WAVE PIBE: an original wavetable synthesized from harmonic levels (the 3rd and 4th harmonics on top, the fundamental low), an 8-cent detuned pair beating, a decay that shortens up the keyboard; LEAD2 adds a little glide and drive. No samples of any instrument are in it |
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
| LATIN (recorded + synthesized) | KICK | TIMBL timbal | MARAC maracas | GUIRO | CABAS cabasa | CONGA low conga | QUINT high conga | CAMPN |

LATIN plays the TR-727 style timbal, maracas, cabasa and two congas recorded by the Compas author and released with
Compas under GPL-3.0-only; its kick, güiro and campana are CUMB's synthesized ones.
TUNE pitches every lane +-12 semitones, DECAY below 64 fades a hit
shorter, TONE below 64 darkens it, SNAP shapes the attack. Sources and licences: `assets/samples-cumbia/README.md`.

## Phrases and grooves (SAVE > PHRASES)

After Felucca's 13 patterns:

- 14 CUMB BAS: a held quarter on 1 and 3, two short eighths on 2 and 4 (i - V). The held note against the short
  ones, and the release on the downbeat, is the weight. It sets GATE 56 (the eighths short; the quarters are ties).
- 15 CUMB ORG: the organ's chucks on the offbeats (GATE 40).
- 16 CUMB LD: an arpeggio hook with a chromatic step, short notes against held ones (GATE 72).

Drum grooves (any kit; on CUMB / LATIN the lanes are the instruments above):

| Groove | What | SWG |
| --- | --- | --- |
| 17 BOMBON | the villera way: kick heavy on 1 and 3, the shaker (MARAC; CLAP on CUMB) only on 2 and 4, against the bass | 0 |
| 18 VILLERA | the same, busier: güiro, quinto, campana | 0 |
| 19 COLOMBIA | the root: tambora, llamador on the offbeats, alegre, the guache's sixteenths, the long scrape | 22 |
| 20 SANTAFE | santafesina: congas driving | 15 |
| 21 SONIDERA | sonidera | 35 |
| 22 AMAZONIA | Peru: the cowbell anchoring the offbeats, the güiro the subdivisions, simple drums | 12 |
| 23 RASPA | güiro and kick, nothing else | 18 |
| 24 EDITADA | claps on 2 and 4 and a timbal fill | 0 |
| 25 PORRO | Colombia's porro: the tambora's syncopation, the band's cymbal on 2 and 4 | 10 |
| 26 VALLENAT | vallenato: the guacharaca's sixteenths, the caja answering | 15 |
| 27 CHICHA | Peru's chicha: timbal on 2 and 4, the bongó busy, the cowbell on the offbeats | 12 |
| 28 NORTENA | Mexico's norteña: kick on 1 and 3, the snare on every offbeat eighth | 0 |
| 29 TECNO | tecnocumbia: drum-machine kick and clap, cowbell in eighths | 0 |
| 30 REBAJADA | rebajada: sonidera slowed down (68..76 BPM), heavy | 35 |
| 31 DIGITAL | digital cumbia: a pushed kick, claps on 2 and 4 | 0 |
| 32 TROPICAL | Mexico's cumbia tropical: straight güiro, timbal on 2 and 4, the congas' tumbao | 10 |

A groove sets the **song's** swing (GLO SWG), not its track's: one feel for every track, as a groove extracted from a
recording is applied to all of them. Keep it small (100 is 70:30; 12..40 is the 52..58 % range producers use).
The feel itself is in the playing: NUDGE a step early or late (SEQ > DETAIL), and play along with the shaker or the
cowbell until it sits.

Tempos: Colombian cumbia around 100..115 BPM, sonidera 100..125, villera around 110..130, amazónica 90..110.

Performance (FX held, Felucca's): ECHO THROW, REV THROW, REPEAT, TAPE STOP, OCT DN, the filters: the editada tricks.

## SONG > BUILD: a whole song in a style

On GLO > SONG the white keys E4 .. G5 build a song: VILLERA, COLOMBIA, VALLENATO, SONIDERA, REBAJADA, SANTAFE, CHICHA,
AMAZONIA, NORTENA, DIGITAL (an empty song lights them). A dialog asks first: it replaces PROJECT slots A..D. Then:

- the sounds: T1 bass, T2 keys, T3 lead, T4 drums, the style's (VILLERA: BAJO PESAD, CUMBIA ORG, PIBE LEAD, the LATIN
  kit; COLOMBIA: ACORDEON and FLAUTA as the gaita; CHICHA / AMAZONIA: FARFISA or VOX ORG and GUITARRA; ..), its tempo
  and its swing for the whole song
- four sections, saved as projects <STYLE> A..D: A the verse (groove, bass, chucks, the lead resting), B the chorus
  (the cowbell up, the lead's hook), C the break (alegre / quinto, timbal, the lead's solo), D the end (a timbal roll,
  everything on the root)
- the order: intro (drums, then the bass) - verse - chorus - verse - chorus - bridge (percussion and bass) - break -
  a bar of the roll - chorus - outro - the end: 66 bars, and it plays at once

The bass is the held quarter against short eighths, the shaker against the bass, the lead's short notes against held
ones; the harmony i - V (I - V for VALLENATO and NORTENA). The lead's notes are drawn anew each build (an arpeggio on
the chord, a chromatic step into a held note now and then; the solo on the pentatonic scale). Everything is then
ordinary Felucca: edit the sections (load a project, change it, save it back), the order on the SONG page, the sounds.

## What is in it (for sharing it)

Everything here is GPL-3.0-only (the engines MIT): code, presets, grooves, and the only recordings, the five Compas
TR-727 style hits. No Korg, SoundFont or other commercial sounds are in the firmware or this repository: the PIBE
wave is generated by `tools/gen_busy_wave.py` from a short table of harmonic levels (`tools/busy_harm.json`), numbers,
not audio. No preset or menu names Korg or "Busy Boy" (the source comments say what the sound is in the style of); M-VAVE, FM-1, Korg and Roland are trademarks of their
owners and this project is not affiliated with them. User sample slots (USR1..3) are yours: load only sounds you have
the rights to.

## Building

As Felucca (`BUILDING.md`). The build also runs `tools/gen_cumbia_pcm.py` (the LATIN kit's five recordings, about 27 KB of flash).
The engines are no longer a submodule: `firmware/engines/` is Felucca-Engines (MIT) with the Cumbia presets added.

Host demos of the sounds and grooves: `tests/cumbia_demo.c` (see its top) renders WAVs with the firmware's own DSP.

## Credits

Felucca: Leo Kuroshita, Hügelton Instruments, and its contributors (CREDITS.md). The TR-727 style recordings: Joe David Walters, Compas
(https://github.com/TexasRADIO/compas-firmware).
