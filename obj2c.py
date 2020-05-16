"""
This module generates a fragment of C code, in the style of that found in 
the ``model.inc.c`` files, that encodes the geometry of the model specified
by the Wavefront OBJ file.

Example:
    Specify the path to the ``.obj`` file and pipe the output of the script 
    into the desired destination ``.c`` file.

        $ python obj2c.py left_hand_closed.obj > left_hand_closed.inc.c

This is a work in progress and it currently has some serious limitations:
    * It only encodes the geometry information of the OBJ file, so no 
      texture mapping or any other info.
    * The generated fragment of C code has to be manually pasted into the 
      desired source file. Make sure that the name of the Gfx structure
      you're pasting matches the one you're replacing.
    * It hasn't been properly tested.

"""

def parse(filename):
    from os.path import basename, splitext
    from re import sub

    # WARNIGN!
    # `gfx_name` is just a guess. You have to manually check that the name 
    # of the Gfx structure you're pasting matches the one you're replacing.
    clean = lambda fn: sub('\W|^(?=\d)','_', fn)
    gfx_name = clean(splitext(basename(filename))[0])    
    gfx_vertices = []
    gfx_normals = []
    vertex_to_normal = {}
    gfx_v_count = 0

    vtx_name = ''
    vtx_faces = []
    vtx_v_count = 0

    output_upper = []
    output_lower = [f'const Gfx {gfx_name}[] = {{'] 

    with open(filename, 'r') as obj:
        for line in obj:
            line = line.strip()

            if line.startswith('v '):
                coordinates = [eval(x) for x in line.split()[1:4]]
                gfx_vertices.append(coordinates)
                vtx_v_count += 1
                gfx_v_count += 1
    
            if line.startswith('vn '):
                coordinates = [eval(x) for x in line.split()[1:4]]
                gfx_normals.append([_encode_normal(x) for x in coordinates])

            if line.startswith('g '):
                vtx_name = line.split()[1]

            if line.startswith('f '):
                pairs = [pair.split('//') for pair in line.split()[1:4]]
                vtx_faces.append([int(pair[0]) for pair in pairs])
                for (x, y) in pairs:
                    vertex_to_normal[int(x) - 1] = int(y) - 1

            if line.startswith('# ') and line.endswith('faces'):
                output_upper.append(f'static const Vtx {vtx_name}[] = {{')
                for i in range(gfx_v_count - vtx_v_count, gfx_v_count):
                    v_string = '[{}, {}, {}]'.format(*gfx_vertices[i])
                    n_string = '[{}, {}, {}, 0x00]'.format(*gfx_normals[vertex_to_normal[i]])
                    combined = f'    [[{v_string}, 0, [0, 0], {n_string}]],'
                    output_upper.append(combined.replace('[', '{').replace(']', '}'))

                output_upper.append('};\n')
                output_lower.append(f'    gsSPVertex({vtx_name}, {vtx_v_count}, 0),')

                n = len(vtx_faces)
                correction = vtx_v_count - gfx_v_count - 1 
                for i in range(int(n / 2)):
                    f1 = [vtx_faces[2 * i][j] + correction for j in range(3)]
                    f2 = [vtx_faces[2 * i + 1][j] + correction for j in range(3)]
                    output_lower.append('    gsSP2Triangles({}, {}, {}, 0x0, {}, {}, {}, 0x0),'.format(*f1, *f2))
                
                if n % 2 != 0:
                    f3 = [vtx_faces[-1][j] + correction for j in range(3)]
                    output_lower.append('    gsSP1Triangle({}, {}, {}, 0x0),'.format(*f3))
                                
                vtx_v_count = 0
                vtx_faces = []

    output_lower.append('    gsSPEndDisplayList(),')
    output_lower.append('};')

    for line in output_upper + output_lower:
        print(line)

def _encode_normal(x):
    x *= 127
    if x <= 0: x += 255
    return hex(int(x))

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('filename', help = 'filename of the .obj file to parse')
    args = parser.parse_args()
    parse(args.filename)