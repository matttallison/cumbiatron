/* SPDX-License-Identifier: GPL-3.0-only */
/* (Cumbia) SONG > BUILD: a whole song in a cumbia style, from the white keys E4 .. G5 on GLO > SONG (ui_input.c
 * song_keys), after a dialog: it replaces PROJECT slots A..D. Included after project.c (felucca.c, tests/ui_test.c).
 *
 * What a style sets: the four tracks' sounds (T1 bass, T2 keys, T3 lead, T4 drums: as the power-on parts), the tempo
 * and the song's swing (GLO SWG: one feel for every track), then four sections' parts, saved as projects A..D, and the
 * song's order over them:
 *   A verse    the style's groove, the bass, the keys' chucks (the lead rests: the voice's space)
 *   B chorus   the groove with the cowbell up, the lead's hook
 *   C break    the groove busier (alegre / quinto, timbal), the lead's solo
 *   D end      a timbal roll; the bass, the keys and the lead hold the root
 *   INTRO drums x4, + bass x4, VERSE x8, CHORUS x8, VERSE x8, CHORUS x8, BRIDGE (drums C, bass) x4, BREAK x8,
 *   a bar of fill, CHORUS x8, OUTRO (drums, bass) x4, the end x1
 * The rhythm is the point (what players of the music said): the bass a held note against short ones, the shaker
 * against the bass, the lead's short notes against held ones; the harmony stays i - V (I - V in the major styles).
 * The lead's notes come from rng(): each build is a new tune on the same frame. Then the song plays */
#define CS_K 1u
#define CS_T 2u
#define CS_C 4u
#define CS_G 8u
#define CS_R 16u
#define CS_TU 32u
#define CS_Q 64u
#define CS_B 128u

enum { CB_QS, CB_TRAD, CB_TUMB, CB_NORT, CB_VALL };    /* the bass's half-bar rhythm */
enum { CK_CHUCK, CK_PUMP, CK_OFF2 };                    /* the keys' */
typedef struct {
    const char *name;
    uint8_t bpm, swing, kit, groove;                    /* kit: DRUM's preset (4 CUMBIA KIT, 5 HANAN KIT); GROOVES[] */
    uint8_t bass, brhy;                                 /* ANALOG preset, CB_* */
    uint8_t keys_e, keys_p, krhy;                       /* engine, preset, CK_* */
    uint8_t lead_e, lead_p;
    uint8_t root, major;                                /* the key: its pitch class, major or minor */
} cs_style_t;
#define CS_ANALOG 0u
#define CS_WHEEL 7u
static const cs_style_t CS_STYLE[] = {
    {"VILLERA", 112, 0, 5, 0, 26, CB_QS, CS_WHEEL, 5, CK_CHUCK, CS_ANALOG, 12, 9, 0},     /* BOMBON, BAJO PESAD, CUMBIA ORG, BUSY BOY */
    {"COLOMBIA", 100, 22, 5, 2, 24, CB_TRAD, CS_ANALOG, 19, CK_CHUCK, CS_ANALOG, 17, 9, 0},   /* ACORDEON, FLAUTA (the gaita) */
    {"VALLENATO", 108, 15, 5, 9, 24, CB_VALL, CS_ANALOG, 20, CK_PUMP, CS_ANALOG, 19, 7, 1},  /* BANDONEON, ACORDEON leading */
    {"SONIDERA", 108, 35, 4, 4, 24, CB_TUMB, CS_ANALOG, 19, CK_CHUCK, CS_ANALOG, 14, 9, 0},  /* ACORDEON, MINI LEAD */
    {"REBAJADA", 72, 35, 4, 13, 26, CB_TUMB, CS_ANALOG, 19, CK_CHUCK, CS_ANALOG, 17, 4, 0},  /* slow and low */
    {"SANTAFE", 108, 15, 5, 3, 24, CB_TUMB, CS_ANALOG, 21, CK_CHUCK, CS_ANALOG, 15, 2, 0},   /* SYNTH BRSS, SAW LEAD 4 */
    {"CHICHA", 100, 12, 4, 10, 24, CB_TUMB, CS_ANALOG, 27, CK_OFF2, CS_ANALOG, 29, 4, 0},    /* FARFISA, GUITARRA */
    {"AMAZONIA", 98, 12, 4, 5, 24, CB_TUMB, CS_ANALOG, 28, CK_OFF2, CS_ANALOG, 29, 9, 0},    /* VOX ORG, GUITARRA */
    {"NORTENA", 116, 0, 5, 11, 24, CB_NORT, CS_ANALOG, 20, CK_CHUCK, CS_ANALOG, 19, 7, 1},   /* BANDONEON, ACORDEON */
    {"DIGITAL", 118, 0, 4, 14, 26, CB_QS, CS_ANALOG, 23, CK_CHUCK, CS_ANALOG, 13, 9, 0},     /* MAXI STAB, BUSY BOY 2 */
};
#define CS_N (sizeof CS_STYLE / sizeof CS_STYLE[0])
/* the keys E4 .. G5 (key numbers from F3, seq.c song_key's) */
static const uint8_t CS_KEY[CS_N] = {11, 12, 14, 16, 18, 19, 21, 23, 24, 26};
static const char *cumbia_style_name(uint32_t i) { return CS_STYLE[i % CS_N].name; }
static int32_t cs_style_of_key(uint32_t k)
{
    uint32_t i;
    for (i = 0; i < CS_N; i++)
        if (CS_KEY[i] == k)
            return (int32_t)i;
    return -1;
}

/* half-bar rhythms, 8 steps: R the chord's root, F its fifth above, L its fifth below, O the octave, T a tie, . rest;
 * the keys: X the chord, T a tie */
static const char *const CS_BASS[] = {"RTTTF.F.", "RTTT..F.", "R..F..R.", "RT..FT..", "R..RF.F."};
static const char *const CS_KEYS[] = {"..X...X.", ".X.X.X.X", "..X.X.X."};
/* the lead's bars: S a short note, H a held one (T its ties), . rest; the hook's and the solo's */
static const char *const CS_HOOK[] = {"SSHTSS.SHTTS.SHT", "S.SSHTT.S.SSHTTT", "HTSSHTS.STSSHTT.", "SSS.HTTTSS.SHTTT"};
static const char *const CS_SOLO[] = {"SSSS.SSSHTSSS.ST", "SS.SSSS.SSSSHTTT", "S.SSS.SSSSS.SHTT"};

static step_t *cs_st;                                   /* the steps being written */
static void cs_clear(step_t *s)
{
    uint32_t i;
    for (i = 0; i < NSTEP; i++) {
        memset(&s[i], 0, sizeof s[i]);
        s[i].time = ST_REST;
    }
}
static void cs_note(uint32_t i, const uint8_t *n, uint32_t k, uint32_t flags)
{
    step_t *s = &cs_st[i];
    uint32_t j;
    s->n = (uint8_t)k;
    for (j = 0; j < 4u; j++)
        s->note[j] = j < k ? n[j] : 0;
    s->time = ST_NOTE;
    s->flags = (uint8_t)flags;
    s->vel = (uint8_t)(flags & SF_ACCENT ? 112u : 84u + rng() % 16u);
}
static void cs_tie(uint32_t i) { cs_st[i].time = ST_TIE; }

/* the chord of half bar h (0, 1): i (I) then V; its root as a bass note (octave 1..2) */
static uint32_t cs_root(const cs_style_t *y, uint32_t h) { return 28u + (y->root + (h ? 7u : 0u) + 12u - 4u) % 12u; }

static void cs_bass(const cs_style_t *y, uint32_t held)
{
    uint32_t h, i;
    for (h = 0; h < 2u; h++) {
        uint32_t r = cs_root(y, h);
        const char *p = CS_BASS[y->brhy % (sizeof CS_BASS / sizeof CS_BASS[0])];
        for (i = 0; i < 8u; i++) {
            uint32_t at = h * 8u + i;
            uint8_t n;
            char c = held ? (i ? 'T' : 'R') : p[i];
            if (c == 'T') { cs_tie(at); continue; }
            if (c == '.') continue;
            n = (uint8_t)(c == 'R' ? r : c == 'F' ? r + 7u : c == 'L' ? r - 5u : r + 12u);
            cs_note(at, &n, 1, i == 0u ? SF_ACCENT : 0u);
        }
    }
}

/* the chord as three notes in octave 3 (52..63): the root, the third (minor on i in a minor key; V always major), the
 * fifth */
static void cs_chord(const cs_style_t *y, uint32_t h, uint8_t *n)
{
    uint32_t r = 52u + (y->root + (h ? 7u : 0u) + 12u - 4u) % 12u;
    n[0] = (uint8_t)r;
    n[1] = (uint8_t)(r + (h || y->major ? 4u : 3u));
    n[2] = (uint8_t)(r + 7u);
}
static void cs_keys(const cs_style_t *y, uint32_t held)
{
    uint32_t h, i;
    for (h = 0; h < 2u; h++) {
        uint8_t n[3];
        const char *p = CS_KEYS[y->krhy % (sizeof CS_KEYS / sizeof CS_KEYS[0])];
        cs_chord(y, held ? 0u : h, n);
        for (i = 0; i < 8u; i++) {
            uint32_t at = h * 8u + i;
            if (held) {
                if (at) cs_tie(at); else cs_note(0, n, 3, SF_ACCENT);
            } else if (p[i] == 'X')
                cs_note(at, n, 3, i == 2u ? SF_ACCENT : 0u);
        }
    }
}

/* the lead: chord tones of the half bar's chord (an arpeggio walking up and down from the last note), a chromatic
 * step up into a held note now and then; the solo walks the pentatonic scale instead */
static void cs_lead(const cs_style_t *y, uint32_t solo, uint32_t held)
{
    static const uint8_t PENT_MIN[5] = {0, 3, 5, 7, 10}, PENT_MAJ[5] = {0, 2, 4, 7, 9};
    const char *p = solo ? CS_SOLO[rng() % (sizeof CS_SOLO / sizeof CS_SOLO[0])]
                         : CS_HOOK[rng() % (sizeof CS_HOOK / sizeof CS_HOOK[0])];
    uint32_t base = 64u + (y->root + 12u - 4u) % 12u;      /* the key's root in octave 4..5 (64..75) */
    int32_t idx = 2, dir = 1;
    uint32_t i;
    if (held) {
        uint8_t n = (uint8_t)base;
        cs_note(0, &n, 1, SF_ACCENT);
        for (i = 1; i < 16u; i++) cs_tie(i);
        return;
    }
    for (i = 0; i < 16u; i++) {
        uint32_t h = i / 8u, tone[6], nt, k;
        uint8_t n;
        if (p[i] == 'T') { cs_tie(i); continue; }
        if (p[i] == '.') continue;
        if (solo) {                                     /* two octaves of the pentatonic scale */
            const uint8_t *sc = y->major ? PENT_MAJ : PENT_MIN;
            for (k = 0; k < 5u; k++) { tone[k] = base + sc[k]; }
            nt = 5;
            if (idx > 9) idx = 9;
            n = (uint8_t)(tone[(uint32_t)idx % 5u] + ((uint32_t)idx / 5u) * 12u - 12u);
        } else {                                        /* the chord's tones over an octave and a half */
            uint8_t c[3];
            cs_chord(y, h, c);
            for (k = 0; k < 3u; k++) { tone[k] = c[k] + 12u; tone[k + 3u] = c[k] + 24u; }
            nt = 6;
            if (idx > 5) idx = 5;
            n = (uint8_t)tone[(uint32_t)idx % nt];
            if (p[i] == 'H' && i && cs_st[i - 1u].time == ST_REST && rng() % 4u == 0u) {   /* a chromatic step into it */
                uint8_t a = (uint8_t)(n - 1u);
                cs_note(i - 1u, &a, 1, 0);
            }
        }
        cs_note(i, &n, 1, p[i] == 'H' || i % 4u == 0u ? SF_ACCENT : 0u);
        if (rng() % 5u == 0u)
            dir = -dir;
        idx += dir * (int32_t)(1u + (solo && rng() % 3u == 0u));
        if (idx < 0) { idx = 1; dir = 1; }
        if (idx > (int32_t)(solo ? 9u : 5u)) { idx = (int32_t)(solo ? 8u : 4u); dir = -1; }
    }
}

/* the drums: the groove (B: the cowbell on the offbeats; C: alegre / quinto and the timbal busier) or the end's roll */
static void cs_drums(const cs_style_t *y, uint32_t part)
{
    uint32_t i, g = y->groove % NGROOVES;
    for (i = 0; i < 16u; i++) {
        step_t *s = &cs_st[i];
        uint32_t hit = GROOVES[g].hit[i], acc = GROOVES[g].acc[i];
        if (part == 1u && (i % 4u) == 2u)
            hit |= CS_B;
        if (part == 2u) {
            if (i == 3u || i == 7u || i == 11u || i == 13u || i == 15u) hit |= CS_Q;
            if (i == 4u || i == 12u) hit |= CS_T, acc |= CS_T;
        }
        if (part == 3u) {                               /* the roll: the kick, then the timbal, faster to the end */
            hit = i == 0u ? CS_K | CS_R : i >= 8u ? CS_T : 0u;
            acc = i == 0u || i == 15u ? hit : 0u;
        }
        if (!hit)
            continue;
        s->time = ST_NOTE;
        s->hit = (uint8_t)hit;
        s->acc = (uint8_t)(acc & hit);
        s->vel = (uint8_t)(84u + rng() % 16u);
        if (part == 3u && i >= 12u)
            s->flags = (uint8_t)(((i - 11u) < 4u ? i - 11u : 3u) << SF_RATCH_SH);   /* x2 .. x4 */
    }
}

static void cs_timing(track_t *t, uint32_t gate)
{
    t->p[P_SLEN] = 16;
    t->p[P_SDIV] = TP[P_SDIV].def;
    t->p[P_SSWING] = 0;
    t->p[P_SGATE] = (int16_t)gate;
}

/* section part (0..3: A..D) into the four tracks */
static void cs_section(const cs_style_t *y, uint32_t part)
{
    cs_st = trk[0].step; cs_clear(cs_st); cs_bass(y, part == 3u); cs_timing(&trk[0], 56);
    cs_st = trk[1].step; cs_clear(cs_st); cs_keys(y, part == 3u); cs_timing(&trk[1], 40);
    cs_st = trk[2].step; cs_clear(cs_st);
    if (part) cs_lead(y, part == 2u, part == 3u);
    cs_timing(&trk[2], 72);
    cs_st = trk[3].step; cs_clear(cs_st); cs_drums(y, part); cs_timing(&trk[3], TP[P_SGATE].def);
}

/* the order: {bass, keys, lead, drums} slots (CHAIN_SILENT "-") and repeats */
#define S_ CHAIN_SILENT
static const uint8_t CS_ORDER[][5] = {
    {S_, S_, S_, 0, 4}, {0, S_, S_, 0, 4},              /* intro: the drums, then the bass */
    {0, 0, S_, 0, 8}, {1, 1, 1, 1, 8},                  /* verse, chorus */
    {0, 0, S_, 0, 8}, {1, 1, 1, 1, 8},
    {0, S_, S_, 2, 4}, {2, 2, 2, 2, 8},                 /* the bridge (percussion and bass), the break */
    {0, S_, S_, 3, 1}, {1, 1, 1, 1, 8},                 /* a bar of the roll, the last chorus */
    {0, S_, S_, 0, 4}, {3, 3, 3, 3, 1},                 /* outro, the end */
};
#undef S_

static void cumbia_build(uint32_t style)
{
    const cs_style_t *y = &CS_STYLE[style % CS_N];
    uint32_t i, k;
    char name[PROJ_NAME_LEN + 1];
    if (transport_busy()) {
        ui_message("STOP FIRST");
        return;
    }
    set_engine_of(&trk[0], CS_ANALOG);
    apply_preset_to(&trk[0], y->bass);
    set_engine_of(&trk[1], y->keys_e);
    apply_preset_to(&trk[1], y->keys_p);
    set_engine_of(&trk[2], y->lead_e);
    apply_preset_to(&trk[2], y->lead_p);
    set_engine_of(&trk[3], ENGI_DRUM);
    apply_preset_to(&trk[3], y->kit);
    for (i = 0; i < NTRK; i++)
        motion_clear(&trk[i]);
    song.g[G_BPM] = y->bpm;
    song.g[G_SWING] = y->swing;
    chain_defaults(&chain_config);
    for (i = 0; i < sizeof CS_ORDER / sizeof CS_ORDER[0]; i++) {
        for (k = 0; k < NTRK; k++)
            chain_config.row[i].slot[k] = CS_ORDER[i][k];
        chain_config.row[i].repeat = CS_ORDER[i][4];
    }
    chain_config.count = (uint8_t)(sizeof CS_ORDER / sizeof CS_ORDER[0]);
    ui.song_row = 0;
    for (i = 4u; i-- > 0u;) {                          /* D, C, B, then A: the tracks are left with the verse */
        cs_section(y, i);
        str_cpy(name, y->name, sizeof name);
        k = str_len(name);
        if (k > PROJ_NAME_LEN - 2u)
            k = PROJ_NAME_LEN - 2u;
        name[k] = ' ';
        name[k + 1u] = (char)('A' + i);
        name[k + 2u] = 0;
        if (project_save_as(i, name))
            return;                                     /* (its message says why) */
    }
    for (i = 0; i < NTRK; i++)
        pat_sig[i] = steps_sig(&trk[i]);
    chain_play_ui();
    ui_say(y->name, " SONG");
}
#undef CS_K
#undef CS_T
#undef CS_C
#undef CS_G
#undef CS_R
#undef CS_TU
#undef CS_Q
#undef CS_B
#define CS_B_LANE 128u                                  /* (the cowbell lane, for tests) */
