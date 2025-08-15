import struct

# ================= 配置区 =================
AOK_KEY_PREFIX = "AK1"  # 版本/产品前缀
SECRET1 = bytes([0xB3,0x71,0xC2,0x5E,0x19,0xA4,0x7D,0x21])
SECRET2 = bytes([0x5A,0x9C,0x03,0xE7,0x44,0xD2,0x18,0xBF])
SECRET3 = bytes([0x6D,0x11,0x8A,0x2F,0x90,0xCE,0x33,0x74])
SECRET4 = bytes([0x0F,0x57,0xAA,0x61,0x2B,0x9E,0xD8,0x46])
B32TAB = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567"
# ==========================================


def fnv1a64(data: bytes) -> int:
    """FNV-1a 64位哈希"""
    h = 0xcbf29ce484222325
    for b in data:
        h ^= b
        h = (h * 0x100000001b3) & 0xFFFFFFFFFFFFFFFF
    return h


def crc16_ccitt(data: bytes) -> int:
    """CRC16-CCITT (0x1021, init=0xFFFF)"""
    crc = 0xFFFF
    for b in data:
        crc ^= b << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc


def base32_encode(data: bytes) -> str:
    """无填充 Base32"""
    buffer = 0
    bits = 0
    out = []
    for b in data:
        buffer = (buffer << 8) | b
        bits += 8
        while bits >= 5:
            bits -= 5
            out.append(B32TAB[(buffer >> bits) & 0x1F])
    if bits > 0:
        out.append(B32TAB[(buffer << (5 - bits)) & 0x1F])
    return "".join(out)


def make_fingerprint_120bit(app_id: str) -> bytes:
    """生成 120bit 指纹（15字节）"""
    id_bytes = app_id.encode("utf-8")

    h1 = fnv1a64(SECRET1 + id_bytes + SECRET2)
    h2 = fnv1a64(SECRET3 + id_bytes + SECRET4)

    # 8字节 h1 + h2 高 7 字节
    return struct.pack(">Q", h1) + struct.pack(">Q", h2)[:7]


def generate_key(app_id: str) -> str:
    """根据 ID 生成 KEY"""
    fp = make_fingerprint_120bit(app_id)
    b32_fp = base32_encode(fp)  # 长度应为24

    # 校验段（10bit）
    crc = crc16_ccitt(app_id.encode("utf-8"))
    chk10 = crc & 0x03FF
    chk_chars = B32TAB[(chk10 >> 5) & 0x1F] + B32TAB[chk10 & 0x1F]

    return f"{AOK_KEY_PREFIX}{b32_fp}{chk_chars}"


def verify_key(app_id: str, key: str) -> bool:
    """校验 KEY 是否匹配"""
    return key == generate_key(app_id)


# ================= 使用示例 =================
if __name__ == "__main__":
    id1 = "AolkmeSDKadmin"
    key1 = generate_key(id1)
    print("ID :", id1)
    print("KEY:", key1)
    print("Verify:", verify_key(id1, key1))

    id2 = "A123456789BCDEF"
    key2 = generate_key(id2)
    print("ID :", id2)
    print("KEY:", key2)
    print("Verify:", verify_key(id2, key2))
