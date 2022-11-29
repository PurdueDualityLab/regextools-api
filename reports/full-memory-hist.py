
import matplotlib.pyplot as plt
import numpy as np
import pandas
import math
import argparse


def make_bins(width: int, max) -> list[int]:
    return [idx * width for idx in range(0, math.ceil(max / width))]


def main(semantic_data_path: str, random_data_path: str):
    semantic_size_data = pandas.read_csv(semantic_data_path)
    random_size_data = pandas.read_csv(random_data_path)
    semantic_sizes_mb = semantic_size_data.filter(['Size (kb)']).applymap(lambda val: val / 1000).to_numpy().flatten()
    random_sizes_mb = random_size_data.filter(['Size (kb)']).applymap(lambda val: val / 1000).to_numpy().flatten()
    print(semantic_sizes_mb)
    print(random_sizes_mb)

    # Make a bin iterable for 10mb chunks
    bin_ranges = make_bins(250, max(random_sizes_mb))

    plt.hist([semantic_sizes_mb, random_sizes_mb], histtype='bar', bins=bin_ranges, stacked=True, color=['blue', 'orange'], label=['semantic', 'random'])
    plt.xlabel("Size (mb)")
    plt.ylabel("Frequency")
    plt.title('Comparison of DFA Sizes (mb), Random vs. Semantic')
    plt.legend()
    plt.show()



if __name__ == '__main__':
    arg_parser = argparse.ArgumentParser('full-memory-hist', description='Create histograms based on memory usage')
    arg_parser.add_argument('semantic_path', help="File to the semantic-size-data.csv file you want to use")
    arg_parser.add_argument('random_path', help="File to the random-size-data.csv file you want to use")
    # semantic = '/home/charlie/Programming/regextools/semantic-size-data.csv'
    # random = '/home/charlie/Programming/regextools/random-size-data.csv'
    args = arg_parser.parse_args()
    main(args.semantic_path, args.random_path)
