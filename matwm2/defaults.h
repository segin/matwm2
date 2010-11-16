#define DEF_CFG "\
background            darkred\n\
inactive_background   dimgray\n\
foreground            lightgray\n\
inactive_foreground   darkgray\n\
font                  fixed\n\
border_width          4\n\
title_spacing         2\n\
buttons_left          sticky ontop\n\
buttons_right         iconify expand maximize close\n\
center_title          false\n\
center_wlist_items		false\n\
desktops              4\n\
snap                  5\n\
taskbar_ontop         false\n\
focus_new             false\n\
click_focus           false\n\
click_raise           false\n\
doubleclick_time      300\n\
doubleclick           maximize\n\
button1               move\n\
button2               none\n\
button3               resize\n\
button4               raise\n\
button5               lower\n\
mouse_modifier        mod1\n\
no_snap_modifier      shift\n\
ignore_modifier       mod2 lock\n\
key                   mod1 Tab                next\n\
key                   mod1 shift Tab          prev\n\
key                   control mod1 s          iconify\n\
key                   control mod1 x          maximize\n\
key                   control mod1 h          maximize h\n\
key                   control mod1 v          maximize v\n\
key                   control mod1 f          fullscreen\n\
key                   control mod1 e          expand\n\
key                   control mod1 r          expand a\n\
key                   control mod1 z          expand h\n\
key                   control mod1 j          expand v\n\
key                   control mod1 q          close\n\
key                   control mod1 d          sticky\n\
key                   control mod1 t          title\n\
key                   control mod1 y          to_border tl\n\
key                   control mod1 u          to_border tr\n\
key                   control mod1 b          to_border bl\n\
key                   control mod1 n          to_border br\n\
key                   control mod1 o          ontop\n\
key                   control mod1 p          below\n\
key                   control mod1 a          iconify_all\n\
key                   control mod1 Right      next_desktop\n\
key                   control mod1 Left       prev_desktop\n\
key                   control mod1 Return     exec xterm\n\
key                   mod4 s                  iconify\n\
key                   mod4 x                  maximize\n\
key                   mod4 h                  maximize h\n\
key                   mod4 v                  maximize v\n\
key                   mod4 f                  fullscreen\n\
key                   mod4 e                  expand\n\
key                   mod4 r                  expand a\n\
key                   mod4 z                  expand h\n\
key                   mod4 j                  expand v\n\
key                   mod4 q                  close\n\
key                   mod4 d                  sticky\n\
key                   mod4 t                  title\n\
key                   mod4 y                  to_border tl\n\
key                   mod4 u                  to_border tr\n\
key                   mod4 b                  to_border bl\n\
key                   mod4 n                  to_border br\n\
key                   mod4 o                  ontop\n\
key                   mod4 p                  below\n\
key                   mod4 a                  iconify_all\n\
key                   mod4 Right              next_desktop\n\
key                   mod4 Left               prev_desktop\n"

#define MINSIZE                 5
#define CFGFN                   ".matwmrc"
#define GCFGFN                  "/etc/matwmrc"
#define NO_TITLE                "-"

