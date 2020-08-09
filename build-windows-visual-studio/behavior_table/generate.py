behaviors = []

with open('behavior_data.c', 'r') as f:
    lines = f.readlines()

with open('../../data/behavior_data.c', 'w+') as f:
    line_number = -1
    current_behavior = None
    for line in lines:
        f.write(line)
        if current_behavior is not None:
            f.write('    ID(id_' + current_behavior + '),\n')
            current_behavior = None

        line_number += 1
        if not line.startswith('const BehaviorScript'):
            continue
        if 'BEGIN(' not in lines[line_number + 1]:
            continue
        behavior_name = line.split('const BehaviorScript ')[1].split('[]')[0]
        current_behavior = behavior_name
        behaviors.append(behavior_name)

with open('../../include/behavior_table.h', 'w+') as f:
    f.write('#ifndef BEHAVIOR_TABLE_H\n')
    f.write('#define BEHAVIOR_TABLE_H\n\n')
    f.write('extern BehaviorScript* gBehaviorTable;\n\n')
    f.write('enum BehaviorId {\n')
    for behavior in behaviors:
        f.write('    id_' + behavior + ',\n')
    f.write('    id_bhv_max_count // must be the last in the list\n')
    f.write('};\n')

    f.write('#endif\n')

with open('../../data/behavior_table.c', 'w+') as f:
    f.write('#include "behavior_table.h"\n\n')
    f.write('BehaviorScript* gBehaviorTable[id_bhv_max_count] = {\n')
    for behavior in behaviors:
        f.write('    [id_' + behavior + '] = &' + behavior + ',\n')
    f.write('};\n')

print(str(behaviors))