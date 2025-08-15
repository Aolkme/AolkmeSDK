#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>

/* ================= 配置区 ================= */
#define AOK_KEY_MAX_LEN      32   // outKey 缓冲区建议大小
#define AOK_KEY_PREFIX       "AK1"// 版本+产品前缀，改版本时一起变更
#define AOK_KEY_PREFIX_LEN   3




// 私有盐（放在只读段、不要泄露；可用编译期脚本混淆/分段存放）
static const uint8_t SECRET1[] = {0xB3,0x71,0xC2,0x5E,0x19,0xA4,0x7D,0x21};
static const uint8_t SECRET2[] = {0x5A,0x9C,0x03,0xE7,0x44,0xD2,0x18,0xBF};
static const uint8_t SECRET3[] = {0x6D,0x11,0x8A,0x2F,0x90,0xCE,0x33,0x74};
static const uint8_t SECRET4[] = {0x0F,0x57,0xAA,0x61,0x2B,0x9E,0xD8,0x46};
/* ========================================= */

/* -------- FNV-1a 64-bit（轻量） -------- */
static uint64_t fnv1a64_update(uint64_t h, const uint8_t *p, size_t n){
    const uint64_t FNV_PRIME = 0x100000001b3ULL;
    for(size_t i=0;i<n;i++){
        h ^= (uint64_t)p[i];
        h *= FNV_PRIME;
    }
    return h;
}
static uint64_t fnv1a64_span(const uint8_t *p, size_t n){
    const uint64_t FNV_OFFSET = 0xcbf29ce484222325ULL;
    return fnv1a64_update(FNV_OFFSET, p, n);
}

/* -------- CRC16-CCITT (0x1021, init 0xFFFF) 用于Key末尾校验 -------- */
static uint16_t crc16_ccitt(const uint8_t *p, size_t n){
    uint16_t crc = 0xFFFF;
    for(size_t i=0;i<n;i++){
        crc ^= (uint16_t)p[i] << 8;
        for(int b=0;b<8;b++){
            if(crc & 0x8000) crc = (crc<<1) ^ 0x1021;
            else             crc = (crc<<1);
        }
    }
    return crc;
}

/* -------- Base32（RFC4648，无填充'='） -------- */
static const char B32TAB[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

static size_t base32_encode(const uint8_t *in, size_t inlen, char *out, size_t outcap){
    // 输出长度 = ceil(inlen*8/5)
    size_t need = (inlen*8 + 4)/5;
    if(outcap < need+1) return 0; // +1 留给 '\0'
    uint32_t buffer = 0; int bits = 0; size_t o = 0;

    for(size_t i=0;i<inlen;i++){
        buffer = (buffer<<8) | in[i];
        bits += 8;
        while(bits >= 5){
            bits -= 5;
            out[o++] = B32TAB[(buffer>>bits) & 0x1F];
        }
    }
    if(bits > 0){
        out[o++] = B32TAB[(buffer<<(5-bits)) & 0x1F];
    }
    out[o] = '\0';
    return o;
}





/* -------- 生成 120bit 指纹：concat(H1[8], H2[7]) -------- */
static void make_fingerprint_120bit(const char *id, uint8_t out15[15]){
    const uint8_t *pid = (const uint8_t*)id;
    size_t idlen = strlen(id);

    // H1 = FNV64( SECRET1 || ID || SECRET2 )
    uint64_t h1 = fnv1a64_span(SECRET1, sizeof(SECRET1));
    h1 = fnv1a64_update(h1, pid, idlen);
    h1 = fnv1a64_update(h1, SECRET2, sizeof(SECRET2));

    // H2 = FNV64( SECRET3 || ID || SECRET4 )
    uint64_t h2 = fnv1a64_span(SECRET3, sizeof(SECRET3));
    h2 = fnv1a64_update(h2, pid, idlen);
    h2 = fnv1a64_update(h2, SECRET4, sizeof(SECRET4));

    // out15 = H1(8字节) || H2(低7字节)
    for(int i=0;i<8;i++) out15[i]   = (uint8_t)((h1 >> (56 - 8*i)) & 0xFF);
    for(int i=0;i<7;i++) out15[8+i] = (uint8_t)((h2 >> (56 - 8*i)) & 0xFF); // 取高位到低位，最后丢弃最低的8bit
}






/* -------- 核心：生成KEY -------- */
bool Aolkme_GenerateKey(char* id, char* outKey, size_t outSize){
    if(!id || !outKey || outSize < 8) return false; // 基本健壮性
    // 目标格式： "AK1" + base32(fingerprint_15B)=24字 + checksum_2字
    // 计算 base32 长度：ceil(15*8/5)=24
    uint8_t fp[15];
    make_fingerprint_120bit(id, fp);

    // 计算校验（对 ID 计算 CRC16），取低10bit → 2个Base32字符
    uint16_t crc = crc16_ccitt((const uint8_t*)id, strlen(id));
    uint16_t chk10 = crc & 0x03FF; // 10 bits

    char b32[24+1];
    if(base32_encode(fp, sizeof(fp), b32, sizeof(b32)) != 24) return false;

    // 拼接 KEY
    // 总长 = 3 + 24 + 2 = 29（不含 '\0'）
    if(outSize < 30) return false; // 至少 30（含 '\0'）
    char *p = outKey;

    // 前缀
    memcpy(p, AOK_KEY_PREFIX, AOK_KEY_PREFIX_LEN); p += AOK_KEY_PREFIX_LEN;
    // 指纹Base32
    memcpy(p, b32, 24); p += 24;
    // 校验 2 字符（高5位、低5位）
    p[0] = B32TAB[(chk10 >> 5) & 0x1F];
    p[1] = B32TAB[ chk10       & 0x1F];
    p += 2;
    *p = '\0';
    return true;
}

/* -------- 核心：校验KEY -------- */
static int b32val(char c){
    if(c>='A' && c<='Z') return c - 'A';
    if(c>='2' && c<='7') return 26 + (c-'2');
    return -1;
}










bool Aolkme_VerifyKey(char* id, char* key){
    if(!id || !key) return false;
    size_t klen = strlen(key);
    // 长度必须为 29 且前缀正确
    if(klen != 29) return false;
    if(memcmp(key, AOK_KEY_PREFIX, AOK_KEY_PREFIX_LEN)!=0) return false;

    // 解析校验 2 字符 → 10bit
    int v1 = b32val(key[27]);
    int v2 = b32val(key[28]);
    if(v1 < 0 || v2 < 0) return false;
    uint16_t chk10_in = (uint16_t)(((v1 & 0x1F) << 5) | (v2 & 0x1F));

    // 重新生成期望 KEY，并比对全串（最稳妥）
    char expect[AOK_KEY_MAX_LEN];
    if(!Aolkme_GenerateKey(id, expect, sizeof(expect))) return false;

    // 快速先比对校验段（可选，提早失败）
    uint16_t crc = crc16_ccitt((const uint8_t*)id, strlen(id));
    uint16_t chk10 = crc & 0x03FF;
    if(chk10 != chk10_in) return false;

    // 最终比对
    return (strcmp(expect, key) == 0);
}










