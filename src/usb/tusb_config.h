#pragma once

#define CFG_TUSB_RHPORT0_MODE (OPT_MODE_DEVICE | OPT_MODE_FULL_SPEED)

#define CFG_TUD_ENDPOINT0_SIZE 64

#define CFG_TUD_MIDI 1
#define CFG_TUD_VENDOR 1

#if TUD_OPT_HIGH_SPEED
#define CFG_BUFSIZE 512
#else
#define CFG_BUFSIZE 64
#endif

#define CFG_TUD_MIDI_RX_BUFSIZE CFG_BUFSIZE
#define CFG_TUD_MIDI_TX_BUFSIZE CFG_BUFSIZE

#define CFG_TUD_VENDOR_RX_BUFSIZE CFG_BUFSIZE
#define CFG_TUD_VENDOR_TX_BUFSIZE CFG_BUFSIZE