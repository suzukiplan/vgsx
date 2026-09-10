#include "vgs.h"

/* Initial sprite top-left, in 16x16 map chips. */
#ifndef PLAYER_INIT_X
#define PLAYER_INIT_X 114
#endif
#ifndef PLAYER_INIT_Y
#define PLAYER_INIT_Y 91
#endif
#ifndef MAP_ANGLE
#define MAP_ANGLE 0
#endif
#ifndef MAP_ANGLE_DEPTH
#define MAP_ANGLE_DEPTH 66
#endif
/* Vertical reference span before MAP_SCALE and MAP_ANGLE (16px per chip). */
#ifndef MAP_VERTICAL_CHIP_NUM
#define MAP_VERTICAL_CHIP_NUM 128
#endif
#ifndef MAP_FOCAL_LENGTH
#define MAP_FOCAL_LENGTH 128
#endif
#ifndef CAMERA_GROUND_PERCENT
#define CAMERA_GROUND_PERCENT 80
#endif
#ifndef MAP_SCALE
#define MAP_SCALE 400
#endif

#ifndef STAR_COUNT
#define STAR_COUNT 200
#endif
#if STAR_COUNT < 0 || STAR_COUNT > 1024
#error "STAR_COUNT must be in 0..1024"
#endif

#if PLAYER_INIT_X < 0 || PLAYER_INIT_X >= 128 || PLAYER_INIT_Y < 0 || PLAYER_INIT_Y >= 128
#error "PLAYER_INIT_X/Y must be map-chip coordinates in 0..127"
#endif
#if MAP_ANGLE < 0 || MAP_ANGLE > 75
#error "MAP_ANGLE must be in 0..75 degrees"
#endif
#if MAP_ANGLE_DEPTH < 0 || MAP_ANGLE_DEPTH > 75
#error "MAP_ANGLE_DEPTH must be in 0..75 degrees"
#endif
#if MAP_VERTICAL_CHIP_NUM < 1 || MAP_VERTICAL_CHIP_NUM > 128
#error "MAP_VERTICAL_CHIP_NUM must be in 1..128 chips"
#endif
#if MAP_FOCAL_LENGTH < 100 || MAP_FOCAL_LENGTH > 4096
#error "MAP_FOCAL_LENGTH must be in 100..4096 pixels"
#endif
#if CAMERA_GROUND_PERCENT < 0 || CAMERA_GROUND_PERCENT > 100
#error "CAMERA_GROUND_PERCENT must be in 0..100"
#endif
#if MAP_SCALE < 4 || MAP_SCALE > 3200
#error "MAP_SCALE must be in 4..3200 percent (Mode 7 coefficient range)"
#endif

#define MAP_WIDTH 128
#define MAP_HEIGHT 128
#define MAP_PATTERN 128 /* font.chr precedes mapchip.chr in the ROM. */
#define PLAYER_HALF_SIZE 8
#define CAMERA_X 160
#define CAMERA_Y 100
#define SPEED 1280 /* Q8.8 world pixels per frame: 5px at 60fps. */

extern const uint8_t rom_map[16392];
#include "route.h"
#define ROUTE_COUNT ((int)(sizeof(route) / sizeof(route[0])))

static struct {
    int32_t x, y;
    int32_t from_x, from_y;
    int32_t distance, length;
    int32_t heading; /* Q8.8 degrees, wrapped to 0..360. */
    int next;
} player;

static struct {
    int height;
    int count;
    int32_t heading;
    struct Star {
        int32_t x; /* Q8.8, in [-320, 640). */
        int y;
        int32_t speed; /* Q8.8 pixels per degree. */
        uint32_t color;
    } stars[1024];
} sky;

/* Ceiling of the projected ground's top edge, using the VDP's trig values.
 * Exact integer fractions avoid leaving a seam or painting over the ground. */
static int sky_height_for(int angle, int focal)
{
    long long sine = vgs_sin(angle);
    long long cosine = vgs_cos(angle);
    long long near_den = focal * 256LL - 99 * sine;
    long long far_den = focal * 256LL + 100 * sine;
    long long denominator = near_den * far_den;
    long long top = 199 * denominator - 99 * cosine * focal * far_den - 100 * cosine * focal * near_den;
    if (top <= 0) return 0;
    int height = (top + denominator - 1) / denominator;
    return height > 200 ? 200 : height;
}

static void draw_stars(void)
{
    int i;
    for (i = 0; i < sky.count; i++) {
        const struct Star* star = &sky.stars[i];
        if (0 <= star->x && star->x < 320 * 256) {
            vgs_draw_pixel(2, star->x / 256, star->y, star->color);
        }
    }
}

static void init_sky(void)
{
    int i;
    vgs_draw_mode(1, TRUE);
    vgs_draw_mode(2, TRUE);
    vgs_cls_bg(1, 0);
    vgs_cls_bg(2, 0);
    sky.height = sky_height_for(MAP_ANGLE_DEPTH, MAP_FOCAL_LENGTH);
    sky.count = sky.height ? STAR_COUNT : 0;
    sky.heading = player.heading;
    vgs_skip_bg(1, sky.height == 0);
    vgs_skip_bg(2, sky.height == 0);
    for (i = 0; i < sky.height; i++) {
        uint32_t blue = sky.height > 1 ? 255 * (sky.height - 1 - i) / (sky.height - 1) : 255;
        vgs_draw_lineH(1, 0, i, 320, blue);
    }
    for (i = 0; i < sky.count; i++) {
        struct Star* star = &sky.stars[i];
        int brightness = 1 + vgs_rand() % 255;
        star->x = ((int)(vgs_rand() % 960) - 320) * 256;
        star->y = vgs_rand() % sky.height;
        star->color = (brightness << 16) | (brightness << 8);
        star->speed = 64 + brightness * 448 / 255;
    }
    draw_stars();
}

static void update_stars(void)
{
    int i;
    int32_t turn = (player.heading - sky.heading + 540 * 256) % (360 * 256) - 180 * 256;
    sky.heading = player.heading;
    if (!turn || !sky.count) return;
    /* Clear all old pixels first, so overlapping stars do not erase one another
     * during redraw. The separate sky layer never needs repainting. */
    for (i = 0; i < sky.count; i++) {
        const struct Star* star = &sky.stars[i];
        if (0 <= star->x && star->x < 320 * 256) {
            vgs_draw_pixel(2, star->x / 256, star->y, 0);
        }
    }
    for (i = 0; i < sky.count; i++) {
        struct Star* star = &sky.stars[i];
        // Turning right moves the scenery left; bright (near) stars move faster.
        star->x -= turn * star->speed / 256;
        if (star->x < -320 * 256) star->x += 960 * 256;
        if (star->x >= 640 * 256) star->x -= 960 * 256;
    }
    draw_stars();
}

static uint32_t square_root(uint32_t value)
{
    uint32_t result = 0;
    uint32_t bit = 1U << 30;
    while (bit > value) bit >>= 2;
    while (bit) {
        if (value >= result + bit) {
            value -= result + bit;
            result = (result >> 1) + bit;
        } else {
            result >>= 1;
        }
        bit >>= 2;
    }
    return result;
}

static void load_map(void)
{
    int x, y;
    vgs_cls_bg_all(0);
    vgs_draw_mode(0, FALSE);
    vgs_skip_bg(1, TRUE);
    vgs_skip_bg(2, TRUE);
    vgs_skip_bg(3, TRUE);
    VGS_VREG_SX0 = 0;
    VGS_VREG_SY0 = 0;
    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            /* map.c has an 8-byte big-endian size header and zero-based chip IDs.
             * bmp2chr -s 1 stores each chip's four patterns consecutively. */
            uint32_t pattern = MAP_PATTERN + rom_map[8 + y * MAP_WIDTH + x] * 4;
            vgs_put_bg(0, x * 2, y * 2, pattern);
            vgs_put_bg(0, x * 2 + 1, y * 2, pattern + 1);
            vgs_put_bg(0, x * 2, y * 2 + 1, pattern + 2);
            vgs_put_bg(0, x * 2 + 1, y * 2 + 1, pattern + 3);
        }
    }
}

static void init_player(void)
{
    uint32_t closest = 0xFFFFFFFF;
    int nearest = 0;
    int i;
    player.x = (PLAYER_INIT_X * 16 + PLAYER_HALF_SIZE) * 256;
    player.y = (PLAYER_INIT_Y * 16 + PLAYER_HALF_SIZE) * 256;
    for (i = 0; i < ROUTE_COUNT; i++) {
        int32_t dx = (route[i].x - player.x) / 256;
        int32_t dy = (route[i].y - player.y) / 256;
        uint32_t distance = dx * dx + dy * dy;
        if (distance < closest) {
            closest = distance;
            nearest = i;
        }
    }
    /* Join the lane gradually, retaining the requested initial position. */
    player.next = (nearest + 6) % ROUTE_COUNT;
    player.from_x = player.x;
    player.from_y = player.y;
    player.distance = 0;
    {
        int32_t dx = (route[player.next].x - player.x) / 16;
        int32_t dy = (route[player.next].y - player.y) / 16;
        player.length = square_root((uint32_t)(dx * dx) + (uint32_t)(dy * dy)) * 16;
        if (!player.length) player.length = 1;
    }
    player.heading = vgs_degree(player.x / 256, player.y / 256,
                                route[player.next].x / 256, route[player.next].y / 256) *
                     256;
}

/* Follow a point at a fixed arc distance, not a waypoint that jumps every 8px. */
static void look_ahead(int32_t distance, int32_t* x, int32_t* y)
{
    int next = player.next;
    int32_t length = player.length;
    int32_t from_x = player.from_x;
    int32_t from_y = player.from_y;
    distance += player.distance;
    while (distance >= length) {
        distance -= length;
        from_x = route[next].x;
        from_y = route[next].y;
        length = route[next].length;
        next = (next + 1) % ROUTE_COUNT;
    }
    *x = from_x + (long long)(route[next].x - from_x) * distance / length;
    *y = from_y + (long long)(route[next].y - from_y) * distance / length;
}

static void move_player(void)
{
    int32_t target, turn;
    int32_t ahead_x, ahead_y;
    player.distance += SPEED;
    while (player.distance >= player.length) {
        player.distance -= player.length;
        player.from_x = route[player.next].x;
        player.from_y = route[player.next].y;
        player.length = route[player.next].length;
        player.next = (player.next + 1) % ROUTE_COUNT;
    }
    player.x = player.from_x + (long long)(route[player.next].x - player.from_x) * player.distance / player.length;
    player.y = player.from_y + (long long)(route[player.next].y - player.from_y) * player.distance / player.length;

    /* Retain subpixel coordinates and subdegree camera motion. The low-pass
     * response has no overshoot, including when crossing 0/360 degrees. */
    look_ahead(64 * 256, &ahead_x, &ahead_y);
    target = vgs_degree(player.x, player.y, ahead_x, ahead_y) * 256;
    turn = (target - player.heading + 540 * 256) % (360 * 256) - 180 * 256;
    turn /= 8;
    if (turn > 256) turn = 256;
    if (turn < -256) turn = -256;
    player.heading = (player.heading + turn + 360 * 256) % (360 * 256);
}

static int camera_range_valid(int chips, int scale, int tilt_cosine)
{
    if (chips < 1 || chips > 128 || scale < 4 || scale > 3200 || tilt_cosine <= 0) return FALSE;
    return 65536LL * 100 * chips * 16 / (200LL * scale * tilt_cosine) <= 32767;
}

static void update_camera(void)
{
    int32_t degree = player.heading / 256;
    int32_t fraction = player.heading % 256;
    int32_t sine = vgs_sin(degree) * (256 - fraction) + vgs_sin((degree + 1) % 360) * fraction;
    int32_t cosine = vgs_cos(degree) * (256 - fraction) + vgs_cos((degree + 1) % 360) * fraction;
    int32_t tilt = vgs_cos(MAP_ANGLE);
    int32_t depth_sine = vgs_sin(MAP_ANGLE_DEPTH);
    /* Inverse-project a point 80% down the visible ground. This keeps more of
     * the course ahead in view while the future player stays at a fixed anchor. */
    int32_t near_weight = 99 * CAMERA_GROUND_PERCENT - 100 * (100 - CAMERA_GROUND_PERCENT);
    int32_t far_weight = 99 * (100 - CAMERA_GROUND_PERCENT) - 100 * CAMERA_GROUND_PERCENT;
    long long depth_numerator = (long long)MAP_FOCAL_LENGTH * 256 * near_weight + 990000LL * depth_sine;
    int32_t depth_denominator = MAP_FOCAL_LENGTH * 256 * 100 - depth_sine * far_weight;
    int32_t camera_y = CAMERA_Y + (depth_numerator >= 0
                                       ? (depth_numerator + depth_denominator / 2) / depth_denominator
                                       : -((-depth_numerator + depth_denominator / 2) / depth_denominator));

    /* Inverse mapping: screen right is the player's right; screen up is forward.
     * Tilt compresses the displayed depth axis by cos(MAP_ANGLE). A single
     * affine matrix supplies the tilt; M7_DEPTH adds the perspective warp. */
    VGS_VREG_M7_A0 = -sine * 100 / (MAP_SCALE * 256);
    VGS_VREG_M7_B0 = -(long long)cosine * 100 * MAP_VERTICAL_CHIP_NUM * 16 / (200LL * MAP_SCALE * tilt);
    VGS_VREG_M7_C0 = cosine * 100 / (MAP_SCALE * 256);
    VGS_VREG_M7_D0 = -(long long)sine * 100 * MAP_VERTICAL_CHIP_NUM * 16 / (200LL * MAP_SCALE * tilt);
    VGS_VREG_M7_CX0 = CAMERA_X;
    VGS_VREG_M7_CY0 = camera_y;
    /* Preserve the shared subpixel phase on both axes: rounding X/Y separately
     * makes a diagonal trajectory alternate sideways by a whole source pixel. */
    VGS_VREG_M7_TX0 = player.x / 256 - CAMERA_X;
    VGS_VREG_M7_TY0 = player.y / 256 - camera_y;
    VGS_VREG_M7_FRAC0 = (player.x & 255) | ((player.y & 255) << 8);
    VGS_VREG_M7_DEPTH0 = MAP_ANGLE_DEPTH;
    VGS_VREG_M7_FOCAL0 = MAP_FOCAL_LENGTH;
    VGS_VREG_M7_EN0 = 1;
}

int main(void)
{
    if (!camera_range_valid(MAP_VERTICAL_CHIP_NUM, MAP_SCALE, vgs_cos(MAP_ANGLE))) {
        const char* error = "Mode 7: increase MAP_SCALE or reduce MAP_VERTICAL_CHIP_NUM/MAP_ANGLE.\n";
        while (*error) VGS_OUT_CONSOLE = *error++;
        return 1;
    }
    load_map();
    init_player();
    update_camera();
    init_sky();
    while (ON) {
        vgs_vsync();
        move_player();
        update_camera();
        update_stars();
    }
    return 0;
}
