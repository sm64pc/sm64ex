enum newcam_flagvalues
{
    NC_FLAG_XTURN = 0x0001,//If this flag is set, the camera's yaw can be moved by the player.
    NC_FLAG_YTURN = 0x0002, //If this flag is set, the camera's pitch can be moved by the player.
    NC_FLAG_ZOOM = 0x0004, //If this flag is set, the camera's distance can be set by the player.
    NC_FLAG_8D = 0x0008, //If this flag is set, the camera will snap to an 8 directional axis
    NC_FLAG_4D = 0x0010, //If this flag is set, the camera will snap to a 4 directional axis
    NC_FLAG_2D = 0x0020, //If this flag is set, the camera will stick to 2D.
    NC_FLAG_FOCUSX = 0x0040, //If this flag is set, the camera will point towards its focus on the X axis.
    NC_FLAG_FOCUSY = 0x0080, //If this flag is set, the camera will point towards its focus on the Y axis.
    NC_FLAG_FOCUSZ = 0x0100, //If this flag is set, the camera will point towards its focus on the Z axis.
    NC_FLAG_POSX = 0x0200, //If this flag is set, the camera will move along the X axis.
    NC_FLAG_POSY = 0x0400, //If this flag is set, the camera will move along the Y axis.
    NC_FLAG_POSZ = 0x0800, //If this flag is set, the camera will move along the Z axis.
    NC_FLAG_COLLISION = 0x1000, //If this flag is set, the camera will collide and correct itself with terrain.
    NC_FLAG_SLIDECORRECT = 0x2000, //If this flag is set, the camera will attempt to centre itself behind Mario whenever he's sliding.

    NC_MODE_NORMAL = NC_FLAG_XTURN | NC_FLAG_YTURN | NC_FLAG_ZOOM | NC_FLAG_FOCUSX | NC_FLAG_FOCUSY | NC_FLAG_FOCUSZ | NC_FLAG_POSX | NC_FLAG_POSY | NC_FLAG_POSZ | NC_FLAG_COLLISION,
    NC_MODE_SLIDE = NC_FLAG_XTURN | NC_FLAG_YTURN | NC_FLAG_ZOOM | NC_FLAG_FOCUSX | NC_FLAG_FOCUSY | NC_FLAG_FOCUSZ | NC_FLAG_POSX | NC_FLAG_POSY | NC_FLAG_POSZ | NC_FLAG_COLLISION | NC_FLAG_SLIDECORRECT,
    NC_MODE_FIXED = NC_FLAG_XTURN | NC_FLAG_YTURN | NC_FLAG_ZOOM | NC_FLAG_FOCUSX | NC_FLAG_FOCUSY | NC_FLAG_FOCUSZ,
    NC_MODE_2D = NC_FLAG_XTURN | NC_FLAG_YTURN | NC_FLAG_ZOOM | NC_FLAG_FOCUSX | NC_FLAG_FOCUSY | NC_FLAG_FOCUSZ | NC_FLAG_POSX | NC_FLAG_POSY | NC_FLAG_POSZ | NC_FLAG_COLLISION,
    NC_MODE_8D = NC_FLAG_XTURN | NC_FLAG_YTURN | NC_FLAG_ZOOM | NC_FLAG_8D | NC_FLAG_FOCUSX | NC_FLAG_FOCUSY | NC_FLAG_FOCUSZ | NC_FLAG_POSX | NC_FLAG_POSY | NC_FLAG_POSZ | NC_FLAG_COLLISION,
    NC_MODE_FIXED_NOMOVE = 0x0000,
    NC_MODE_NOTURN = NC_FLAG_ZOOM | NC_FLAG_FOCUSX | NC_FLAG_FOCUSY | NC_FLAG_FOCUSZ | NC_FLAG_POSX | NC_FLAG_POSY | NC_FLAG_POSZ | NC_FLAG_COLLISION,
    NC_MODE_NOROTATE = NC_FLAG_YTURN | NC_FLAG_ZOOM | NC_FLAG_FOCUSX | NC_FLAG_FOCUSY | NC_FLAG_FOCUSZ | NC_FLAG_POSX | NC_FLAG_POSY | NC_FLAG_POSZ | NC_FLAG_COLLISION

};

extern void newcam_init_settings(void);
extern void newcam_disable(void);
extern void newcam_diagnostics(void);

extern s16 newcam_sensitivityX; //How quick the camera works.
extern s16 newcam_sensitivityY;
extern s16 newcam_invertX;
extern s16 newcam_invertY;
extern s16 newcam_panlevel; //How much the camera sticks out a bit in the direction you're looking.
extern s16 newcam_aggression; //How much the camera tries to centre itself to Mario's facing and movement.
extern u8 newcam_active; // basically the thing that governs if newcam is on.
extern s16 newcam_analogue;
extern u16 newcam_intendedmode;

extern u16 newcam_mode;
extern s16 newcam_yaw;
