#include <assert.h>
#include <stdio.h>
#define main example_main
#include "../program.c"
#undef main

static void check_star_bitmap(void)
{
    static uint32_t expected[65536];
    memset(expected, 0, sizeof(expected));
    for (int i = 0; i < sky.count; i++) {
        const struct Star* star = &sky.stars[i];
        assert(star->x >= -320 * 256 && star->x < 640 * 256);
        assert(star->y >= 0 && star->y < sky.height);
        if (star->x >= 0 && star->x < 320 * 256)
            expected[star->y * 320 + star->x / 256] = star->color;
    }
    assert(!memcmp(expected, test_bg[2], sizeof(expected)));
}

static void test_sky(void)
{
    for (int angle = 0; angle <= 75; angle++) {
        for (int focal = 100; focal <= 4096; focal += 37) {
            double sine = vgs_sin(angle) / 256.0, cosine = vgs_cos(angle) / 256.0;
            double top = 199 - 99 * cosine / (1 - 99 * sine / focal)
                             - 100 * cosine / (1 + 100 * sine / focal);
            int expected = top <= 0 ? 0 : (int)ceil(top - 1e-9);
            assert(sky_height_for(angle, focal) == expected);
        }
    }
    init_sky();
    assert(test_regs[11] == TRUE && test_regs[12] == TRUE);
    assert(sky.count == (sky.height ? STAR_COUNT : 0));
    assert(test_regs[28] == (sky.height == 0));
    assert(test_regs[29] == (sky.height == 0));
    for (int y = 0; y < 200; y++) {
        uint32_t blue = y < sky.height ? (sky.height == 1 ? 255 : 255 * (sky.height - 1 - y) / (sky.height - 1)) : 0;
        for (int x = 0; x < 320; x++) assert(test_bg[1][y * 320 + x] == blue);
    }
    for (int i = 0; i < sky.count; i++) {
        int brightness = (sky.stars[i].color >> 8) & 255;
        assert(brightness > 0);
        assert(sky.stars[i].color == (uint32_t)(brightness * 0x010100));
        assert(sky.stars[i].speed == 64 + brightness * 448 / 255);
    }
    check_star_bitmap();
    unsigned calls = test_pixel_calls;
    update_stars();
    assert(test_pixel_calls == calls);
    for (int i = 0; i < 1000; i++) {
        player.heading = (player.heading + 199) % (360 * 256);
        update_stars();
        check_star_bitmap();
    }
    /* Overlapping pixels, unequal depths, heading seam and both X boundaries. */
    vgs_cls_bg(2, 0);
    sky.height = 40;
    sky.count = 4;
    sky.heading = 359 * 256;
    sky.stars[0] = (struct Star){100 * 256, 10, 64, 0x202000};
    sky.stars[1] = (struct Star){100 * 256, 10, 512, 0xFFFF00};
    sky.stars[2] = (struct Star){-320 * 256, 20, 512, 0xFFFF00};
    sky.stars[3] = (struct Star){640 * 256 - 1, 20, 512, 0xFFFF00};
    draw_stars();
    player.heading = 0;
    update_stars();
    assert(sky.stars[0].x == 100 * 256 - 64);
    assert(sky.stars[1].x == 98 * 256);
    assert(sky.stars[2].x == 638 * 256);
    assert(test_bg[2][10 * 320 + 100] == 0);
    check_star_bitmap();
    player.heading = 358 * 256;
    update_stars();
    assert(sky.stars[0].x == 100 * 256 + 64);
    assert(sky.stars[1].x == 102 * 256);
    assert(sky.stars[3].x == -318 * 256 - 1);
    check_star_bitmap();
    init_player();
    init_sky();
    puts("OK: sky boundary, gradient, star range, parallax, wrap and overlap");
}

int main(void)
{
    int laps = 0;
    int previous_turn = 0;
    int reversals = 0;
    int fractional_frames = 0;
    assert(camera_range_valid(MAP_VERTICAL_CHIP_NUM, MAP_SCALE, vgs_cos(MAP_ANGLE)));
    assert(!camera_range_valid(25, 4, vgs_cos(75)));
    assert(camera_range_valid(25, 8, vgs_cos(75)));
    assert(!camera_range_valid(0, 100, 256));
    assert(!camera_range_valid(129, 100, 256));
    assert(!camera_range_valid(25, 100, 0));
    load_map();
    for (int y = 0; y < 128; y++) {
        for (int x = 0; x < 128; x++) {
            int pattern = 128 + rom_map[8 + y * 128 + x] * 4;
            assert(test_bg[0][y * 512 + x * 2] == (uint32_t)pattern);
            assert(test_bg[0][y * 512 + x * 2 + 1] == (uint32_t)pattern + 1);
            assert(test_bg[0][y * 512 + x * 2 + 256] == (uint32_t)pattern + 2);
            assert(test_bg[0][y * 512 + x * 2 + 257] == (uint32_t)pattern + 3);
        }
    }
    init_player();
    test_sky();
    assert(player.x == (114 * 16 + 8) * 256);
    assert(player.y == (91 * 16 + 8) * 256);
    for (int frame = 0; frame < 12000; frame++) {
        int old_next = player.next;
        int old_heading = player.heading;
        double old_x = player.x / 256.0, old_y = player.y / 256.0;
        move_player();
        if (player.next < old_next) laps++;
        double speed = hypot(player.x / 256.0 - old_x, player.y / 256.0 - old_y);
        if (!(fabs(speed - SPEED / 256.0) < 0.06)) fprintf(stderr, "frame=%d speed=%f segment=%d\n", frame, speed, player.next);
        assert(fabs(speed - SPEED / 256.0) < 0.06);
        assert(player.x > 0 && player.x < 2048 * 256);
        assert(player.y > 0 && player.y < 2048 * 256);
        int turn = (player.heading - old_heading + 540 * 256) % (360 * 256) - 180 * 256;
        assert(turn >= -256 && turn <= 256);
        if (frame >= 100 && turn) {
            if (previous_turn && (turn > 0) != (previous_turn > 0)) reversals++;
            previous_turn = turn;
        }
        if (player.heading % 256) fractional_frames++;
        {
            int32_t current_x, current_y;
            look_ahead(0, &current_x, &current_y);
            assert(current_x == player.x && current_y == player.y);
        }
        update_camera();
        assert(((int32_t)VGS_VREG_M7_TX0 + CAMERA_X) * 256 + (int)(VGS_VREG_M7_FRAC0 & 255) == player.x);
        assert(((int32_t)VGS_VREG_M7_TY0 + (int32_t)VGS_VREG_M7_CY0) * 256 + (int)((VGS_VREG_M7_FRAC0 >> 8) & 255) == player.y);
        assert(VGS_VREG_M7_EN0 == 1);
        assert(VGS_VREG_M7_DEPTH0 == MAP_ANGLE_DEPTH);
        assert(VGS_VREG_M7_FOCAL0 == MAP_FOCAL_LENGTH);
        {
            double sine = vgs_sin(MAP_ANGLE_DEPTH) / 256.0;
            double cosine = vgs_cos(MAP_ANGLE_DEPTH) / 256.0;
            double near_y = 99 * cosine / (1 - 99 * sine / MAP_FOCAL_LENGTH);
            double top = 199 - near_y - 100 * cosine / (1 + 100 * sine / MAP_FOCAL_LENGTH);
            double v = (int32_t)VGS_VREG_M7_CY0 - 100;
            double anchor = 199 - near_y + v * cosine / (1 - v * sine / MAP_FOCAL_LENGTH);
            assert(fabs(anchor - (top + (199 - top) * CAMERA_GROUND_PERCENT / 100.0)) <= 2);
        }
        for (int i = 45; i <= 57; i += 4) {
            assert((int32_t)test_regs[i] >= -32768 && (int32_t)test_regs[i] <= 32767);
        }
    }
    assert(laps >= 4);
    /* This circuit changes curvature direction, but must not oscillate at
     * every waypoint. Previously there were over 400 reversals per lap. */
    assert(reversals < 25 * (laps + 1));
    assert(fractional_frames > 1000);
    player.heading = 270 * 256;
    update_camera();
    assert((int32_t)VGS_VREG_M7_A0 == 256 * 100 / MAP_SCALE);
    {
        double expected_vertical_span = MAP_VERTICAL_CHIP_NUM * 16.0 * 100 / MAP_SCALE * 256 / vgs_cos(MAP_ANGLE);
        double rendered_vertical_span = (int32_t)VGS_VREG_M7_D0 / 256.0 * 200;
        assert(fabs(rendered_vertical_span - expected_vertical_span) < 200.0 / 256);
        /* Horizontal scale and the camera anchor are independent of chip count. */
        assert((int32_t)VGS_VREG_M7_A0 == 256 * 100 / MAP_SCALE);
    }
    assert(VGS_VREG_M7_B0 == 0 && VGS_VREG_M7_C0 == 0);
    /* The interpolated aim point must remain continuous across the lap seam. */
    player.next = ROUTE_COUNT - 1;
    player.from_x = route[ROUTE_COUNT - 2].x;
    player.from_y = route[ROUTE_COUNT - 2].y;
    player.length = route[ROUTE_COUNT - 2].length;
    player.distance = player.length - 1;
    {
        int32_t x1, y1, x2, y2;
        look_ahead(64 * 256, &x1, &y1);
        player.distance++;
        look_ahead(64 * 256, &x2, &y2);
        assert(hypot(x2 - x1, y2 - y1) < 3);
    }
    printf("OK: four laps, continuous aim, smooth subdegree camera (%d reversals)\n", reversals);
    return 0;
}
