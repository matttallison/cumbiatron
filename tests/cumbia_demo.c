/* SPDX-License-Identifier: GPL-3.0-only */
/* Cumbia demos: the firmware's own DSP and sequencer (hostsim.c), four parts playing the factory phrases and
 * grooves, to WAV. build/host/cumbia_demo OUT_DIR
 *   villera.wav   112 BPM: LATIN KIT BOMBON, BAJO PESAD, CUMBIA ORG (chords), PIBE LEAD
 *   colombia.wav  100 BPM: LATIN KIT COLOMBIA, CUMBIA BAS, ACORDEON, FLAUTA
 *   amazonia.wav  100 BPM: CUMBIA KIT AMAZONIA, CUMBIA BAS, FARFISA, GUITARRA
 *   presets.wav   every Cumbia preset in turn on the lead phrase (ANALOG, WHEEL), then the two kits' lanes */
#define main hostsim_main
#include "hostsim.c"
#undef main

static FILE *wo;
static uint32_t wframes;
static void out_blk(void)
{
    int32_t o[2 * CTL];
    uint32_t i;
    mix_block(o, CTL);
    for (i = 0; i < 2u * CTL; i++) {
        int16_t s = (int16_t)(o[i] > 32767 ? 32767 : o[i] < -32768 ? -32768 : o[i]);
        fwrite(&s, 2, 1, wo);
    }
    wframes += CTL;
}
static void run_for(double s)
{
    uint32_t end = wframes + (uint32_t)(s * FS) / CTL * CTL;
    while (wframes < end)
        out_blk();
}
static void wav_open(const char *dir, const char *name)
{
    char p[512];
    snprintf(p, sizeof p, "%s/%s", dir, name);
    wo = fopen(p, "wb");
    wframes = 0;
    wav_hdr(wo, 0);
}
static void wav_close(void)
{
    fseek(wo, 0, SEEK_SET);
    wav_hdr(wo, wframes);
    fclose(wo);
}

static void pat(track_t *t, uint32_t k, uint32_t chord)    /* PATTERNS[k], each note a minor triad if chord */
{
    uint32_t i;
    for (i = 0; i < 16u; i++) {
        uint8_t n[3] = {PATTERNS[k].note[i], (uint8_t)(PATTERNS[k].note[i] + 3), (uint8_t)(PATTERNS[k].note[i] + 7)};
        uint32_t fl = PATTERNS[k].flags[i];
        put_step(t, i, n[0] ? (chord ? 3u : 1u) : 0u, n, (fl & 4u) ? ST_TIE : n[0] ? ST_NOTE : ST_REST,
                 n[0] ? fl & (SF_ACCENT | SF_SLIDE) : 0u);
    }
    t->p[P_SLEN] = 16;
    if (phrase_gate(k))
        t->p[P_SGATE] = (int16_t)phrase_gate(k);
}
static void groove(track_t *t, uint32_t g)
{
    uint32_t i;
    for (i = 0; i < 16u; i++) {
        step_t *s = &t->step[i];
        put_step(t, i, 0, 0, GROOVES[g].hit[i] ? ST_NOTE : ST_REST, 0);
        s->hit = GROOVES[g].hit[i];
        s->acc = GROOVES[g].acc[i];
    }
    t->p[P_SLEN] = 16;
    t->p[P_SSWING] = 0;
    song.g[G_SWING] = GROOVES[g].swing;          /* one feel for every track (ui.c pat_load) */
}

static void song4(const char *dir, const char *name, uint32_t bpm, uint32_t kit, uint32_t gr, uint32_t bass_p,
                  uint32_t keys_e, uint32_t keys_p, uint32_t lead_p)
{
    host_tracks_init();
    song.master_q12 = 4096;
    song.g[G_BPM] = (int16_t)bpm;
    host_preset(&trk[0], 0, bass_p);
    pat(&trk[0], 13, 0);
    host_preset(&trk[1], keys_e, keys_p);
    pat(&trk[1], 14, 1);
    host_preset(&trk[2], 0, lead_p);
    pat(&trk[2], 15, 0);
    host_preset(&trk[3], ENGI_DRUM, kit);
    groove(&trk[3], gr);
    wav_open(dir, name);
    transport_req = 1;
    run_for(8 * 4 * 60.0 / bpm);               /* 8 bars */
    transport_req = 2;
    run_for(2.0);
    wav_close();
}

int main(int argc, char **argv)
{
    const char *dir = argc > 1 ? argv[1] : ".";
    uint32_t p, i;
    song4(dir, "villera.wav", 112, 5, 0, 26, 7, 5, 12);       /* BOMBON, BAJO PESAD, CUMBIA ORG, PIBE LEAD */
    song4(dir, "colombia.wav", 100, 5, 2, 24, 0, 19, 17);     /* COLOMBIA, CUMBIA BAS, ACORDEON, FLAUTA */
    song4(dir, "amazonia.wav", 100, 4, 5, 24, 0, 27, 29);     /* AMAZONIA, CUMBIA BAS, FARFISA, GUITARRA */
    song.g[G_SWING] = 0;
    /* presets */
    host_tracks_init();
    song.master_q12 = 4096;
    song.g[G_BPM] = 100;
    wav_open(dir, "presets.wav");
    for (p = 0; p < 21u; p++) {                  /* ANALOG 12..29, WHEEL 5..7 */
        track_t *t = &trk[2];
        uint32_t e = p < 18u ? 0u : 7u, k = p < 18u ? 12u + p : 5u + p - 18u;
        host_preset(t, e, k);
        fprintf(stderr, "%5.1f s  %s %s\n", (double)wframes / FS, ENGINES[e]->name, ENGINES[e]->presets[k].name);
        uint32_t keys = e == 7u || (k >= 19u && k <= 23u) || k == 27u || k == 28u;
        pat(t, k == 24u || k == 26u ? 13u : keys ? 14u : 15u, keys);
        transport_req = 1;
        run_for(2 * 4 * 60.0 / 100);
        transport_req = 2;
        run_for(1.0);
        for (i = 0; i < 16u; i++)
            put_step(t, i, 0, 0, ST_REST, 0);
    }
    for (p = 4; p <= 5u; p++) {                  /* the kits' lanes, one by one, then the groove */
        track_t *t = &trk[3];
        host_preset(t, ENGI_DRUM, p);
        fprintf(stderr, "%5.1f s  DRUM %s\n", (double)wframes / FS, ENGINES[ENGI_DRUM]->presets[p].name);
        for (i = 0; i < 8u; i++) {
            input_on(t, DRUM_LANE_NOTE[i], 110);
            run_for(0.5);
            input_off(t, DRUM_LANE_NOTE[i]);
        }
        groove(t, p == 4u ? 1u : 0u);              /* CUMB: VILLERA, LATIN: BOMBON */
        transport_req = 1;
        run_for(2 * 4 * 60.0 / 100);
        transport_req = 2;
        run_for(1.0);
        for (i = 0; i < 16u; i++)
            put_step(t, i, 0, 0, ST_REST, 0), t->step[i].hit = t->step[i].acc = 0;
    }
    wav_close();
    return 0;
}
