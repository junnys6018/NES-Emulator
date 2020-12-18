tilemap = {'0': 0, 'B': 1, '#': 1 << 4, 'C': 2 << 4, 'F': 3 << 4}

levels = open('levels.txt').read().splitlines()

# remove empty lines
levels = [line for line in levels if line.strip()]

# concatinate blocks of 15 lines into one string
levels = [''.join(levels[i: i + 15]) for i in range(0, len(levels), 15)]

# serialize and write to file
output = bytearray()
for level in levels:
    num_buttons = 0
    flag_pos = 0
    for index, tile in enumerate(level):
        output.append(tilemap[tile])

        if (tile == 'B'):
            num_buttons += 1
        elif (tile == 'F'):
            flag_pos = index

    footer = bytearray([0] * 16)
    footer[0] = flag_pos
    footer[1] = num_buttons

    output = output + footer

open('levels.bin', 'wb').write(output)
