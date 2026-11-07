#!/usr/bin/env python3
"""Extract files from an Xbox 360 STFS (CON/LIVE/PIRS) package.

Block math follows the SDK's StfsContainerDevice (Xenia-derived).
Usage: stfs_extract.py <package> <out_dir>
"""
import os
import struct
import sys

BLOCK = 0x1000
PER_LEVEL = [0xAA, 0x70E4, 0x4AF768]


class Stfs:
    def __init__(self, path):
        self.f = open(path, "rb")
        hdr = self.f.read(0x971A)
        self.magic = hdr[:4]
        self.header_size = struct.unpack(">I", hdr[0x340:0x344])[0]
        vd = hdr[0x379:0x379 + 0x24]
        self.flags = vd[2]
        self.ft_count = struct.unpack("<H", vd[3:5])[0]
        self.ft_block = vd[5] | (vd[6] << 8) | (vd[7] << 16)
        self.alloc = struct.unpack(">I", vd[0x1C:0x20])[0]
        self.bpht = 1 if (self.flags & 1) else 2
        self.step0 = PER_LEVEL[0] + self.bpht
        self.step1 = PER_LEVEL[1] + (PER_LEVEL[0] + 1) * self.bpht
        self.base = (self.header_size + BLOCK - 1) & ~(BLOCK - 1)

    def block_off(self, idx):
        base = PER_LEVEL[0]
        blk = idx
        for _ in range(3):
            blk += ((idx + base) // base) * self.bpht
            if idx < base:
                break
            base *= PER_LEVEL[0]
        return self.base + (blk << 12)

    def hash_block_num(self, idx):
        if idx < PER_LEVEL[0]:
            return 0
        b = (idx // PER_LEVEL[0]) * self.step0
        b += ((idx // PER_LEVEL[1]) + 1) * self.bpht
        if idx < PER_LEVEL[1]:
            return b
        return b + self.bpht

    def next_block(self, idx):
        off = self.base + (self.hash_block_num(idx) << 12) + (idx % PER_LEVEL[0]) * 0x18
        self.f.seek(off + 0x15)
        e = self.f.read(3)
        return (e[0] << 16) | (e[1] << 8) | e[2]

    def read_block(self, idx):
        self.f.seek(self.block_off(idx))
        return self.f.read(BLOCK)

    def entries(self):
        out = []
        blk = self.ft_block
        for _ in range(self.ft_count):
            data = self.read_block(blk)
            for i in range(0, BLOCK, 0x40):
                e = data[i:i + 0x40]
                fl = e[0x28]
                nlen = fl & 0x3F
                if nlen == 0:
                    continue
                name = e[:nlen].decode("ascii", "replace")
                start = e[0x2F] | (e[0x30] << 8) | (e[0x31] << 16)
                nblocks = e[0x29] | (e[0x2A] << 8) | (e[0x2B] << 16)
                parent = struct.unpack(">h", e[0x32:0x34])[0]
                size = struct.unpack(">I", e[0x34:0x38])[0]
                out.append(dict(name=name, dir=bool(fl & 0x80), contig=bool(fl & 0x40),
                                start=start, nblocks=nblocks, parent=parent, size=size))
            blk = self.next_block(blk)
        return out

    def read_file(self, ent):
        chunks = []
        remaining = ent["size"]
        blk = ent["start"]
        n = 0
        while remaining > 0:
            if ent["contig"]:
                b = ent["start"] + n
            else:
                b = blk
            data = self.read_block(b)
            chunks.append(data[:min(BLOCK, remaining)])
            remaining -= BLOCK
            n += 1
            if not ent["contig"] and remaining > 0:
                blk = self.next_block(blk)
        return b"".join(chunks)


def main():
    pkg, out = sys.argv[1], sys.argv[2]
    s = Stfs(pkg)
    ents = s.entries()

    def path_of(i):
        parts = []
        while i != -1 and i != 0xFFFF:
            parts.append(ents[i]["name"])
            i = ents[i]["parent"]
        return os.path.join(*reversed(parts))

    total = 0
    for i, e in enumerate(ents):
        p = os.path.join(out, path_of(i))
        if e["dir"]:
            os.makedirs(p, exist_ok=True)
            continue
        os.makedirs(os.path.dirname(p), exist_ok=True)
        with open(p, "wb") as f:
            f.write(s.read_file(e))
        total += e["size"]
    print(f"{s.magic.decode()} package: {len(ents)} entries, {total} bytes extracted")


if __name__ == "__main__":
    main()
