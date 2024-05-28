import pandas as pd
import h5py
import sys
import os

if len(sys.argv) != 3:
    print("Usage: python3 csv_to_hdf5.py <input_file> <output_file>")
    sys.exit(1)

csv_file = sys.argv[1]
hdf5_file = sys.argv[2]

if not csv_file.endswith('.csv'):
    print("Error: The input file must be a CSV file!")
    sys.exit(1)

if not hdf5_file.endswith('.h5'):
    print("Error: The output file must be an HDF5 file! ==> Defaultly, writing to 'data.h5'")
    hdf5_file = "data.h5"

df = pd.read_csv(csv_file)
data_matrix = df.to_numpy()

with h5py.File(hdf5_file, 'w') as f:
    f.create_dataset('DATA', data=data_matrix, compression='gzip')

