
def gen_item_group(filename):
	with open(filename, 'r') as f:
		sources = f.readlines()

	print('  <ItemGroup>')
	current_filter = ''
	for s in sources:
		s = s.replace('\n', '').replace('\r', '')
		print('    <ClCompile Include="' + s.replace('/', '\\') + '" />')
	print('  </ItemGroup>')

gen_item_group('source.txt')
gen_item_group('headers.txt')
