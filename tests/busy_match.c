/* SPDX-License-Identifier: GPL-3.0-only */
/* (Cumbia) the PIBE LEAD preset against the measured Busy Boy: held notes through the firmware's own ANALOG voice
 * (hostsim.c), to WAV, for tools/busy_match.py. build/host/busy_match OUT.wav PRESET NOTE [NOTE..] */
#define main hostsim_main
#include "hostsim.c"
#undef main
int main(int argc, char **argv)
{
    FILE *w = fopen(argv[1], "wb");
    uint32_t pr = (uint32_t)atoi(argv[2]), a, i, f = 0;
    int32_t o[2 * CTL];
    host_tracks_init();
    song.master_q12 = 4096;
    host_preset(&trk[0], 0, pr);
    trk[0].p[P_CHOR] = trk[0].p[P_DLY] = trk[0].p[P_REV] = 0;   /* dry, as measured */
    if (getenv("E")) {                                /* E=w,dtn,mix,nois,cut,res,drv,ktr: the edit page's values */
        const char *q = getenv("E");
        for (i = 0; i < 8u && *q; i++) {
            trk[0].p[P_E0 + i] = (int16_t)atoi(q);
            while (*q && *q != ',') q++;
            if (*q) q++;
        }
    }
    if (getenv("LD")) trk[0].p[P_LD_PIT] = 0;
    wav_hdr(w, 0);
    for (a = 3; a < (uint32_t)argc; a++) {
        uint32_t n = (uint32_t)atoi(argv[a]);
        input_on(&trk[0], n, 100);
        for (i = 0; i < FS * (getenv("HOLD") ? (uint32_t)atoi(getenv("HOLD")) : 12u) / 10u / CTL; i++, f += CTL) { mix_block(o, CTL); 
            for (uint32_t j = 0; j < 2u * CTL; j++) { int16_t s = (int16_t)(o[j] > 32767 ? 32767 : o[j] < -32768 ? -32768 : o[j]); fwrite(&s, 2, 1, w); } }
        input_off(&trk[0], n);
        for (i = 0; i < FS * (getenv("HOLD") ? 5u : 3u) / 10u / CTL; i++, f += CTL) { mix_block(o, CTL);
            for (uint32_t j = 0; j < 2u * CTL; j++) { int16_t s = (int16_t)(o[j] > 32767 ? 32767 : o[j] < -32768 ? -32768 : o[j]); fwrite(&s, 2, 1, w); } }
    }
    fseek(w, 0, SEEK_SET);
    wav_hdr(w, f);
    fclose(w);
    return 0;
}
