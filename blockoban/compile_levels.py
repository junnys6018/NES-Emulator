foreground = {'0': 0, '#': 1, 'B': 2, 'F': 3}
background = {'0': 0, 'B': 1}

content = open('levels.txt').read().splitlines()

# remove empty lines
content = [line for line in content if line.strip()]

# concatinate blocks of 15 lines into one string
content = [''.join(content[i: i + 15]) for i in range(0, len(content), 15)]

# even elements define the foreground of a level
fg_elems = content[::2]

# odd elements define the background of a level
bg_elems = content[1::2]

# serialize and write to file
output = bytearray()
for (fg_elem, bg_elem) in zip(fg_elems, bg_elems):
	num_buttons = 0
	flag_pos = 0
	flag_found = False
	for (fg, bg) in zip(fg_elem, bg_elem):
		output.append(foreground[fg] << 4 | background[bg])

		if (bg == 'B'):
			num_buttons += 1
		if (fg == 'F'):
			flag_found = True
		if (not flag_found):
			flag_pos += 1
		if (bg == 'B' and (fg != '0')):
			print("[WARN] Button is below non air tile")

	footer = bytearray([0] * 16)
	footer[0] = flag_pos
	footer[1] = num_buttons
	
	output = output + footer

open('levels.bin', 'wb').write(output)