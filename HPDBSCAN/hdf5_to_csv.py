
import h5py
import pandas as pd
import sys

def hdf5_to_csv(hdf5_file, csv_file, key):
    try:
        with h5py.File(hdf5_file, 'r') as hdf:
            print("Keys: %s" % hdf.keys())
            
            data = hdf[key][()]
            data_df = pd.DataFrame(data)
            data_df.to_csv(csv_file, index=False)
            print(f"Successfully converted {hdf5_file} to {csv_file}")
    except KeyError as e:
        print(f"Key error: {e}")
    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python hdf5_to_csv.py <input_hdf5_file> <output_csv_file> <key>")
    else:
        hdf5_file = sys.argv[1]
        csv_file = sys.argv[2]
        key = sys.argv[3]
        hdf5_to_csv(hdf5_file, csv_file, key)
