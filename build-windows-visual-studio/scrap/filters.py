import uuid

item_format = '''    <ClCompile Include="{FILENAME}">
      <Filter>{TOP_FILTER}\\{PATH}</Filter>
    </ClCompile>'''

def gen_item_group(top_filter, filename):
	with open(filename, 'r') as f:
		sources = f.readlines()

	print('  <ItemGroup>')
	current_filter = ''
	for s in sources:
		s = s.replace('\n', '').replace('\r', '').replace('/','\\')
		path = s.rpartition('\\')[0].replace('..\\', '')
		print(item_format
				.replace('{FILENAME}', s)
				.replace('{TOP_FILTER}', top_filter)
				.replace('{PATH}', path))
	print('  </ItemGroup>')

gen_item_group('Source Files', 'source.txt')
gen_item_group('Header Files', 'headers.txt')
