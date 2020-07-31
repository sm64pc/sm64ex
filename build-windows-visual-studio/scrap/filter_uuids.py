import uuid

filter_format = '{TOP_FILTER}\\{PATH}'
item_format = '''    <Filter Include="{FILTER}">
      <UniqueIdentifier>{{UUID}}</UniqueIdentifier>
    </Filter>'''
created_filters = {}

def gen_item_group(top_filter, filename):
	with open(filename, 'r') as f:
		sources = f.readlines()

	for s in sources:
		s = s.replace('\n', '').replace('\r', '').replace('/','\\')
		path = s.rpartition('\\')[0].replace('..\\', '')
		filter = filter_format.replace('{TOP_FILTER}', top_filter).replace('{PATH}', path)

		full = top_filter
		for piece in filter.split('\\')[1:]:
			full += '\\' + piece

			if full not in created_filters:
				print(item_format
						.replace('{FILTER}', full)
						.replace('{UUID}', str(uuid.uuid4())))
				created_filters[full] = True

gen_item_group('Source Files', 'source.txt')
gen_item_group('Header Files', 'headers.txt')
