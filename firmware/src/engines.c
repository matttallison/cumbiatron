/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments */
/* Engine table (order = PRESETS browsing order and the engine numbers of the editor protocol), the
 * factory patterns (SAVE > PHRASES) and the parts' sounds at power-on. */
#include "../engines/src/dsp.c"   /* Felucca-Engines (MIT): the shared DSP blocks */

/* The parts' engine memory (1.5): a part plays one engine at a time, so the engines with large per-part state
 * (engine_t.mem: PHYS's slots, GRAIN's grains and index, DRUM's lanes, FM6's notes, SLICE's reverse windows)
 * keep it in one region per part instead of an array each. The region holds the state of the engine that
 * last claimed it (eng_mem_owner); a switch to another such engine zeroes its engine_t.mem bytes, which is
 * the state each had at power-on, so a switch sounds as it did with the arrays (tests/union_test.c). State
 * that runs on across notes and switches stays outside (WHEEL's wheels, GRAIN's random numbers).
 * Sized by the largest, ADPCM when it is built (else PHYS, 3 x SYMP's slot); an engine that needs more than ENG_MEM_PART grows it for
 * every part (x NPART of the pool). Zero-initialised pool section: main.c fm1_cstart clears it. */
#if FELUCCA_ADPCM
#define ENG_MEM_PART 13344u    /* ADPCM: its 192 ms echo line, 8 voices and the DSP (13288 B; the host's pointers 13320) */
#else
#define ENG_MEM_PART 12888u
#endif
static union {
    uint64_t align;
    uint8_t b[ENG_MEM_PART];
} eng_mem[NPART] __attribute__((section(".pool")));
static uint8_t eng_mem_owner[NPART];       /* engine index + 1 whose state is in the region, 0 = none (zeros) */
#define ENG_MEM(p) ((void *)eng_mem[p].b)  /* part p's region (p < NPART) */
#define ENG_MEM_FITS(name, size) _Static_assert((size) <= ENG_MEM_PART, name " state over ENG_MEM_PART")

#define CUMBIA_PRESETS 1      /* the Cumbia presets appended to ANALOG and WHEEL (their XP: core.h P_*) */
#include "../engines/src/eng_analog.c"   /* Felucca-Engines (MIT): ANALOG LOFI TRIO WHEEL NOISE */
#include "../engines/src/eng_phase.c"   /* Felucca-Engines (MIT): PHASE */
#include "../engines/src/eng_lofi.c"
#include "eng_sample.c"
#include "eng_formant.c"
#include "../engines/src/eng_trio.c"
#include "../engines/src/eng_wheel.c"
#include "eng_grain.c"
#include "eng_phys.c"           /* PHYS: DaisySP physical models (phys_dsp.c, MIT) */
#include "eng_drum.c"           /* DRUM: the 8-lane kit (drum_voice.c) */
#include "../engines/src/eng_noise.c"
#include "eng_fm6.c"            /* FM6: 6-operator FM, msfa ported (fm6_core.c, Apache-2.0) */
#include "fm4_convert.c"        /* DIGITAL's tables, and its sounds -> FM6 */
#if FELUCCA_FM4
#include "eng_digital.c"        /* DIGITAL: four-operator FM (retired; FELUCCA_FM4=1 builds it) */
#endif
#if FELUCCA_SLICE
#include "eng_slice.c"
#endif
#if FELUCCA_ADPCM
#include "eng_adpcm.c"           /* ADPCM: BRR waves and an FIR echo, its DSP ported from snes_spc (adpcm_dsp.c, LGPL-2.1+) */
#endif

/* the editor protocol, user presets and projects store these indices: append, never reorder */
static const engine_t *const ENGINES[NENGINES] = {
    &ENG_ANALOG,                 /* 0 */
#if FELUCCA_FM4
    &ENG_DIGITAL,                /* 1 (ENGI_DIGITAL) */
#else
    &ENG_FM4_GONE,               /* 1: reserved (DIGITAL, retired: its sounds convert to FM6, fm4_convert.c) */
#endif
    &ENG_PHASE,                  /* 2 */
    &ENG_LOFI,                   /* 3 */
    &ENG_SAMPLE,                 /* 4 */
    &ENG_FORMANT,                /* 5 VOICE (eng_formant.c: "voice" is a sounding note in voice.c) */
    &ENG_TRIO,                   /* 6 */
    &ENG_WHEEL,                  /* 7 */
    &ENG_GRAIN,                  /* 8 */
    &ENG_PHYS,                   /* 9 (ENGI_PHYS) */
    &ENG_DRUM,                   /* 10 (ENGI_DRUM) */
    &ENG_NOISE,                  /* 11 */
    &ENG_FM6,                    /* 12 (ENGI_FM6) */
#if FELUCCA_SLICE
    &ENG_SLICE,                  /* 13 (FELUCCA_SLICE=0 builds without it) */
#endif
#if FELUCCA_ADPCM
    &ENG_ADPCM,                  /* 14 (ENGI_ADPCM; FELUCCA_ADPCM=0 builds without it) */
#endif
};

/* a track's engine number as an index (the audio paths: a compare, cheaper than % NENGINES; a bad number: 0) */
static inline uint32_t eng_idx(uint32_t e) { return e < NENGINES ? e : 0u; }

/* part t's engine memory goes to its engine (t->engine; voice.c engine_block, once a block: on a switch, after
 * the old engine's fade, before any note on the new one): zeroed (its engine_t.mem bytes) when another
 * engine's state is in it. An engine without memory leaves it as it is: a switch back finds its state. */
static void eng_mem_claim(const track_t *t)
{
    uint32_t p = (uint32_t)(t - trk), e, k, n;
    uint32_t *w;
    if (p >= NPART)
        return;
    e = eng_idx(t->engine);
    n = ENGINES[e]->mem;
    if (!n || eng_mem_owner[p] == e + 1u)
        return;
    w = (uint32_t *)ENG_MEM(p);
    for (k = 0; k < (n + 3u) / 4u; k++)
        w[k] = 0;
    eng_mem_owner[p] = (uint8_t)(e + 1u);
}

/* the sample source a track's sound plays that has no data (SAMPLE's SET, GRAIN's SRC, SLICE's SRC: an empty or
 * invalid user slot, a set or PIANO this build lacks): its name (it plays a sine: eng_sample.c smp_sine), 0 = none;
 * *code a small number for it (1.., per engine and source) so the UI says it once (ui_input.c sample_notice) */
static uint8_t snd_said[NTRK];   /* per track: the code last said, 0 = none (a project load clears them: said again) */
static const char *snd_missing(const track_t *t, uint32_t *code)
{
    const engine_t *e = ENGINES[eng_idx(t->eng_req)];
    uint32_t si = (uint32_t)t->p[P_E0] % SMP_NALL;
#if FELUCCA_SLICE
    if (e == &ENG_SLICE) {
        si = slc_src_of(t->p);
        *code = 64u + si;
        return slc_get(si) ? 0 : N_SLC_SRC[si];
    }
#endif
    if (e != &ENG_SAMPLE && e != &ENG_GRAIN)
        return 0;
    *code = (e == &ENG_GRAIN ? 32u : 1u) + si % 32u;
    return smp_set_missing(si) ? SMP_ALL_NAMES[si] : 0;
}

/* the order the engines are shown in (PRESETS browsing and its ENG knob, the EDIT layer's keys, the editor's list):
 * engine indices, never DIGITAL's reserved 1 (with FELUCCA_FM4 it follows FM6). The indices stay as they are (the
 * stores and the protocol hold them); only this table orders them */
static const uint8_t ENGINE_ORDER[NENG_SHOWN] = {
    0,                           /* ANALOG */
    12,                          /* FM6 */
#if FELUCCA_FM4
    1,                           /* DIGITAL */
#endif
    2, 3, 4, 5, 6, 7, 8, 9,      /* PHASE LOFI SAMPLE VOICE TRIO WHEEL GRAIN PHYS */
    11,                          /* NOISE */
#if FELUCCA_SLICE
    13,                          /* SLICE */
#endif
    10,                          /* DRUM */
#if FELUCCA_ADPCM
    14,                          /* ADPCM (1.8): last, so the EDIT layer's white keys keep the 13 above (ui_layer.c LY_ENG_KEYS) */
#endif
};

/* the engines one can pick (engine 1 only with FELUCCA_FM4), in ENGINE_ORDER: eng_ok(e), the n-th of them
 * eng_vis(n), e's place among them eng_rank(e), the next / previous one eng_step(e, dir) (wraps) */
static int eng_ok(uint32_t e) { return e < NENGINES && (FELUCCA_FM4 || e != ENGI_DIGITAL); }
static uint32_t eng_vis(uint32_t n) { return ENGINE_ORDER[n % NENG_SHOWN]; }
static uint32_t eng_rank(uint32_t e)
{
    uint32_t n;
    if (!eng_ok(e))
        e = ENGI_FM6;                            /* (DIGITAL without FELUCCA_FM4: its sounds play as FM6) */
    for (n = 0; n < NENG_SHOWN && ENGINE_ORDER[n] != e; n++)
        ;
    return n < NENG_SHOWN ? n : 0u;
}
static uint32_t eng_step(uint32_t e, int32_t dir)
{
    return eng_vis((eng_rank(e % NENGINES) + (dir > 0 ? 1u : NENG_SHOWN - 1u)) % NENG_SHOWN);
}

/* factory sequence patterns: SAVE > PHRASES loads one into the selected track (ui.c pat_load); a preset
 * only suggests one with PAT(n), loading a sound never touches the steps. Absolute notes, loaded as they
 * are (DRUM plays them as GM drums, SLICE as slices; SCL TRANS and OCT transpose): 0 = rest;
 * flags 1 = accent, 2 = slide, 4 = tie (holds the previous note). Names: at most 8 characters */
#define T_ 4
static const struct {
    const char *name;
    uint8_t note[16], flags[16];
} PATTERNS[] = {
    {"ACID", {45, 45, 57, 45, 0, 48, 45, 55, 45, 0, 57, 52, 45, 48, 0, 50},          /* 1 */
     {1, 0, 2, 0, 0, 0, 1, 2, 0, 0, 1, 0, 0, 2, 0, 1}},
    {"OFFBEAT", {0, 36, 0, 36, 0, 36, 0, 48, 0, 36, 0, 36, 0, 39, 0, 43},          /* 2 bass */
     {0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0}},
    {"MELODY", {60, 0, 67, 0, 72, 67, 0, 64, 62, 0, 69, 0, 74, 69, 0, 67},         /* 3 pluck */
     {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0}},
    {"LEAD", {72, 0, 0, 74, 0, 0, 76, 0, 79, 0, 76, 0, 74, 0, 0, 0},               /* 4 */
     {1, T_, 0, 2, T_, 0, 0, 0, 1, 0, 2, 0, 0, T_, T_, 0}},
    {"PAD", {60, 0, 0, 0, 0, 0, 0, 0, 57, 0, 0, 0, 55, 0, 0, 0},                   /* 5 long notes */
     {0, T_, T_, T_, T_, T_, T_, 0, 0, T_, T_, 0, 0, T_, T_, 0}},
    {"KEYS", {0, 0, 60, 0, 0, 63, 0, 0, 0, 0, 60, 0, 0, 65, 0, 63},                /* 6 offbeat stabs */
     {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0}},
    {"BELL", {72, 0, 0, 79, 0, 0, 84, 0, 0, 0, 76, 0, 0, 0, 0, 0},                 /* 7 sparse */
     {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {"SUB", {36, 0, 0, 0, 0, 0, 0, 36, 0, 0, 34, 0, 0, 0, 0, 0},                   /* 8 low, held */
     {1, T_, T_, T_, 0, 0, 0, 0, 0, 0, 0, T_, T_, T_, 0, 0}},
    /* SLICE (eng_slice.c): note = C4 + slice */
    {"CHOP", {60, 61, 62, 67, 64, 65, 60, 69, 68, 70, 62, 67, 72, 72, 74, 64},     /* 9 16 slices re-ordered */
     {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0}},
    {"STUTTER", {60, 60, 61, 61, 62, 0, 63, 63, 64, 65, 65, 0, 66, 66, 66, 67},    /* 10 8 slices, repeats */
     {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0}},
    {"SLICES", {60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75},   /* 11 in order */
     {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0}},
    /* DRUM (General MIDI: 36 kick, 38 snare, 42 closed / 46 open hi-hat) */
    {"BEAT", {36, 42, 42, 42, 38, 42, 36, 42, 36, 42, 42, 36, 38, 42, 46, 42},     /* 12 */
     {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0}},
    /* 16ths up a C minor arpeggio, twice: the line the ARP presets (RAVE, ARP 8BIT, ARP LEAD) suggest
     * now that the arpeggiator is the track's and a preset no longer switches it on */
    {"ARP", {48, 51, 55, 60, 63, 67, 72, 75, 48, 51, 55, 60, 63, 67, 72, 75},       /* 13 */
     {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0}},
    /* Cumbia (i - V in A minor): the bass a held quarter on 1 and 3, two short eighths on 2 and 4, the release the
     * downbeats resolve; the organ's chucks on the offbeats; a lead of short notes and held ones (an arpeggio, a
     * chromatic step). Their GATE: PHRASE_GATE below (the held notes are ties, the rest short) */
    {"CUMB BAS", {33, 0, 0, 0, 40, 0, 40, 0, 28, 0, 0, 0, 40, 0, 40, 0},             /* 14 */
     {1, T_, T_, T_, 0, 0, 0, 0, 1, T_, T_, T_, 0, 0, 0, 0}},
    {"CUMB ORG", {0, 0, 57, 0, 0, 0, 57, 0, 0, 0, 52, 0, 0, 0, 52, 0},             /* 15 */
     {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0}},
    {"CUMB LD", {69, 72, 76, 0, 75, 76, 0, 72, 69, 0, 0, 68, 69, 0, 64, 0},         /* 16 */
     {1, 0, 0, T_, 0, 0, 0, 0, 1, T_, T_, 0, 0, 0, 0, 0}},
};
#undef T_
#define NPATTERNS (sizeof PATTERNS / sizeof PATTERNS[0])

/* Cumbia: factory drum grooves (SAVE > PHRASES, after PATTERNS[]): the lanes struck on steps 1..16 (bit l: lane l, as
 * the DRUM grid: KICK TIMBL CLAP GUIRO RASPA TUMBA QUINT CAMPN on KIT CUMB) and their accents. Loaded as a user
 * preset's grid (ui.c load_grid16); on another kit the same lanes play its own pieces. Names: at most 8 characters */
#define G_K 1u
#define G_T 2u
#define G_C 4u
#define G_G 8u
#define G_R 16u
#define G_TU 32u
#define G_Q 64u
#define G_B 128u
static const struct {
    const char *name;
    uint8_t hit[16], acc[16];
    uint8_t swing;               /* the GLOBAL SWG it sets: one feel for every track (a little: 100 is 70:30) */
} GROOVES[] = {
    /* the villera way (and Bombón asesino's): the kick heavy on 1 and 3, the shaker (MARAC; CUMB's CLAP) only on 2 and 4,
     * against the bass; the tumba answering. Little else: the weight is the kick and the bass */
    {"BOMBON", {G_K, 0, 0, 0, G_C, 0, 0, G_TU, G_K, 0, 0, 0, G_C, 0, 0, G_TU},
     {G_K, 0, 0, 0, G_C, 0, 0, 0, G_K, 0, 0, 0, G_C, 0, 0, 0}, 0},
    {"VILLERA", {G_K | G_R, 0, G_B | G_G, G_Q | G_G, G_C | G_R, 0, G_TU | G_G, G_G,
                 G_K | G_R, 0, G_B | G_G, G_Q | G_G, G_C | G_R, 0, G_TU | G_G, G_T},
     {G_K, 0, 0, 0, G_C, 0, G_TU, 0, G_K, 0, 0, 0, G_C, 0, G_TU, 0}, 0},
    /* the Colombian root: tambora (KICK, the TIMBL its rim), llamador on the offbeats (TUMBA), alegre (QUINT), the
     * guache's sixteenths (GUIRO) leaning on the beat, the long scrape (RASPA) */
    {"COLOMBIA", {G_K | G_G | G_R, G_G, G_TU | G_G, G_Q | G_G, G_G | G_R, G_G, G_TU | G_G, G_G,
                  G_K | G_G | G_R, G_G, G_TU | G_G, G_Q | G_G, G_T | G_G | G_R, G_Q | G_G, G_TU | G_G, G_G},
     {G_K | G_G, 0, G_TU, 0, G_G, 0, G_TU, 0, G_K | G_G, 0, G_TU, G_Q, G_G, 0, G_TU, 0}, 22},
    {"SANTAFE", {G_K | G_R | G_B, 0, G_Q | G_G, G_G, G_R | G_B, 0, G_TU | G_G, G_TU | G_G,
                 G_K | G_R | G_B, 0, G_Q | G_G, G_G, G_R | G_B, 0, G_TU | G_G, G_T | G_TU | G_G},
     {G_K, 0, G_Q, 0, 0, 0, G_TU, 0, G_K, 0, G_Q, 0, 0, 0, G_TU, G_T}, 15},
    {"SONIDERA", {G_K | G_R | G_B, 0, G_G, G_G, G_T | G_R, G_Q, G_G, G_TU | G_G,
                  G_K | G_R | G_B, 0, G_G, G_G, G_T | G_R, G_Q, G_G, G_TU | G_G},
     {G_K, 0, 0, 0, G_T, 0, 0, G_TU, G_K, 0, 0, 0, G_T, 0, 0, G_TU}, 35},
    /* Peru, the Amazon: the cowbell anchors the offbeats, the güiro the subdivisions, the drums simple */
    {"AMAZONIA", {G_K | G_G, 0, G_B | G_G, G_Q, G_R, 0, G_B | G_G | G_TU, G_G,
                  G_K | G_G, 0, G_B | G_G, G_Q, G_R, 0, G_B | G_G | G_TU, G_T},
     {G_K, 0, G_B, 0, 0, 0, G_B, 0, G_K, 0, G_B, 0, 0, 0, G_B, G_T}, 12},
    {"RASPA", {G_K | G_R, 0, G_G, G_G, G_R, 0, G_G, G_G, G_K | G_R, 0, G_G, G_G, G_R, 0, G_G, G_G},
     {G_K | G_R, 0, 0, 0, G_R, 0, 0, 0, G_K | G_R, 0, 0, 0, G_R, 0, 0, 0}, 18},
    {"EDITADA", {G_K | G_R, 0, G_G, G_G, G_C | G_R, 0, G_TU | G_G, G_G,
                 G_K | G_R, 0, G_G, G_Q | G_G, G_C | G_R, G_T, G_T | G_TU, G_T},
     {G_K, 0, 0, 0, G_C, 0, G_TU, 0, G_K, 0, 0, 0, G_C, 0, G_T, G_T}, 0},
    /* Colombia's porro: the tambora's syncopation (1, the "and" of 2), the band's cymbal (TIMBL) on 2 and 4 */
    {"PORRO", {G_K | G_G, 0, G_G, G_Q, G_T | G_G, 0, G_K | G_G, 0, G_K | G_G, 0, G_G, G_Q, G_T | G_G, 0, G_K | G_G, 0},
     {G_K, 0, 0, 0, G_T, 0, G_K, 0, G_K, 0, 0, 0, G_T, 0, G_K, 0}, 10},
    /* vallenato: the guacharaca's chuku-chuku sixteenths, the caja (TUMBA, QUINT) answering */
    {"VALLENAT", {G_K | G_G | G_TU, G_G, G_G, G_G | G_Q, G_G, G_G, G_G | G_Q, G_G | G_Q,
                  G_K | G_G | G_TU, G_G, G_G, G_G | G_Q, G_G, G_G, G_G | G_Q, G_G | G_Q},
     {G_K | G_G, 0, 0, G_G, G_G, 0, 0, G_G, G_K | G_G, 0, 0, G_G, G_G, 0, 0, G_G}, 15},
    /* Peru's chicha: timbal on 2 and 4, the bongó (QUINT) busy, the güiro, the cowbell on the offbeats */
    {"CHICHA", {G_K | G_G, G_Q, G_B | G_G, G_Q, G_T | G_G, G_Q, G_B | G_G, G_Q,
                G_K | G_G, G_Q, G_B | G_G, G_Q, G_T | G_G, G_Q, G_B | G_G, G_Q | G_TU},
     {G_K, 0, G_B, 0, G_T, 0, G_B, 0, G_K, 0, G_B, 0, G_T, 0, G_B, G_TU}, 12},
    /* Mexico's norteña: the bass oom on 1 and 3, the snare (CLAP) on every offbeat eighth: the pah */
    {"NORTENA", {G_K, 0, G_C, 0, G_G, 0, G_C, 0, G_K, 0, G_C, 0, G_G, 0, G_C, 0},
     {G_K, 0, G_C, 0, 0, 0, G_C, 0, G_K, 0, G_C, 0, 0, 0, G_C, 0}, 0},
    /* tecnocumbia: the drum machine, kick and clap, the güiro on the offbeats, the cowbell in eighths */
    {"TECNO", {G_K | G_B, 0, G_G | G_B, 0, G_C | G_B, 0, G_G | G_B, G_TU, G_K | G_B, 0, G_G | G_B, 0, G_C | G_B, 0, G_G | G_B, G_TU},
     {G_K, 0, 0, 0, G_C, 0, 0, 0, G_K, 0, 0, 0, G_C, 0, 0, 0}, 0},
    /* rebajada (Monterrey): sonidera slowed down (play it at 68..76 BPM), heavy, laid back */
    {"REBAJADA", {G_K | G_R, 0, G_G, G_G, G_T | G_R, G_Q, G_G, G_TU, G_K | G_R, 0, G_G, G_G, G_T | G_R, G_Q, G_G, G_TU},
     {G_K, 0, 0, 0, G_T, 0, 0, G_TU, G_K, 0, 0, 0, G_T, 0, 0, G_TU}, 35},
    /* digital cumbia: the kick pushed (on 1, the "a" of 2, 3), claps on 2 and 4, the güiro in eighths */
    {"DIGITAL", {G_K | G_R | G_G, 0, G_G, 0, G_C | G_G, 0, G_TU | G_G, G_K, G_K | G_R | G_G, 0, G_G, 0, G_C | G_G, 0, G_TU | G_G, 0},
     {G_K, 0, 0, 0, G_C, 0, G_TU, 0, G_K, 0, 0, 0, G_C, 0, G_TU, 0}, 0},
    /* Mexico's cumbia tropical (grupera): straight güiro, timbal on 2 and 4, the congas' tumbao */
    {"TROPICAL", {G_K | G_G, 0, G_G, G_Q, G_T | G_G, 0, G_TU | G_G, G_TU, G_K | G_G, 0, G_G, G_Q, G_T | G_G, 0, G_TU | G_G, G_TU},
     {G_K, 0, 0, 0, G_T, 0, G_TU, 0, G_K, 0, 0, 0, G_T, 0, G_TU, 0}, 10},
};
#undef G_K
#undef G_T
#undef G_C
#undef G_G
#undef G_R
#undef G_TU
#undef G_Q
#undef G_B
#define NGROOVES (sizeof GROOVES / sizeof GROOVES[0])
#define NFACT_PAT (NPATTERNS + NGROOVES)   /* the factory phrases: PATTERNS[], then GROOVES[] */
typedef char cumbia_pat0_fits[NPATTERNS == 16u ? 1 : -1];   /* the kits suggest PAT(17): BOMBON */
/* (Cumbia) the GATE a factory phrase sets (0: the track's own): the Cumbia phrases' short notes */
static uint32_t phrase_gate(uint32_t n)   /* GATE 128 = a whole step: the bass's eighths 56, the chucks 40, the lead 72 */
{
    return n == 13u ? 56u : n == 14u ? 40u : n == 15u ? 72u : 0u;
}

/* the parts at power-on (engine, preset, PATTERNS[n - 1] in the sequencer, 0 = empty: all are): bass, pad, lead, drums */
static const uint8_t TRK_DEF[NPART][3] = {{0, 4, 0}, {ENGI_FM6, 4, 0}, {3, 0, 0}, {ENGI_DRUM, 0, 0}}; /* ANALOG ACID,
                                                                       * FM6 PAD (was DIGITAL PAD), LOFI PULSE LD, DRUM KIT */
static uint32_t trk_def_engine(uint32_t i) { return TRK_DEF[i % NPART][0]; }
