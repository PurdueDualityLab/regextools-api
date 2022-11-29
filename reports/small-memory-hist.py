
import matplotlib.pyplot as plt
import numpy as np
import pandas
import math
import argparse


def make_bins(width: int, max: float) -> list[int]:
    return [idx * width for idx in range(0, math.ceil(max / width))]


def main(semantic_data_path: str, random_data_path: str, cutoff_size: int):
    semantic_size_data = pandas.read_csv(semantic_data_path)
    random_size_data = pandas.read_csv(random_data_path)
    semantic_sizes_mb = semantic_size_data.filter(['Size (kb)']).applymap(lambda val: val / 1000).to_numpy().flatten()
    semantic_sizes_small = semantic_sizes_mb[semantic_sizes_mb <= cutoff_size]
    random_sizes_mb = random_size_data.filter(['Size (kb)']).applymap(lambda val: val / 1000).to_numpy().flatten()
    random_sizes_small = random_sizes_mb[random_sizes_mb <= cutoff_size]
    print(semantic_sizes_mb)
    print(random_sizes_mb)

    # Make a bin iterable for 10mb chunks
    bin_ranges = make_bins(10, max(random_sizes_small))
    print(f"Bins: {bin_ranges}")

    # Make labels with info
    labels = [
        f'Semantic (n={len(semantic_sizes_small)}, total_size={sum(semantic_sizes_small):.2f}mb)',
        f'Random   (n={len(random_sizes_small)}, total_size={sum(random_sizes_small):.2f}mb)'
    ]

    plt.hist([semantic_sizes_small, random_sizes_small], histtype='bar', bins=bin_ranges, stacked=True, color=['blue', 'orange'], label=labels)
    plt.xlabel("Size (mb)")
    plt.ylabel("Frequency")
    plt.title(f'Comparison of DFA Sizes (mb) <= {cutoff_size}mb, Random vs. Semantic')
    plt.legend()
    plt.show()



if __name__ == '__main__':
    arg_parser = argparse.ArgumentParser('full-memory-hist', description='Create histograms based on memory usage')
    arg_parser.add_argument('semantic_path', help="File to the semantic-size-data.csv file you want to use")
    arg_parser.add_argument('random_path', help="File to the random-size-data.csv file you want to use")
    arg_parser.add_argument('--cutoff', help="The max size of DFA to take in mb", default=300, type=int)
    args = arg_parser.parse_args()

    # semantic = '/home/charlie/Programming/regextools/semantic-size-data.csv'
    # random = '/home/charlie/Programming/regextools/random-size-data.csv'
    # cutoff_size = 300
    main(args.semantic_path, args.random_path, args.cutoff)
