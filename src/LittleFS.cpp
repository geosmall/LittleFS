/* LittleFS for Teensy
 * Copyright (c) 2020, Paul Stoffregen, paul@pjrc.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <Arduino.h>
#include <LittleFS.h>

// #define SPICONFIG   SPISettings(30000000, MSBFIRST, SPI_MODE0)
#define SPICONFIG SPISettings(1000000, MSBFIRST, SPI_MODE0)


PROGMEM static const struct chipinfo {
    uint8_t id[3];
    uint8_t addrbits;   // number of address bits, 24 or 32
    uint16_t progsize;  // page size for programming, in bytes
    uint32_t erasesize; // sector size for erasing, in bytes
    uint8_t  erasecmd;  // command to use for sector erase
    uint32_t chipsize;  // total number of bytes in the chip
    uint32_t progtime;  // maximum microseconds to wait for page programming
    uint32_t erasetime; // maximum microseconds to wait for sector erase
    const char pn[22];      //flash name
} known_chips[] = {
    {{0xEF, 0x40, 0x15}, 24, 256, 32768, 0x52, 2097152, 3000, 1600000, "W25Q16JV-Q"},  // Winbond W25Q16JV*Q/W25Q16FV
    {{0xEF, 0x40, 0x16}, 24, 256, 32768, 0x52, 4194304, 3000, 1600000, "W25Q32JV-Q"},  // Winbond W25Q32JV*Q/W25Q32FV
    {{0xEF, 0x40, 0x17}, 24, 256, 65536, 0xD8, 8388608, 3000, 2000000, "W25Q64JV-Q"},  // Winbond W25Q64JV*Q/W25Q64FV
    {{0xEF, 0x40, 0x18}, 24, 256, 65536, 0xD8, 16777216, 3000, 2000000, "W25Q128JV-Q"}, // Winbond W25Q128JV*Q/W25Q128FV
    {{0xEF, 0x40, 0x19}, 32, 256, 65536, 0xDC, 33554432, 3000, 2000000, "W25Q256JV-Q"}, // Winbond W25Q256JV*Q
    {{0xEF, 0x40, 0x20}, 32, 256, 65536, 0xDC, 67108864, 3500, 2000000, "W25Q512JV-Q"}, // Winbond W25Q512JV*Q
    {{0xEF, 0x40, 0x21}, 32, 256, 65536, 0xDC, 134217728, 3500, 2000000, "W25Q01JV-Q"},// Winbond W25Q01JV*Q
    {{0x62, 0x06, 0x13}, 24, 256,  4096, 0x20, 524288, 5000, 300000, "SST25PF040C"},  // Microchip SST25PF040C
//{{0xEF, 0x40, 0x14}, 24, 256,  4096, 0x20, 1048576, 5000, 300000, "W25Q80DV"},  // Winbond W25Q80DV  not tested
    {{0xEF, 0x70, 0x17}, 24, 256, 65536, 0xD8, 8388608, 3000, 2000000, "W25Q64JV-M"},  // Winbond W25Q64JV*M (DTR)
    {{0xEF, 0x70, 0x18}, 24, 256, 65536, 0xD8, 16777216, 3000, 2000000, "W25Q128JV-M"}, // Winbond W25Q128JV*M (DTR)
    {{0xEF, 0x70, 0x19}, 32, 256, 65536, 0xDC, 33554432, 3000, 2000000, "W25Q256JV-M"}, // Winbond W25Q256JV*M (DTR)
    {{0xEF, 0x80, 0x19}, 32, 256, 65536, 0xDC, 33554432, 3000, 2000000, "W25Q256JW-M"}, // Winbond (W25Q256JW*M)
    {{0xEF, 0x70, 0x20}, 32, 256, 65536, 0xDC, 67108864, 3500, 2000000, "W25Q512JV-M"}, // Winbond W25Q512JV*M (DTR)
    {{0x1F, 0x84, 0x01}, 24, 256,  4096, 0x20, 524288, 2500, 300000, "AT25SF041"},    // Adesto/Atmel AT25SF041
    {{0x01, 0x40, 0x14}, 24, 256,  4096, 0x20, 1048576, 5000, 300000, "S25FL208K"},   // Spansion S25FL208K
    {{0xC8, 0x40, 0x13}, 24, 256,  4096, 0x20,  524288, 2400, 300000, "GD25Q40C"},   // GigaDevice GD25Q40C
    {{0xC8, 0x40, 0x14}, 24, 256,  4096, 0x20, 1048576, 2400, 300000, "GD25Q80C"},   // GigaDevice GD25Q80C
    {{0xC8, 0x40, 0x15}, 24, 256, 32768, 0x52, 2097152, 4000, 1600000, "GD25Q16E"},  // GigaDevice GD25Q16E
    {{0xC8, 0x40, 0x16}, 24, 256, 32768, 0x52, 4194304, 4000, 1600000, "GD25Q32E"},  // GigaDevice GD25Q32E
    {{0xC8, 0x40, 0x17}, 24, 256, 65536, 0xD8, 8388608, 4000, 3000000, "GD25Q64E"},  // GigaDevice GD25Q64E
    {{0xC8, 0x40, 0x18}, 24, 256, 65536, 0xD8, 16777216, 4000, 3000000, "GD25Q128E"},  // GigaDevice GD25Q128E
    {{0xC8, 0x40, 0x19}, 32, 256, 65536, 0xDC, 33554432, 2000, 1600000, "GD25Q256E"},  // GigaDevice GD25Q256E

};

static const struct chipinfo *chip_lookup(const uint8_t *id)
{
    const unsigned int numchips = sizeof(known_chips) / sizeof(struct chipinfo);
    for (unsigned int i = 0; i < numchips; i++) {
        const uint8_t *chip = known_chips[i].id;
        if (id[0] == chip[0] && id[1] == chip[1] && id[2] == chip[2]) {
            return known_chips + i;
        }
    }
    return nullptr;
}

FLASHMEM
bool LittleFS_SPIFlash::begin(uint8_t cspin, SPIClass &spiport)
{
    pin = cspin;
    port = &spiport;

    //Serial.println("flash begin");
    configured = false;
    digitalWrite(pin, HIGH);
    pinMode(pin, OUTPUT);
    port->begin();

    uint8_t buf[4] = {0x9F, 0, 0, 0};
    port->beginTransaction(SPICONFIG);
    digitalWrite(pin, LOW);
    port->transfer(buf, 4);
    digitalWrite(pin, HIGH);
    port->endTransaction();

    Serial.printf("Flash ID: %02X %02X %02X  %02X\n", buf[1], buf[2], buf[3], buf[4]);
    const struct chipinfo *info = chip_lookup(buf + 1);
    if (!info) return false;
    hwinfo = info;
    Serial.printf("Flash size is %.2f Mbyte\n", (float)info->chipsize / 1048576.0f);

    memset(&lfs, 0, sizeof(lfs));
    memset(&config, 0, sizeof(config));
    config.context = (void *)this;
    config.read = &static_read;
    config.prog = &static_prog;
    config.erase = &static_erase;
    config.sync = &static_sync;
    config.read_size = info->progsize;
    config.prog_size = info->progsize;
    config.block_size = info->erasesize;
    config.block_count = info->chipsize / info->erasesize;
    config.block_cycles = 400;
    config.cache_size = info->progsize;
    config.lookahead_size = info->progsize;
    // config.lookahead_size = config.block_count/8;
    config.name_max = LFS_NAME_MAX;
    configured = true;

    Serial.println("attempting to mount existing media");
    if (lfs_mount(&lfs, &config) < 0) {
        Serial.println("couldn't mount media, attemping to format");
        if (lfs_format(&lfs, &config) < 0) {
            Serial.println("format failed :(");
            port = nullptr;
            return false;
        }
        Serial.println("attempting to mount freshly formatted media");
        if (lfs_mount(&lfs, &config) < 0) {
            Serial.println("mount after format failed :(");
            port = nullptr;
            return false;
        }
    }
    mounted = true;
    Serial.println("success");
    return true;
}

FLASHMEM
const char *LittleFS_SPIFlash::getMediaName()
{
    if (!hwinfo) return nullptr;
    return ((const struct chipinfo *)hwinfo)->pn;
}

FLASHMEM
bool LittleFS::quickFormat()
{
    if (!configured) return false;
    if (mounted) {
        //Serial.println("unmounting filesystem");
        lfs_unmount(&lfs);
        mounted = false;
        // TODO: What happens if lingering LittleFSFile instances
        // still have lfs_file_t structs allocated which reference
        // this previously mounted filesystem?
    }
    //Serial.println("attempting to format existing media");
    if (lfs_format(&lfs, &config) < 0) {
        //Serial.println("format failed :(");
        return false;
    }
    //Serial.println("attempting to mount freshly formatted media");
    if (lfs_mount(&lfs, &config) < 0) {
        //Serial.println("mount after format failed :(");
        return false;
    }
    mounted = true;
    //Serial.println("success");
    return true;
}

static bool blockIsBlank(struct lfs_config *config, lfs_block_t block, void *readBuf, bool full = true);
static bool blockIsBlank(struct lfs_config *config, lfs_block_t block, void *readBuf, bool full)
{
    if (!readBuf) return false;
    for (lfs_off_t offset = 0; offset < config->block_size; offset += config->read_size) {
        memset(readBuf, 0, config->read_size);
        config->read(config, block, offset, readBuf, config->read_size);
        const uint8_t *buf = (uint8_t *)readBuf;
        for (unsigned int i = 0; i < config->read_size; i++) {
            if (buf[i] != 0xFF) return false;
        }
        if (!full)
            return true; // first bytes read as 0xFF
    }
    return true; // all bytes read as 0xFF
}

static int cb_usedBlocks(void *inData, lfs_block_t block)
{
    static lfs_block_t maxBlock;
    static uint32_t totBlock;
    if (nullptr == inData) {   // not null during traverse
        uint32_t totRet = totBlock;
        if (0 != block) {
            maxBlock = block;
            totBlock = 0;
        }
        return totRet; // exit after init, end, or bad call
    }
    totBlock++;
    if (block > maxBlock) return block;   // this is beyond media blocks
    uint32_t iiblk = block / 8;
    uint8_t jjbit = 1 << (block % 8);
    uint8_t *myData = (uint8_t *)inData;
    myData[iiblk] = myData[iiblk] | jjbit;
    return 0;
}

FLASHMEM
uint32_t LittleFS::formatUnused(uint32_t blockCnt, uint32_t blockStart)
{
    if (!configured) return 0;
    uint32_t iiblk = 1 + (config.block_count / 8);
    uint8_t *checkused = (uint8_t *)malloc(iiblk);
    if (checkused == nullptr) return 0;
    void *buffer = malloc(config.read_size);
    if (buffer == nullptr) {
        free(checkused);
        return 0;
    }
    memset(checkused, 0, iiblk);
    cb_usedBlocks(nullptr, config.block_count);   // init and pass MAX block_count
    int err = lfs_fs_traverse(&lfs, cb_usedBlocks, checkused); // on return 1 bits are used blocks

    if (err < 0) {
        free(checkused);
        free(buffer);
        return 0;
    }
    uint32_t block = blockStart, jj = 0;
    if (block >= config.block_count) blockStart = 0;
    if (0 == blockCnt) blockCnt = config.block_count;
    while (block < config.block_count && jj < blockCnt) {
        iiblk = block / 8;
        uint8_t jjbit = 1 << (block % 8);
        if (!(checkused[iiblk] & jjbit)) {   // block not in use
            if (!blockIsBlank(&config, block, buffer, false)) {
                (*config.erase)(&config, block);
                jj++;
            }
        }
        block++;
    }
    free(checkused); // This discards LFS_(check)used list. If each Format by LFS were known we could add to a static copy.
    // Traverse takes 2 to 20ms on each entry - some images and media may take longer
    // TODO?: if kept and updated the 'free dirty blocks' could be ignored and traverse skipped until all prior dirty were formatted
    free(buffer);
    if (block >= config.block_count) block = 0;
    return block; // return lastChecked block to store to start next pass as blockStart
}

FLASHMEM
bool LittleFS::lowLevelFormat(char progressChar, Print *pr)
{
    if (!configured) return false;
    if (mounted) {
        lfs_unmount(&lfs);
        mounted = false;
    }
    int ii = config.block_count / 120;
    void *buffer = malloc(config.read_size);
    for (unsigned int block = 0; block < config.block_count; block++) {
        if (pr && progressChar && (0 == block % ii)) pr->write(progressChar);
        if (!blockIsBlank(&config, block, buffer)) {
            (*config.erase)(&config, block);
        }
    }
    free(buffer);
    if (pr && progressChar) pr->println();
    return quickFormat();
}

static void make_command_and_address(uint8_t *buf, uint8_t cmd, uint32_t addr, uint8_t addrbits)
{
    buf[0] = cmd;
    if (addrbits == 24) {
        buf[1] = addr >> 16;
        buf[2] = addr >> 8;
        buf[3] = addr;
    } else {
        buf[1] = addr >> 24;
        buf[2] = addr >> 16;
        buf[3] = addr >> 8;
        buf[4] = addr;
    }
}
static void printtbuf(const void *buf, unsigned int len) __attribute__((unused));
static void printtbuf(const void *buf, unsigned int len)
{
    //const uint8_t *p = (const uint8_t *)buf;
    //Serial.print("    ");
    //while (len--) Serial.printf("%02X ", *p++);
    //Serial.println();
}

int LittleFS_SPIFlash::read(lfs_block_t block, lfs_off_t offset, void *buf, lfs_size_t size)
{
    if (!port) return LFS_ERR_IO;
    const uint32_t addr = block * config.block_size + offset;
    const uint8_t addrbits = ((const struct chipinfo *)hwinfo)->addrbits;
    const uint8_t cmd = (addrbits == 24) ? 0x03 : 0x13; // standard read command
    uint8_t cmdaddr[5];
    //Serial.printf("  addrbits=%d\n", addrbits);
    make_command_and_address(cmdaddr, cmd, addr, addrbits);
    //printtbuf(cmdaddr, 1 + (addrbits >> 3));
    memset(buf, 0, size);
    port->beginTransaction(SPICONFIG);
    digitalWrite(pin, LOW);
    port->transfer(cmdaddr, 1 + (addrbits >> 3));
    port->transfer(buf, size);
    digitalWrite(pin, HIGH);
    port->endTransaction();
    //printtbuf(buf, 20);
    return 0;
}

int LittleFS_SPIFlash::prog(lfs_block_t block, lfs_off_t offset, const void *buf, lfs_size_t size)
{
    if (!port) return LFS_ERR_IO;
    const uint32_t addr = block * config.block_size + offset;
    const uint8_t addrbits = ((const struct chipinfo *)hwinfo)->addrbits;
    const uint8_t cmd = (addrbits == 24) ? 0x02 : 0x12; // page program
    uint8_t cmdaddr[5];
    make_command_and_address(cmdaddr, cmd, addr, addrbits);
    //printtbuf(cmdaddr, 1 + (addrbits >> 3));
    port->beginTransaction(SPICONFIG);
    digitalWrite(pin, LOW);
    port->transfer(0x06); // 0x06 = write enable
    digitalWrite(pin, HIGH);
    // delayNanoseconds(250);
    delayMicroseconds(1);
    digitalWrite(pin, LOW);
    port->transfer(cmdaddr, 1 + (addrbits >> 3));
    port->transfer(buf, nullptr, size);
    digitalWrite(pin, HIGH);
    port->endTransaction();
    //printtbuf(buf, 20);
    const uint32_t progtime = ((const struct chipinfo *)hwinfo)->progtime;
    return wait(progtime);
}

int LittleFS_SPIFlash::erase(lfs_block_t block)
{
    if (!port) return LFS_ERR_IO;
    void *buffer = malloc(config.read_size);
    if (buffer != nullptr) {
        if (blockIsBlank(&config, block, buffer)) {
            free(buffer);
            return 0; // Already formatted exit no wait
        }
        free(buffer);
    }
    const uint32_t addr = block * config.block_size;
    uint8_t cmdaddr[5];
    const uint8_t erasecmd = ((const struct chipinfo *)hwinfo)->erasecmd;
    const uint8_t addrbits = ((const struct chipinfo *)hwinfo)->addrbits;
    make_command_and_address(cmdaddr, erasecmd, addr, addrbits);
    //printtbuf(cmdaddr, 1 + (addrbits >> 3));
    port->beginTransaction(SPICONFIG);
    digitalWrite(pin, LOW);
    port->transfer(0x06); // 0x06 = write enable
    digitalWrite(pin, HIGH);
    // delayNanoseconds(250);
    delayMicroseconds(1);
    digitalWrite(pin, LOW);
    port->transfer(cmdaddr, 1 + (addrbits >> 3));
    digitalWrite(pin, HIGH);
    port->endTransaction();
    const uint32_t erasetime = ((const struct chipinfo *)hwinfo)->erasetime;
    return wait(erasetime);
}

int LittleFS_SPIFlash::wait(uint32_t microseconds)
{
    elapsedMicros usec = 0;
    while (1) {
        port->beginTransaction(SPICONFIG);
        digitalWrite(pin, LOW);
        uint16_t status = port->transfer16(0x0500); // 0x05 = get status
        digitalWrite(pin, HIGH);
        port->endTransaction();
        if (!(status & 1)) break;
        if (usec > microseconds) return LFS_ERR_IO; // timeout
        yield();
    }
    //Serial.printf("  waited %u us\n", (unsigned int)usec);
    return 0; // success
}

//-----------------------------------------------------------------------------
// Wrapper classes begin methods
//-----------------------------------------------------------------------------
bool LittleFS_SPI::begin(uint8_t cspin, SPIClass &spiport)
{
    if (cspin != 0xff) csPin_ = cspin;
    if (flash.begin(csPin_, spiport)) {
        sprintf(display_name, (const char *)F("Flash_%u"), csPin_);
        pfs = &flash;
        return true;
    }
    // none of the above.
    pfs = &fsnone;
    return false;
}
