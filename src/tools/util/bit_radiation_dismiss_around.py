def digit(n, r):
    n = str(n)
    l = len(n)
    for i in range(r - l):
        n = '0' + n
    return n

for coord in range(64):
    place = 1 << coord
    res = 0
    tmp = place
    while (tmp & 0x7F7F7F7F7F7F7F7F):
        tmp = (tmp & 0x7F7F7F7F7F7F7F7F) << 1
        res |= tmp
    tmp = place
    while (tmp & 0xFEFEFEFEFEFEFEFE):
        tmp = (tmp & 0xFEFEFEFEFEFEFEFE) >> 1
        res |= tmp
    tmp = place
    while (tmp & 0x00FFFFFFFFFFFFFF):
        tmp = (tmp & 0x00FFFFFFFFFFFFFF) << 8
        res |= tmp
    tmp = place
    while (tmp & 0xFFFFFFFFFFFFFF00):
        tmp = (tmp & 0xFFFFFFFFFFFFFF00) >> 8
        res |= tmp
    tmp = place
    while (tmp & 0x00FEFEFEFEFEFEFE):
        tmp = (tmp & 0x00FEFEFEFEFEFEFE) << 7
        res |= tmp
    tmp = place
    while (tmp & 0x7F7F7F7F7F7F7F00):
        tmp = (tmp & 0x7F7F7F7F7F7F7F00) >> 7
        res |= tmp
    tmp = place
    while (tmp & 0x007F7F7F7F7F7F7F):
        tmp = (tmp & 0x007F7F7F7F7F7F7F) << 9
        res |= tmp
    tmp = place
    while (tmp & 0xFEFEFEFEFEFEFE00):
        tmp = (tmp & 0xFEFEFEFEFEFEFE00) >> 9
        res |= tmp
    res2 = (place & 0x7F7F7F7F7F7F7F7F) << 1
    res2 |= (place & 0xFEFEFEFEFEFEFEFE) >> 1
    res2 |= (place & 0x00FFFFFFFFFFFFFF) << 8
    res2 |= (place & 0xFFFFFFFFFFFFFF00) >> 8
    res2 |= (place & 0x00FEFEFEFEFEFEFE) << 7
    res2 |= (place & 0x7F7F7F7F7F7F7F00) >> 7
    res2 |= (place & 0x007F7F7F7F7F7F7F) << 9
    res2 |= (place & 0xFEFEFEFEFEFEFE00) >> 9
    res &= ~res2
    res_hex = hex(res)[2:]
    res_hex_filled = digit(res_hex, 16).upper()
    res_str = '0x' + res_hex_filled + 'ULL'
    print(res_str + ', ', end='')
    if coord % 8 == 7:
        print('')