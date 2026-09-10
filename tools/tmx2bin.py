#!/usr/bin/env python3
import argparse
import re
import struct
import sys
import xml.etree.ElementTree as ET
from pathlib import Path


TMX_FLIP_FLAGS = 0xF0000000


class ConversionError(Exception):
    pass


def strip_namespace(tag):
    return tag.rsplit("}", 1)[-1]


def parse_uint_attr(element, name, context):
    value = element.get(name)
    if value is None:
        raise ConversionError(f"{context} is missing '{name}'")
    try:
        parsed = int(value, 10)
    except ValueError as exc:
        raise ConversionError(f"{context} has invalid '{name}': {value}") from exc
    if parsed < 0:
        raise ConversionError(f"{context} has negative '{name}': {value}")
    return parsed


def iter_tile_layers(root):
    return [element for element in root.iter() if strip_namespace(element.tag) == "layer"]


def get_child(element, child_name):
    for child in element:
        if strip_namespace(child.tag) == child_name:
            return child
    return None


def parse_csv_data(data_element):
    text = data_element.text or ""
    if not text.strip():
        return []
    return [int(token, 10) for token in re.split(r"[\s,]+", text.strip()) if token]


def parse_xml_tile_data(data_element):
    gids = []
    for child in data_element:
        if strip_namespace(child.tag) != "tile":
            continue
        gid = child.get("gid")
        if gid is None:
            raise ConversionError("<tile> is missing 'gid'")
        gids.append(int(gid, 10))
    return gids


def read_layer_gids(layer):
    data_element = get_child(layer, "data")
    if data_element is None:
        raise ConversionError(f"layer '{layer.get('name', '')}' is missing <data>")

    encoding = data_element.get("encoding")
    compression = data_element.get("compression")
    if compression:
        raise ConversionError(f"compressed TMX layer data is not supported: {compression}")

    try:
        if encoding == "csv":
            return parse_csv_data(data_element)
        if encoding is None:
            return parse_xml_tile_data(data_element)
    except ValueError as exc:
        raise ConversionError("layer data contains a non-integer GID") from exc

    raise ConversionError(f"unsupported TMX layer encoding: {encoding}")


def convert_gids_to_bytes(gids):
    output = bytearray()
    for index, gid in enumerate(gids):
        if gid & TMX_FLIP_FLAGS:
            raise ConversionError(f"flipped/rotated tile GID is not supported at index {index}: {gid}")

        if gid <= 0:
            raise ConversionError(f"empty or invalid tile GID at index {index}: {gid}")

        chip = gid - 1
        if chip > 0xFF:
            raise ConversionError(f"tile GID does not fit uint8 after zero-base conversion at index {index}: {gid}")
        output.append(chip)
    return bytes(output)


def read_tmx_layer(tmx_path, layer_index):
    try:
        tree = ET.parse(tmx_path)
    except ET.ParseError as exc:
        raise ConversionError(f"failed to parse TMX XML: {exc}") from exc
    except OSError as exc:
        raise ConversionError(f"failed to read TMX file: {exc}") from exc

    root = tree.getroot()
    if strip_namespace(root.tag) != "map":
        raise ConversionError("TMX root element must be <map>")

    if root.get("infinite") == "1":
        raise ConversionError("infinite TMX maps are not supported")

    map_width = parse_uint_attr(root, "width", "<map>")
    map_height = parse_uint_attr(root, "height", "<map>")

    layers = iter_tile_layers(root)
    if not layers:
        raise ConversionError("TMX does not contain any tile layers")
    if layer_index < 0 or layer_index >= len(layers):
        raise ConversionError(f"layer index out of range: {layer_index} (layer count: {len(layers)})")

    layer = layers[layer_index]
    width = int(layer.get("width", map_width))
    height = int(layer.get("height", map_height))
    if width <= 0 or height <= 0:
        raise ConversionError(f"layer has invalid size: {width}x{height}")

    gids = read_layer_gids(layer)
    expected_tiles = width * height
    if len(gids) != expected_tiles:
        raise ConversionError(f"layer tile count mismatch: got {len(gids)}, expected {expected_tiles}")

    return width, height, convert_gids_to_bytes(gids)


def resolve_paths(args, parser):
    if len(args.paths) > 2:
        parser.error("too many path arguments")

    if args.output and len(args.paths) == 1:
        first = Path(args.output)
        second = Path(args.paths[0])

        # Accept the documented order:
        #   tmx2bin.py -o input.tmx output.bin
        if first.suffix.lower() == ".tmx" and second.suffix.lower() != ".tmx":
            return first, second

        return second, first

    if args.output and len(args.paths) == 0:
        parser.error("missing input TMX path")

    if args.output and len(args.paths) == 2:
        parser.error("use either '-o OUTPUT INPUT' or 'INPUT OUTPUT', not both")

    if not args.output and len(args.paths) == 2:
        return Path(args.paths[0]), Path(args.paths[1])

    parser.error("missing output BIN path")


def build_arg_parser():
    parser = argparse.ArgumentParser(
        description="Convert a Tiled TMX tile layer to a compact binary tile map.",
        usage="%(prog)s [-h] [-l LAYER] [-o OUTPUT] INPUT.tmx [OUTPUT.bin]",
    )
    parser.add_argument("paths", nargs="+", help="input TMX path and optional output BIN path")
    parser.add_argument("-o", "--output", help="output BIN path")
    parser.add_argument(
        "-l",
        "--layer",
        type=int,
        default=0,
        help="zero-based tile layer index to export (default: 0)",
    )
    return parser


def main(argv=None):
    parser = build_arg_parser()
    args = parser.parse_args(argv)
    tmx_path, bin_path = resolve_paths(args, parser)

    try:
        width, height, chip_data = read_tmx_layer(tmx_path, args.layer)
        with bin_path.open("wb") as output:
            output.write(struct.pack(">II", width, height))
            output.write(chip_data)
    except ConversionError as exc:
        print(f"tmx2bin: {exc}", file=sys.stderr)
        return 1
    except OSError as exc:
        print(f"tmx2bin: failed to write BIN file: {exc}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
