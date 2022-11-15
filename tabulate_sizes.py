#!/usr/bin/env python3

import csv
import glob
from typing import Dict


def main(mode: str):
    size_files = glob.glob(f'./data/cluster-files/20k/{mode}/size_*')

    frequency_data: Dict[int, int] = {}
    failed_sizes = set()

    for index, size_file_path in enumerate(size_files):
        # read in the content of the
        with open(size_file_path, 'r') as size_file:
            lines = size_file.readlines()
            size_kb = int(lines[0])
            did_build = lines[1].strip() == 'true'
            if size_kb in frequency_data:
                frequency_data[size_kb] += 1
            else:
                frequency_data[size_kb] = 1

            if not did_build:
                failed_sizes.add(size_kb)

    # at this point, we have a freqency map of all of the sizes. We can compute a few different sizes
    cumulative_size_kb = 0            # 1. Cumulative size: all the DFAs
    build_sizes_kb = 0                # 2. sizes of only the DFAs that build successfully
    failed_count = len(failed_sizes)  # 3. number of DFAs that failed with a 6 GiB ceiling
    for size, frequency in frequency_data.items():
        size_contribution = (size * frequency)
        cumulative_size_kb += size_contribution
        if size not in failed_sizes:
            build_sizes_kb += size_contribution

    with open(f'{mode}-summary-report.csv', 'w') as summary_file:
        csv_writer = csv.writer(summary_file)
        # write the header
        csv_writer.writerow(["Stat", "Size"])
        csv_writer.writerow(["Total clusters", len(size_files)])
        csv_writer.writerow(["Cumulative size (kb)", cumulative_size_kb])
        csv_writer.writerow(["Built size (kb)", build_sizes_kb])
        csv_writer.writerow(["Failed count", failed_count])

    with open(f'{mode}-size-data.csv', 'w') as size_file:
        csv_writer = csv.writer(size_file)
        csv_writer.writerow(['Size (kb)', 'Frequency'])
        for size, freq in frequency_data.items():
            csv_writer.writerow([size, freq])

    # Done


if __name__ == '__main__':
    mode = 'semantic'
    main(mode)
