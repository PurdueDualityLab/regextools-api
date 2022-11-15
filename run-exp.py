#!/usr/bin/env python3

import glob
import subprocess
import logging

def main(binary_path: str, mode: str):
    # Get all of the input files
    all_input_files = glob.glob(f'data/cluster-files/20k/{mode}/input*')
    for idx, input_file in enumerate(all_input_files):
        actual_file_name = input_file.split('/')[-1]
        output_file = f'data/cluster-files/20k/{mode}/size_' + actual_file_name
        # print(f'Running with input {input_file} and output {output_file}')
        result = subprocess.run([binary_path, input_file, output_file])
        logging.info(f'Finished with cluster {idx + 1}')


if __name__ == '__main__':
    binary = './bazel-bin/performance/memory/full_dfa'
    mode = 'semantic'
    main(binary, mode)
