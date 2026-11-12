/* SPDX-License-Identifier: GPL-3.0-only */
/* (Cumbia) SONG > BUILD (firmware/src/cumbia_song.c) on the real UI and sequencer (the stubs of tests/ui_test.c):
 *   keys      on GLO > SONG, an empty song lights E4 .. G5; a key opens BUILD <STYLE> SONG?, OCT- leaves everything,
 *             OCT+ builds
 *   the song  12 sections over projects A..D, saved and named <STYLE> A..D, the sounds of the style on T1..T4 (bass
 *             ANALOG, DRUM's kit), its tempo and the song's swing (every track's own 0), the tracks left with the verse
 *   parts     the bass's first step the key's root, accented; the drums the style's groove (B: the cowbell up; D: the
 *             roll with ratchets); the lead silent in A, a hook in B, a solo in C, the root held in D
 *   playing   the song starts at once, the intro on the drums alone, goes through its 66 bars and stops; it sounds
 * Built and run by tests/run_tests.sh. */
#define UI_TEST_NO_MAIN 1
#include "ui_test.c"

static uint32_t peak_blocks(uint32_t n)
{
    int32_t o[2 * CTL];
    uint32_t i, pk = 0;
    while (n--) {
        mix_block(o, CTL);                               /* (the sequencer's events with it, as the audio ISR) */
        for (i = 0; i < 2u * CTL; i++) {
            uint32_t a = (uint32_t)(o[i] < 0 ? -o[i] : o[i]);
            pk = a > pk ? a : pk;
        }
    }
    return pk;
}

/* DEMO=dir: each style's built song to dir/song-<style>.wav (SECS=n: its first n seconds; default the whole song) */
static void demo(const char *dir)
{
    uint32_t s;
    for (s = 0; s < CS_N; s++) {
        char p[512];
        FILE *w;
        uint32_t n = 0, max = (getenv("SECS") ? (uint32_t)atoi(getenv("SECS")) : 400u) * FS / CTL, tail = 0;
        int32_t o[2 * CTL];
        snprintf(p, sizeof p, "%s/song-%s.wav", dir, CS_STYLE[s].name);
        w = fopen(p, "wb");
        if (!w)
            return;
        fwrite("RIFF\0\0\0\0WAVEfmt \x10\0\0\0\x01\0\x02\0\x44\xac\0\0\x10\xb1\x02\0\x04\0\x10\0data\0\0\0\0", 1, 44, w);
        ui_power_on();
        song.master_q12 = 4096;
        srand(s + 1u);
        cumbia_build(s);
        while (n < max && tail < 2u * FS / CTL) {
            uint32_t i;
            mix_block(o, CTL);
            for (i = 0; i < 2u * CTL; i++) {
                int16_t v = (int16_t)(o[i] > 32767 ? 32767 : o[i] < -32768 ? -32768 : o[i]);
                fwrite(&v, 2, 1, w);
            }
            n++;
            tail += !chain.running;
        }
        {
            uint32_t bytes = n * CTL * 4u, riff = bytes + 36u;
            fseek(w, 4, SEEK_SET); fwrite(&riff, 4, 1, w);
            fseek(w, 40, SEEK_SET); fwrite(&bytes, 4, 1, w);
        }
        fclose(w);
        printf("cumbia_song: %s %.1f s\n", p, (double)n * CTL / FS);
    }
}

int main(void)
{
    if (getenv("DEMO")) {
        demo(getenv("DEMO"));
        return 0;
    }
    uint32_t s, i, k;
    int bad = 0, ok;
    char nm[PROJ_NAME_LEN + 1];
    for (s = 0; s < CS_N; s++) {
        const cs_style_t *y = &CS_STYLE[s];
        ui_power_on();
        go_page(GR_SONG);
        frame();
        ok = (song_leds() >> CS_KEY[s]) & 1u;
        fm1_in.notes = host_notes = 1u << CS_KEY[s];
        frame();
        fm1_in.notes = host_notes = 0;
        frame();
        ok &= ui.confirm == CF_BUILD_SONG && ui.confirm_trk == s;
        press(B_OCTDN);
        ok &= ui.confirm == CF_NONE && chain_config.count == 0u && !project_used(0);
        fm1_in.notes = host_notes = 1u << CS_KEY[s];
        frame();
        fm1_in.notes = host_notes = 0;
        frame();
        press(B_OCTUP);
        ok &= chain_config.count == 12u && song.g[G_BPM] == y->bpm && song.g[G_SWING] == y->swing;
        for (k = 0; k < 4u; k++) {
            ok &= project_used(k) && project_name(k, nm) && nm[str_len(nm) - 1u] == (char)('A' + k);
        }
        ok &= trk[0].eng_req == 0u && trk[0].preset == y->bass && trk[3].eng_req == ENGI_DRUM && trk[3].preset == y->kit &&
              trk[1].eng_req == y->keys_e && trk[1].preset == y->keys_p && trk[2].eng_req == y->lead_e &&
              trk[2].preset == y->lead_p;
        for (i = 0; i < NTRK; i++)
            ok &= trk[i].p[P_SSWING] == 0 && trk[i].p[P_SLEN] == 16;
        /* the verse in the tracks: the bass's root, the groove, the lead silent */
        ok &= trk[0].step[0].time == ST_NOTE && trk[0].step[0].note[0] == cs_root(y, 0) && (trk[0].step[0].flags & SF_ACCENT);
        for (i = 0; i < 16u; i++) {
            ok &= trk[3].step[i].hit == GROOVES[y->groove].hit[i];
            ok &= trk[2].step[i].time == ST_REST;
        }
        ok &= chain.armed || chain.running;
        bad += check(y->name, ok);
        seq_stop();
        events_block(CTL);
    }
    {   /* the sections as saved: B's cowbell, C's solo, D's roll and held root */
        const cs_style_t *y = &CS_STYLE[0];
        uint32_t notes = 0, rat = 0;
        ui_power_on();
        cumbia_build(0);
        seq_stop();
        events_block(CTL);
        project_load(1);
        ok = (trk[3].step[2].hit & CS_B_LANE) != 0;
        for (i = 0; i < 16u; i++)
            notes += trk[2].step[i].time == ST_NOTE;
        ok &= notes >= 4u;
        project_load(2);
        for (i = 0, notes = 0; i < 16u; i++)
            notes += trk[2].step[i].time == ST_NOTE;
        ok &= notes >= 6u;
        project_load(3);
        for (i = 12; i < 16u; i++)
            rat += (trk[3].step[i].flags & SF_RATCH) != 0;
        ok &= rat == 4u && trk[0].step[0].note[0] == cs_root(y, 0) && trk[0].step[1].time == ST_TIE &&
              trk[2].step[15].time == ST_TIE;
        bad += check("sections: B the cowbell up and the hook, C the solo, D the roll (ratchets) and the held root", ok);
    }
    {   /* it plays: the intro sounds on the drums, the whole song runs and stops */
        uint32_t n = 0, pk;
        ui_power_on();
        song.master_q12 = 4096;                          /* (the MASTER pot, as hostsim) */
        cumbia_build(0);
        pk = peak_blocks(FS / CTL);                       /* the first second */
        ok = chain.running && pk > 500u && !trk[0].v[0].active;
        while (chain.running && n < 200u * FS / CTL) {
            events_block(CTL);
            n++;
        }
        ok &= !chain.running && n > 130u * FS / CTL;     /* 66 bars at 112 BPM: 141 s */
        printf("cumbia_song: VILLERA plays %.1f s (peak in the intro %u)\n", (double)n * CTL / FS, pk);
        bad += check("the song plays: the intro (drums alone) sounds, all 66 bars, then it stops", ok);
    }
    printf(bad ? "cumbia_song: FAILED\n" : "cumbia_song: all checks ok\n");
    return bad != 0;
}
