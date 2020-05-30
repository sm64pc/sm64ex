"""
This module generates a fragment of C code, in the style of that found in 
the ``model.inc.c`` files, that encodes the geometry (vertices, normals and
texture coordinates) of the model specified by the Wavefront OBJ file.

Example:
    Specify the path to the ``.obj`` file and pipe the output of the script 
    into the desired destination ``.c`` file.

        $ python obj2c.py left_hand_closed.obj > left_hand_closed.inc.c

This is a work in progress and it currently has some serious limitations:
    * The generated fragment of C code has to be manually pasted into the 
      desired source file. Make sure that the name of the Gfx structure
      you're pasting matches the one you're replacing.
    * It assumes that the target texture is a 32x32 map
    * It hasn't been properly tested.

"""

def parse(filename):
    from os.path import basename, splitext
    from datetime import datetime
    from re import sub

    clean = lambda fn: sub('\W|^(?=\d)','_', fn)
    
    # WARNIGN!
    # `gfx_name` is just a guess. You have to manually check that the name 
    # of the Gfx structure you're pasting matches the one you're replacing.
    gfx_name = clean(splitext(basename(filename))[0])
    gfx_vertices = []
    gfx_normals = []
    gfx_texture = []
    vertex_to_normal = {}
    vertex_to_texture = {}
    gfx_v_count = 0

    vtx_name = ''
    vtx_faces = []
    vtx_v_count = 0

    output_upper = []
    output_upper.append("// Armando Arredondo's SM64 Wavefront OBJ Geometry Converter")
    output_upper.append(f'// File Created: {datetime.now()}\n')
    output_lower = [f'const Gfx {gfx_name}[] = {{'] 
    reading_vtx = False

    def _record_vtx():
        nonlocal gfx_vertices
        nonlocal gfx_normals
        nonlocal gfx_texture
        nonlocal vertex_to_normal
        nonlocal vertex_to_texture
        nonlocal gfx_v_count
        nonlocal vtx_name
        nonlocal vtx_faces
        nonlocal vtx_v_count
        nonlocal output_upper
        nonlocal output_lower
        nonlocal reading_vtx

        output_upper.append(f'static const Vtx {vtx_name}[] = {{')
        for i in range(gfx_v_count - vtx_v_count, gfx_v_count):
            v_string = '[{}, {}, {}]'.format(*gfx_vertices[i])
            n_string = '[{}, {}, {}, 0x00]'.format(*gfx_normals[vertex_to_normal[i]])
            if i in vertex_to_texture:
                t_string = '[{}, {}]'.format(*gfx_texture[vertex_to_texture[i]])
            else:
                t_string = '[0, 0]'

            combined = f'    [[{v_string}, 0, {t_string}, {n_string}]],'
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
        reading_vtx = False

    with open(filename, 'r') as obj:
        for line in obj:
            line = line.strip()

            if line.startswith('v '):
                if reading_vtx:
                    _record_vtx()

                coordinates = [eval(x) for x in line.split()[1:4]]
                gfx_vertices.append(coordinates)
                vtx_v_count += 1
                gfx_v_count += 1
                continue
    
            if line.startswith('vn '):
                if reading_vtx:
                    _record_vtx()

                coordinates = [eval(x) for x in line.split()[1:4]]
                gfx_normals.append([_encode_normal(x) for x in coordinates])
                continue

            if line.startswith('vt '):
                if reading_vtx:
                    _record_vtx()

                coordinates = line.split()
                u = eval(coordinates[1])
                v = 1 - eval(coordinates[2])
                gfx_texture.append([_encode_texture(u), _encode_texture(v)])
                continue

            if line.startswith('g '):
                vtx_name = line.split()[1]
                reading_vtx = True
                continue

            if line.startswith('f '):
                _assert(reading_vtx)
                sets = [pair.split('/') for pair in line.split()[1:4]]
                vtx_faces.append([int(s[0]) for s in sets])
                for (x, y, z) in sets:
                    vertex_to_normal[int(x) - 1] = int(z) - 1
                    try:
                        vertex_to_texture[int(x) - 1] = int(y) - 1
                    except:
                        pass
                continue

    _assert(reading_vtx)
    _record_vtx()
    output_lower.append('    gsSPEndDisplayList(),')
    output_lower.append('};')

    for line in output_upper + output_lower:
        print(line)

def _encode_normal(x):
    x *= 127
    if x <= 0: x += 255
    return hex(int(x))

def _encode_texture(x):
    from math import floor
    return floor(x * 32) << 5

def _assert(p):
    if not p:
        raise RuntimeError("Unrecognized format in input file")

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('filename', help = 'filename of the .obj file to parse')
    args = parser.parse_args()
    parse(args.filename)