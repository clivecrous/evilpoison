#ifndef _KEYMAP_H
#define _KEYMAP_H

#define PREFIX_KEY XK_e
#define PREFIX_MOD ControlMask

enum KEYVALS {
    KEY_NONE,
    KEY_PREFIX,
    KEY_MOVEWIN,
    KEY_NEXT,
    KEY_NEW,
    KEY_TOPLEFT,
    KEY_TOPRIGHT,
    KEY_BOTTOMLEFT,
    KEY_BOTTOMRIGHT,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_DOWN,
    KEY_UP,
    KEY_RESIZELEFT,
    KEY_RESIZERIGHT,
    KEY_RESIZEDOWN,
    KEY_RESIZEUP,

    KEY_MOUSEDRAG,
    KEY_MOUSESWEEP,

    KEY_LOWER,
    KEY_INFO,
    KEY_MAXVERT,
    KEY_MAXHORIZ,
    KEY_MAX,

    KEY_VSPLIT,
    KEY_HSPLIT,

#ifdef VWM
    KEY_FIX,
    KEY_PREVDESK,
    KEY_NEXTDESK,
    KEY_DESK1,
    KEY_DESK2,
    KEY_DESK3,
    KEY_DESK4,
    KEY_DESK5,
    KEY_DESK6,
    KEY_DESK7,
    KEY_DESK8,
#endif
    KEY_KILL
};

#define KEY_TO_VDESK(key) ((key) - KEY_DESK1)
#define valid_vdesk(v) ((unsigned)(v) < KEY_TO_VDESK(KEY_DESK8))

#endif
