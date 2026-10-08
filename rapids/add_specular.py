#!/usr/bin/env python3
"""Add a low-specular KHR_materials_specular value to every GLB material."""
import json
import struct
import sys
from pathlib import Path

EXTENSION = "KHR_materials_specular"
SPECULAR_FACTOR = 0.05
JSON_CHUNK = b"JSON"


def patch_glb(path: Path) -> None:
    data = path.read_bytes()
    magic, version, total_length = struct.unpack_from("<4sII", data)
    if magic != b"glTF" or version != 2 or total_length != len(data):
        raise ValueError(f"{path} is not a valid GLB 2.0 file")

    chunks = []
    offset = 12
    while offset < total_length:
        chunk_length, chunk_type = struct.unpack_from("<I4s", data, offset)
        start = offset + 8
        end = start + chunk_length
        if end > total_length:
            raise ValueError(f"truncated GLB chunk in {path}")
        chunks.append((chunk_type, data[start:end]))
        offset = end
    if offset != total_length:
        raise ValueError(f"invalid GLB chunk alignment in {path}")

    json_indices = [i for i, (kind, _) in enumerate(chunks) if kind == JSON_CHUNK]
    if len(json_indices) != 1:
        raise ValueError(f"expected one JSON chunk in {path}")

    index = json_indices[0]
    document = json.loads(chunks[index][1].decode("utf-8").rstrip(" \t\r\n\0"))
    used = document.setdefault("extensionsUsed", [])
    if EXTENSION not in used:
        used.append(EXTENSION)

    materials = document.setdefault("materials", [])
    primitives = [
        primitive
        for mesh in document.get("meshes", [])
        for primitive in mesh.get("primitives", [])
    ]
    if any("material" not in primitive for primitive in primitives):
        default_index = len(materials)
        materials.append({
            "name": "Default",
            "pbrMetallicRoughness": {
                "baseColorFactor": [1, 1, 1, 1],
                "metallicFactor": 1,
                "roughnessFactor": 1,
            },
        })
        for primitive in primitives:
            primitive.setdefault("material", default_index)

    for material in materials:
        extension = material.setdefault("extensions", {}).setdefault(EXTENSION, {})
        extension["specularFactor"] = SPECULAR_FACTOR
        extension["glossinessFactor"] = 0.9

    json_bytes = json.dumps(document, separators=(",", ":"), ensure_ascii=False).encode("utf-8")
    json_bytes += b" " * (-len(json_bytes) % 4)
    chunks[index] = (JSON_CHUNK, json_bytes)

    body = b"".join(
        struct.pack("<I4s", len(payload), kind) + payload
        for kind, payload in chunks
    )
    path.write_bytes(struct.pack("<4sII", b"glTF", 2, 12 + len(body)) + body)


if len(sys.argv) != 2:
    raise SystemExit(f"usage: {Path(sys.argv[0]).name} FILE.glb")

patch_glb(Path(sys.argv[1]))
