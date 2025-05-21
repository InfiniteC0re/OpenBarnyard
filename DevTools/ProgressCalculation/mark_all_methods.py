import csv
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
    cpp_files.extend(find_cpp_files('D:\\dev\\OpenBarnyard\\Toshi\\Source'))
    cpp_files.extend(find_cpp_files('D:\\dev\\OpenBarnyard\\OpenBarnyard\\Source'))

    try:
        os.remove('toshi-marker-out.txt')
    except Exception:
        pass

    try:
        os.remove('toshi-marker-unique.csv')
    except Exception:
        pass

    pool_size = 16

    # define worker function before a Pool is instantiated
    def worker(file):
        try:
            if 'Profiler' not in file:
                subprocess.DEVNULL
                subprocess.call([
                    r'toshi-marker.exe',
                    file,
                    '--',
                    r'-ID:\\dev\\OpenBarnyard\\Toshi\\Source',
                    r'-ID:\\dev\\OpenBarnyard\\OpenBarnyard\\Source',
                    r'-ID:\\dev\\OpenBarnyard\\Toshi\\Vendor\\bink\\include',
                    r'-ID:\\dev\\OpenBarnyard\\Toshi\\Vendor\\curl\\include',
                    r'-ID:\\dev\\OpenBarnyard\\Toshi\\Vendor\\DX81\\include',
                    r'-ID:\\dev\\OpenBarnyard\\Toshi\\Vendor\\fmod\\include',
                    r'-ID:\\dev\\OpenBarnyard\\Toshi\\Vendor\\glew\\include',
                    r'-ID:\\dev\\OpenBarnyard\\Toshi\\Vendor\\glm\\include',
                    r'-ID:\\dev\\OpenBarnyard\\Toshi\\Vendor\\Opcode\\Source',
                    r'-ID:\\dev\\OpenBarnyard\\Toshi\\Vendor\\sdl2\\include',
                    r'-ID:\\dev\\OpenBarnyard\\Toshi\\Vendor\\stb',
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
                ], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

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
    
    # Do the actual marking
    source_code_files = {}
    source_code_opened_files = {}

    with open('toshi-marker-unique.csv', 'r') as file:
        csv_reader = csv.reader(file)
        for row in csv_reader:
            name = row[0]
            arguments = row[1]
            file_name = row[2]
            line_num = row[3]
            
            if file_name not in source_code_files:
                source_code_files[file_name] = {}
            
            func_name_dict = f'{name}'
            
            if func_name_dict not in source_code_files[file_name]:
                source_code_files[file_name][func_name_dict] = []
            
            source_code_files[file_name][func_name_dict].append({
                'name': name,
                'arguments': arguments,
                'file_name': file_name,
                'line_num': int(line_num)
            })
            

    with open('functions_with_signatures.csv', 'r') as file:
        csv_reader = csv.reader(file)
        for row in csv_reader:
            name = row[0]
            address = row[1]
            type = row[2]
            num_args = row[3]
            func_name = row[4]
            ret_type = row[5]
            signature = row[6]
            
            if type != "Function":
                continue
            
            for source_file_name in source_code_files:            
                if func_name in source_code_files[source_file_name]:
                    # This is a match
                    
                    # Read file data and cache it's data if it is not read yet
                    if source_file_name not in source_code_opened_files:
                        with open(source_file_name, 'r') as source_code_file:
                            lines = []
                            for line in source_code_file:
                                lines.append(line)
                                
                            source_code_opened_files[source_file_name] = lines
                    
                    # Get source code of the file        
                    source_code_lines = source_code_opened_files[source_file_name]
                    found_methods = source_code_files[source_file_name][func_name]
                    assumed_method = None
                    
                    if len(found_methods) == 1:
                        # This is the only method we got from the source code, so let's assume it's the one
                        assumed_method = found_methods[0]
                    else:
                        # TODO: Support overloaded methods
                        for method in found_methods:
                            pass
                    
                    if assumed_method != None:
                        decl_line = assumed_method['line_num'] - 1
                        has_marker = False
                        marker_line = -1
                        
                        # Make sure we don't already have a marker there
                        multiline_comment = False
                        for i in range(decl_line - 1, 0, -1):
                            line = str(source_code_lines[i]).strip()
                            
                            if len(line) == 0:
                                continue
                            
                            if line.startswith('/*'):
                                multiline_comment = False
                                continue
                            
                            if line.endswith('*/') and (len(line) > 3 or not line.startswith('/*')):
                                multiline_comment = True
                                continue
                            
                            if multiline_comment:
                                continue
                            
                            if line.startswith('//'):
                                comment = line[2:].strip()
                                
                                if comment.startswith('$Barnyard: FUNCTION'):
                                    has_marker = True
                                    marker_line = i + 1
                                
                                continue
                            else:
                                marker_line = i + 1
                                break
                        
                        # Skip empty lines
                        for i in range(marker_line, len(source_code_lines)):
                            line = str(source_code_lines[i]).strip()
                            
                            if len(line) == 0:
                                continue
                            
                            marker_line = i
                            break
                        
                        if not has_marker:
                            # count tabs/spaces
                            definition = source_code_lines[decl_line]
                            num_spaces = 0
                            num_tabs = 0
                            
                            for i in range(0, len(definition)):
                                if definition[i] == ' ':
                                    num_spaces += 1
                                elif definition[i] == '\t':
                                    num_tabs += 1
                                else:
                                    break
                            
                            num_tabs += num_spaces // 4
                            num_spaces -= (num_spaces // 4) * 4
                            source_code_lines.insert(marker_line, f'{num_tabs * '\t'}{num_spaces * ' '}// $Barnyard: FUNCTION {address}\n')
                            
                            # Update numbers of lines
                            for method_name in source_code_files[source_file_name]:
                                for method in source_code_files[source_file_name][method_name]:
                                    if method['line_num'] >= decl_line + 1:
                                        method['line_num'] = int(method['line_num'] + 1)
                            
                            print(f'{marker_line + 1} {decl_line + 1} - {source_code_lines[marker_line].strip()} - ' + source_code_lines[decl_line + 1].strip())
                            
                        
                    break

            
    # Flush changes
    for source_file_name in source_code_opened_files:
        with open(source_file_name, 'w') as source_code_file:
            source_code_file.writelines(source_code_opened_files[source_file_name])
        
    
if __name__ == "__main__":
    main()

