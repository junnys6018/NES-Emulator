foreground = {'0': 0, '#': 1, 'B': 2}
background = {'0': 0, 'B': 1, 'F': 2, 'I': 3}

content = open('levels.txt').read().splitlines()

# remove empty lines
content = [line for line in content if line.strip()]

# concatinate blocks of 15 lins into one string
content = [''.join(content[i: i + 15]) for i in range(0, len(content), 15)]

# even elements define the foreground of a level
fg_elems = content[::2]

# odd elements define the background of a level
bg_elems = content[1::2]

# serialize and write to file
output = bytearray()
for (fg_elem, bg_elem) in zip(fg_elems, bg_elems):
	for (fg, bg) in zip(fg_elem, bg_elem):
		output.append(foreground[fg] << 4 | background[bg])
	# pad 16 bytes
	for _ in range(16):
		output.append(0)

open('levels.bin', 'wb').write(output)