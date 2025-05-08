import os
import subprocess
from typing import List
from multiprocessing.pool import ThreadPool as Pool
import time


def find_cpp_files(root_dir: str) -> List[str]:
    cpp_files = []
    for root, _, files in os.walk(root_dir):
        for file in files:
            if file.endswith(('.cpp')):
                cpp_files.append(os.path.join(root, file))
    return cpp_files


def main():
    cpp_files = []
    cpp_files.extend(find_cpp_files('D:\\_dev\\OpenBarnyard\\Source\\Toshi\\Source'))
    cpp_files.extend(find_cpp_files('D:\\_dev\\OpenBarnyard\\Source\\OpenBarnyard\\Source'))

    try:
        os.remove('toshi-marker-out.txt')
    except Exception:
        pass

    try:
        os.remove('toshi-marker-unique.csv')
    except Exception:
        pass

    pool_size = 32

    # define worker function before a Pool is instantiated
    def worker(file):
        try:
            if 'Profiler' not in file:
                subprocess.call([
                    r'toshi-marker.exe',
                    file,
                    '--',
                    r'-ID:\\_dev\\OpenBarnyard\\Source\\Toshi\\Source',
                    r'-ID:\\_dev\\OpenBarnyard\\Source\\OpenBarnyard\\Source',
                    r'-ID:\\_dev\\OpenBarnyard\\Source\\Toshi\\Vendor\\bink\\include',
                    r'-ID:\\_dev\\OpenBarnyard\\Source\\Toshi\\Vendor\\curl\\include',
                    r'-ID:\\_dev\\OpenBarnyard\\Source\\Toshi\\Vendor\\DX81\\include',
                    r'-ID:\\_dev\\OpenBarnyard\\Source\\Toshi\\Vendor\\fmod\\include',
                    r'-ID:\\_dev\\OpenBarnyard\\Source\\Toshi\\Vendor\\glew\\include',
                    r'-ID:\\_dev\\OpenBarnyard\\Source\\Toshi\\Vendor\\glm\\include',
                    r'-ID:\\_dev\\OpenBarnyard\\Source\\Toshi\\Vendor\\Opcode\\Source',
                    r'-ID:\\_dev\\OpenBarnyard\\Source\\Toshi\\Vendor\\sdl2\\include',
                    r'-ID:\\_dev\\OpenBarnyard\\Source\\Toshi\\Vendor\\stb',
                    '--language=c++',
                    '-includeToshiPCH.h',
                    '-std=c++20',
                    '-DTOSHI_SKU_WINDOWS',
                    '-DTRENDERINTERFACE_DX8',
                    '-DTOSHI_USER_ENGINE',
                    '-DTOSHI_RELEASE',
                    '-m32',
                    '-ferror-limit=0',
                    '-Wno-everything'
                ])

        except:
            pass

    pool = Pool(pool_size)

    for file in cpp_files:
        pool.apply_async(worker, (file,))
        
    pool.close()
    pool.join()

    time.sleep(1)
    lines_seen = set() # holds lines already seen
    outfile = open('toshi-marker-unique.csv', "w")
    for line in open('toshi-marker-out.txt', "r"):
        line = line.replace('/', '\\')
        line = line.replace('\\\\', '\\')
        if line not in lines_seen: # not a duplicate
            lines_seen.add(line)

    outfile.writelines(sorted(lines_seen))
    outfile.close()
        
    
if __name__ == "__main__":
    main()