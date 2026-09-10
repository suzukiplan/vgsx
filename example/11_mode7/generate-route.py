#!/usr/bin/env python3
"""Generate a lane with about 16px of sprite-edge clearance (stdlib only)."""

from collections import deque
import json
import math
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
WIDTH = 512
WORLD_SCALE = 4
# 16px clearance + 8px sprite half-width + one source pixel of smoothing allowance.
LANE_RADIUS = 7


def read_road():
    metadata = json.loads((ROOT / "mapchip-patterns.json").read_text())
    raw = bytes(int(value, 16) for value in re.findall(
        r"0x([0-9a-fA-F]{2})", (ROOT / "map.c").read_text()))
    if len(raw) != 16392 or raw[:8] != bytes.fromhex("0000008000000080"):
        raise ValueError("map.c must contain an 8-byte 128x128 header and 16384 chip IDs")
    patterns = metadata["patterns"]
    if any(len(pattern) != 16 for pattern in patterns):
        raise ValueError("Every chip must have a 4x4 source material pattern")
    road = bytearray(WIDTH * WIDTH)
    for index, tile in enumerate(raw[8:]):
        if tile >= len(patterns):
            raise ValueError(f"Unknown chip ID {tile} at map cell {index}")
        for pixel, material in enumerate(patterns[tile]):
            x = index % 128 * 4 + pixel % 4
            y = index // 128 * 4 + pixel // 4
            road[y * WIDTH + x] = material in (1, 2, 3, 4)
    return road


def inner_island(road):
    """The largest enclosed non-road component is the inside of this circuit."""
    seen = bytearray(len(road))
    largest = []
    for start in range(len(road)):
        if road[start] or seen[start]:
            continue
        queue = deque([start])
        seen[start] = 1
        component = []
        touches_edge = False
        while queue:
            index = queue.popleft()
            component.append(index)
            x, y = index % WIDTH, index // WIDTH
            touches_edge |= x in (0, WIDTH - 1) or y in (0, WIDTH - 1)
            for nx, ny in ((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)):
                if 0 <= nx < WIDTH and 0 <= ny < WIDTH:
                    neighbor = ny * WIDTH + nx
                    if not seen[neighbor] and not road[neighbor]:
                        seen[neighbor] = 1
                        queue.append(neighbor)
        if not touches_edge and len(component) > len(largest):
            largest = component
    if not largest:
        raise ValueError("No enclosed course island found")
    return set(largest)


def lane_boundary(island):
    expanded = set(island)
    offsets = [(dx, dy) for dy in range(-LANE_RADIUS, LANE_RADIUS + 1)
               for dx in range(-LANE_RADIUS, LANE_RADIUS + 1)
               if dx * dx + dy * dy <= LANE_RADIUS * LANE_RADIUS]
    for index in island:
        if all(i in island for i in (index - 1, index + 1, index - WIDTH, index + WIDTH)):
            continue
        for dx, dy in offsets:
            x, y = index % WIDTH + dx, index // WIDTH + dy
            if not (0 < x < WIDTH - 1 and 0 < y < WIDTH - 1):
                raise ValueError("Offset lane reaches the map boundary")
            expanded.add(y * WIDTH + x)

    # Directed edges avoid diagonal pixel-connectivity ambiguity.
    edges = {}
    for index in sorted(expanded):
        x, y = index % WIDTH, index // WIDTH
        candidates = []
        if index - WIDTH not in expanded:
            candidates.append(((x, y), (x + 1, y)))
        if index + 1 not in expanded:
            candidates.append(((x + 1, y), (x + 1, y + 1)))
        if index + WIDTH not in expanded:
            candidates.append(((x + 1, y + 1), (x, y + 1)))
        if index - 1 not in expanded:
            candidates.append(((x, y + 1), (x, y)))
        for start, end in candidates:
            if start in edges:
                raise ValueError("Ambiguous lane boundary; remove diagonal-only connections")
            edges[start] = end
    loops = []
    while edges:
        first = next(iter(edges))
        point, loop = first, []
        while point in edges:
            loop.append(point)
            point = edges.pop(point)
        if point != first:
            raise ValueError("Lane boundary is not closed")
        loops.append(loop)
    # Reversal puts the enclosed island on the driver's left.
    return max(loops, key=len)[::-1]


def sample_lane(boundary, road):
    # Smooth source-pixel stair steps, then sample by arc length rather than index.
    path = [tuple(sum(boundary[(i + j) % len(boundary)][axis] for j in range(-4, 5))
                  / 9 * WORLD_SCALE for axis in range(2)) for i in range(len(boundary))]
    lengths = [math.dist(point, path[(i + 1) % len(path)]) for i, point in enumerate(path)]
    total = sum(lengths)
    count = round(total / 8)
    if count < 2:
        raise ValueError("Lane is too short")
    samples = []
    segment, offset = 0, 0
    for i in range(count):
        distance = i * total / count
        while offset + lengths[segment] < distance:
            offset += lengths[segment]
            segment += 1
        a, b = path[segment], path[(segment + 1) % len(path)]
        ratio = (distance - offset) / lengths[segment]
        point = tuple(round((a[k] + (b[k] - a[k]) * ratio) * 256) for k in range(2))
        if not road[(point[1] // 1024) * WIDTH + point[0] // 1024]:
            raise ValueError("Smoothed lane leaves the road")
        samples.append(point)
    return samples


def main():
    road = read_road()
    samples = sample_lane(lane_boundary(inner_island(road)), road)
    lines = ["/* Generated by generate-route.py from map.c and mapchip-patterns.json. */",
             "/* Q8.8 world-pixel centers, counter-clockwise: inner edge stays on the left. */",
             "static const struct RoutePoint { int32_t x, y, length; } route[] = {"]
    for i, (x, y) in enumerate(samples):
        nx, ny = samples[(i + 1) % len(samples)]
        length = round(math.hypot(nx - x, ny - y))
        if length <= 0:
            raise ValueError("Lane contains a zero-length segment")
        lines.append(f"    {{{x}, {y}, {length}}},")
    lines.append("};")
    (ROOT / "route.h").write_text("\n".join(lines) + "\n")
    print(f"Generated {len(samples)} lane points; inner edge stays on the left")


if __name__ == "__main__":
    main()
