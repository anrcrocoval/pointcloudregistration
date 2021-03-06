import sys          # argv, exit()
import random       # seed(), shuffle()

def print_usage_and_exit():
    print('SYNOPSIS')
    print()
    print('{} [-h | --help]'.format(sys.argv[0]))
    print('    print this help message and exit')
    print()
    print('{} ifile ofile'.format(sys.argv[0]))
    print('    1) read .pcd file <ifile>')
    print('    2) randomly permuts the order in which the points are listed, without actually changing the points')
    print('    3) write resulting point cloud to file ofile in pcd format')
    #print('    Parameters xmin, ymin and zmin default to 0')
    print()
    sys.exit()

def get_args():
    if (len(sys.argv) > 1 and sys.argv[1] in ['-h', '--help']):
        print_usage_and_exit()
    elif len(sys.argv) > 2:
        infilename = sys.argv[1]
        outfilename = sys.argv[2]
    return (infilename, outfilename)

def read_file(infilename):
    header = []
    pointcloud = []
    with open(infilename, 'r') as f:
        for line in f:
            row = line.split()
            if row[0] in ['VERSION', 'FIELDS', 'SIZE', 'TYPE', 'COUNT', 'WIDTH', 'HEIGHT', 'VIEWPOINT', 'POINTS', 'DATA']:
                header.append(line)
            else:
                (sx,sy,sz,srgba) = row
                (x,y,z,rgba) = (float(sx), float(sy), float(sz), int(srgba))
                pointcloud.append((x,y,z,rgba))
    return header, pointcloud

def write_file(header, pointcloud, outfilename):
    with open(outfilename, 'w') as f:
        for line in header:
            f.write(line)
        for (x,y,z,rgba) in pointcloud:
            f.write("{} {} {} {}\n".format(x, y, z, rgba))

def main():
    random.seed()
    (infilename, outfilename) = get_args()
    header, pointcloud = read_file(infilename)
    random.shuffle(pointcloud)
    write_file(header, pointcloud, outfilename)

if __name__ == '__main__':
    main()
