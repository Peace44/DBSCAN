import pandas as pd
import h5py

input_file = "rand_pts"
csv_file = input_file + '.csv'
hdf5_file = input_file + '.h5'

df = pd.read_csv(csv_file)
data_matrix = df.to_numpy()

with h5py.File(hdf5_file, 'w') as f:
    f.create_dataset('DATA', data=data_matrix, compression='gzip')