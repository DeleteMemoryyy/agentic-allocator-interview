#!/usr/bin/env python3
"""只使用 Python 标准库解锁经过认证的面试阶段包。"""

from __future__ import annotations

import argparse
import base64
import getpass
import hashlib
import hmac
import json
import secrets
import struct
import sys
from pathlib import Path
from typing import Any

MASK32 = 0xFFFFFFFF
CONSTANTS = (0x61707865, 0x3320646E, 0x79622D32, 0x6B206574)
PACKET_VERSION = 1
KDF_N = 1 << 15
KDF_R = 8
KDF_P = 1


class PacketError(ValueError):
    pass


def _rotate_left(value: int, shift: int) -> int:
    return ((value << shift) & MASK32) | (value >> (32 - shift))


def _quarter_round(state: list[int], a: int, b: int, c: int, d: int) -> None:
    state[a] = (state[a] + state[b]) & MASK32
    state[d] = _rotate_left(state[d] ^ state[a], 16)
    state[c] = (state[c] + state[d]) & MASK32
    state[b] = _rotate_left(state[b] ^ state[c], 12)
    state[a] = (state[a] + state[b]) & MASK32
    state[d] = _rotate_left(state[d] ^ state[a], 8)
    state[c] = (state[c] + state[d]) & MASK32
    state[b] = _rotate_left(state[b] ^ state[c], 7)


def _chacha20_block(key: bytes, counter: int, nonce: bytes) -> bytes:
    if len(key) != 32 or len(nonce) != 12 or not 0 <= counter <= MASK32:
        raise PacketError("ChaCha20 密钥、nonce 或计数器无效")
    state = list(CONSTANTS)
    state.extend(struct.unpack("<8I", key))
    state.append(counter)
    state.extend(struct.unpack("<3I", nonce))
    working = state.copy()
    for _ in range(10):
        _quarter_round(working, 0, 4, 8, 12)
        _quarter_round(working, 1, 5, 9, 13)
        _quarter_round(working, 2, 6, 10, 14)
        _quarter_round(working, 3, 7, 11, 15)
        _quarter_round(working, 0, 5, 10, 15)
        _quarter_round(working, 1, 6, 11, 12)
        _quarter_round(working, 2, 7, 8, 13)
        _quarter_round(working, 3, 4, 9, 14)
    return struct.pack(
        "<16I", *((working[index] + state[index]) & MASK32 for index in range(16))
    )


def _xor_stream(key: bytes, nonce: bytes, data: bytes) -> bytes:
    output = bytearray(len(data))
    for offset in range(0, len(data), 64):
        counter = 1 + offset // 64
        if counter > MASK32:
            raise PacketError("阶段包过大")
        block = _chacha20_block(key, counter, nonce)
        chunk = data[offset : offset + 64]
        output[offset : offset + len(chunk)] = bytes(
            left ^ right for left, right in zip(chunk, block)
        )
    return bytes(output)


def _poly1305_tag(one_time_key: bytes, message: bytes) -> bytes:
    if len(one_time_key) != 32:
        raise PacketError("Poly1305 密钥无效")
    r = int.from_bytes(one_time_key[:16], "little")
    r &= 0x0FFFFFFC0FFFFFFC0FFFFFFC0FFFFFFF
    s = int.from_bytes(one_time_key[16:], "little")
    accumulator = 0
    modulus = (1 << 130) - 5
    for offset in range(0, len(message), 16):
        block = message[offset : offset + 16]
        number = int.from_bytes(block + b"\x01", "little")
        accumulator = ((accumulator + number) * r) % modulus
    return ((accumulator + s) % (1 << 128)).to_bytes(16, "little")


def _pad16(data: bytes) -> bytes:
    return b"\x00" * ((16 - len(data) % 16) % 16)


def _aead_tag(key: bytes, nonce: bytes, aad: bytes, ciphertext: bytes) -> bytes:
    poly_key = _chacha20_block(key, 0, nonce)[:32]
    authenticated = (
        aad
        + _pad16(aad)
        + ciphertext
        + _pad16(ciphertext)
        + struct.pack("<QQ", len(aad), len(ciphertext))
    )
    return _poly1305_tag(poly_key, authenticated)


def _derive_key(passphrase: str, salt: bytes, *, n: int, r: int, p: int) -> bytes:
    if not passphrase:
        raise PacketError("解锁密钥为空")
    try:
        return hashlib.scrypt(
            passphrase.encode("utf-8"),
            salt=salt,
            n=n,
            r=r,
            p=p,
            dklen=32,
            maxmem=128 * 1024 * 1024,
        )
    except (ValueError, TypeError) as error:
        raise PacketError("阶段包使用了不支持的 KDF 参数") from error


def _b64encode(value: bytes) -> str:
    return base64.b64encode(value).decode("ascii")


def _b64decode(value: Any, *, expected_length: int | None = None) -> bytes:
    if not isinstance(value, str):
        raise PacketError("阶段包中的二进制字段不是字符串")
    try:
        decoded = base64.b64decode(value, validate=True)
    except (ValueError, TypeError) as error:
        raise PacketError("阶段包包含无效的 base64 数据") from error
    if expected_length is not None and len(decoded) != expected_length:
        raise PacketError("阶段包中的二进制字段长度无效")
    return decoded


def _header(packet: dict[str, Any]) -> dict[str, Any]:
    return {
        "aead": packet["aead"],
        "kdf": packet["kdf"],
        "name": packet["name"],
        "version": packet["version"],
    }


def _aad(packet: dict[str, Any]) -> bytes:
    return json.dumps(
        _header(packet), sort_keys=True, separators=(",", ":"), ensure_ascii=True
    ).encode("ascii")


def seal_packet(name: str, plaintext: bytes, passphrase: str) -> bytes:
    """构建带版本号的 scrypt + ChaCha20-Poly1305 阶段包。"""

    if not name or len(passphrase) < 20:
        raise PacketError("阶段包名称或高熵解锁密钥无效")
    salt = secrets.token_bytes(16)
    nonce = secrets.token_bytes(12)
    packet: dict[str, Any] = {
        "version": PACKET_VERSION,
        "name": name,
        "kdf": {
            "name": "scrypt",
            "n": KDF_N,
            "r": KDF_R,
            "p": KDF_P,
            "salt": _b64encode(salt),
        },
        "aead": {"name": "chacha20-poly1305", "nonce": _b64encode(nonce)},
    }
    key = _derive_key(passphrase, salt, n=KDF_N, r=KDF_R, p=KDF_P)
    ciphertext = _xor_stream(key, nonce, plaintext)
    packet["ciphertext"] = _b64encode(ciphertext)
    packet["tag"] = _b64encode(_aead_tag(key, nonce, _aad(packet), ciphertext))
    return (json.dumps(packet, indent=2, sort_keys=True) + "\n").encode("utf-8")


def open_packet(serialized: bytes, passphrase: str) -> tuple[str, bytes]:
    try:
        packet = json.loads(serialized)
    except (UnicodeDecodeError, json.JSONDecodeError) as error:
        raise PacketError("阶段包不是有效的 JSON") from error
    if not isinstance(packet, dict):
        raise PacketError("阶段包根节点必须是对象")
    try:
        kdf = packet["kdf"]
        aead = packet["aead"]
        name = packet["name"]
        version = packet["version"]
    except KeyError as error:
        raise PacketError("阶段包头不完整") from error
    if (
        version != PACKET_VERSION
        or not isinstance(name, str)
        or not isinstance(kdf, dict)
        or not isinstance(aead, dict)
        or kdf.get("name") != "scrypt"
        or aead.get("name") != "chacha20-poly1305"
    ):
        raise PacketError("阶段包算法或版本不受支持")
    try:
        n, r, p = kdf["n"], kdf["r"], kdf["p"]
        if (n, r, p) != (KDF_N, KDF_R, KDF_P):
            raise PacketError("阶段包的 KDF 参数不受支持")
        salt = _b64decode(kdf["salt"], expected_length=16)
        nonce = _b64decode(aead["nonce"], expected_length=12)
        ciphertext = _b64decode(packet["ciphertext"])
        supplied_tag = _b64decode(packet["tag"], expected_length=16)
    except (KeyError, TypeError) as error:
        raise PacketError("阶段包字段不完整") from error
    key = _derive_key(passphrase, salt, n=n, r=r, p=p)
    expected_tag = _aead_tag(key, nonce, _aad(packet), ciphertext)
    if not hmac.compare_digest(supplied_tag, expected_tag):
        raise PacketError("解锁密钥错误或阶段包已损坏")
    return name, _xor_stream(key, nonce, ciphertext)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("packet", type=Path, help="加密的 .packet 文件")
    parser.add_argument("--key", help="解锁密钥；省略时将以隐藏回显方式输入")
    parser.add_argument("--output", type=Path, help="把明文写入此文件，而不是标准输出")
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    unlock_key = args.key if args.key is not None else getpass.getpass("解锁密钥：")
    try:
        name, plaintext = open_packet(args.packet.read_bytes(), unlock_key)
    except (OSError, PacketError) as error:
        raise SystemExit(f"无法解锁阶段包：{error}") from error
    if args.output:
        if args.output.exists():
            raise SystemExit(f"拒绝覆盖已有输出文件：{args.output}")
        args.output.write_bytes(plaintext)
        print(f"已解锁 {name}：{args.output}")
    else:
        sys.stdout.buffer.write(plaintext)
        if plaintext and not plaintext.endswith(b"\n"):
            sys.stdout.buffer.write(b"\n")


if __name__ == "__main__":
    main()
