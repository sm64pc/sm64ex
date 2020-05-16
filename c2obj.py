"""
This module attempts to parse the ``model.inc.c`` files and extract the
3D models within as standard Wavefront OBJ files. 

Example:
    Specify the path to the ``.inc.c`` file and a directory where to save 
    the extracted ``.obj`` files.

        $ python c2obj.py ./actors/mario/model.inc.c ./actors/mario/obj/

This is a work in progress and it currently has some serious limitations:
    * It only extracts geometry information, so no textures or any other info
    * It makes assumptions about the layout of the code in the C source
    * It hasn't been properly tested.

"""

def parse(filename, output_directory):
    from os import path, mkdir

    if not path.isdir(output_directory):
        try:
            mkdir(output_directory)
        except OSError:
            print(f'Could not use output directory {output_directory}.')
        
    vtx_def = 'static const Vtx '
    vtx_data = {}
    reading_vtx = False
    current_vtx_name = ''
    current_vtx_data = []
    current_vtx_vertices = 0

    gfx_def = 'const Gfx '
    reading_gfx = False
    current_gfx_vertices = 0
    current_gfx_faces = 0
    insert_vert_call = 'gsSPVertex('
    insert_1tri_call = 'gsSP1Triangle(' 
    insert_2tri_call = 'gsSP2Triangles('
    gfx_count = 0

    end_of_block = '};'

    with open(filename, 'r') as f:
        for line in f:
            line = line.strip()

            if line.startswith(vtx_def):
                vtx_name = line.split(' ')[3][:-2]
                current_vtx_name = vtx_name
                current_vtx_data = []
                reading_vtx = True
                continue

            if line.startswith(gfx_def):
                from datetime import datetime
                
                current_gfx_name = line.split(' ')[2][:-2]
                current_gfx_file = open(path.join(output_directory, current_gfx_name + '.obj'), 'w')
                current_gfx_file.write("# Armando Arredondo's SM64 Wavefront OBJ Geometry Converter\n")
                current_gfx_file.write('# File Created: {}\n\n'.format(datetime.now()))
                reading_gfx = True
                continue
            
            if line == end_of_block:
                if reading_vtx:
                    vtx_data[current_vtx_name] = current_vtx_data
                    reading_vtx = False

                elif reading_gfx:
                    current_gfx_file.write(f'# {current_gfx_faces} faces\n\n')
                    current_gfx_file.close()
                    current_gfx_vertices = 0
                    reading_gfx = False
                    gfx_count += 1
                
                continue
            
            if reading_vtx:
                line = line.replace('{', '[').replace('}', ']')
                tri = eval(line[:-1])[0]
                current_vtx_data.append(tri)
                continue
            
            if reading_gfx:
                if line.startswith(insert_vert_call):
                    args = line[len(insert_vert_call):].split(',')
                    current_vtx_name = args[0]
                    
                    if current_gfx_vertices > 0:
                        current_gfx_file.write(f'# {current_gfx_faces} faces\n\n')
                    
                    current_gfx_faces = 0
                    current_vtx_vertices = len(vtx_data[current_vtx_name])
                    current_gfx_vertices += current_vtx_vertices

                    current_gfx_file.write(f'#\n# object {current_vtx_name}\n#\n\n')
                    current_vtx_data = vtx_data[current_vtx_name]
                    for tri in current_vtx_data:
                        v = tri[0]
                        current_gfx_file.write('v  {:.3f} {:.3f} {:.3f}\n'.format(*v))
                    current_gfx_file.write(f'# {current_vtx_vertices} vertices\n\n')

                    for tri in current_vtx_data:
                        n = [_decode_normal(u) for u in tri[3][:3]]
                        current_gfx_file.write('vn {:.3f} {:.3f} {:.3f}\n'.format(*n))
                    current_gfx_file.write(f'# {current_vtx_vertices} vertex normals\n\n')

                    current_gfx_file.write(f'g {current_vtx_name}\n\n')
                
                elif line.startswith(insert_2tri_call):
                    args = line[len(insert_2tri_call):].split(',')
                    correction = current_gfx_vertices - current_vtx_vertices + 1
                    indexes = [eval(args[i]) + correction for i in [0, 1, 2, 4, 5, 6]]
                    current_gfx_file.write('f {0}//{0} {1}//{1} {2}//{2}\n'.format(*indexes[:3]))
                    current_gfx_file.write('f {0}//{0} {1}//{1} {2}//{2}\n'.format(*indexes[3:]))
                    current_gfx_faces += 2

                elif line.startswith(insert_1tri_call):
                    args = line[len(insert_1tri_call):].split(',')
                    correction = current_gfx_vertices - current_vtx_vertices + 1
                    indexes = [eval(args[i]) + correction for i in [0, 1, 2]]
                    current_gfx_file.write('f {0}//{0} {1}//{1} {2}//{2}\n'.format(*indexes))
                    current_gfx_faces += 1

                continue
    
    print(f'{gfx_count} models extracted.')

def _decode_normal(x):
    y = x if x <= 127 else x - 255
    return y / 127

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('filename', help = 'filename of the .inc.c source file')
    parser.add_argument('output_directory', help = 'directory where to put the extracted .obj files')
    args = parser.parse_args()
    parse(args.filename, args.output_directory)