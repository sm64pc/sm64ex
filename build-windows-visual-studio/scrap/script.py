import uuid

filter_format = '{TOP_FILTER}\\{PATH}'
item_format = '''    <Filter Include="{FILTER}">
      <UniqueIdentifier>{{UUID}}</UniqueIdentifier>
    </Filter>'''

def gen_item_group(top_filter, filename):
	with open(filename, 'r') as f:
		sources = f.readlines()

	current_filter = ''
	for s in sources:
		s = s.replace('\n', '').replace('\r', '').replace('/','\\')
		path = s.rpartition('\\')[0].replace('..\\', '')
		filter = filter_format.replace('{TOP_FILTER}', top_filter).replace('{PATH}', path)
		if filter != current_filter:
			print(item_format
					.replace('{FILTER}', filter)
					.replace('{UUID}', str(uuid.uuid4())))
			current_filter = filter

gen_item_group('Source Files', 'source.txt')
gen_item_group('Header Files', 'headers.txt')
