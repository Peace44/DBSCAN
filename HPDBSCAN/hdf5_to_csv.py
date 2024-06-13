
import h5py
import pandas as pd
import sys



if len(sys.argv) != 3:
    print("Usage: python hdf5_to_csv.py <input_hdf5_file> <output_csv_file>")
else:
    hdf5_file = sys.argv[1]
    csv_file = sys.argv[2]

with h5py.File(hdf5_file, 'r') as hdf:            
    data = hdf['DATA'][:]
    data_df = pd.DataFrame(data, columns=['x','y','z'])

    clusters = hdf['CLUSTERS'][:]
    clusters_df = pd.DataFrame(clusters, columns=['cluster'])

    clusters_df['cluster'] = clusters_df['cluster'].abs()
    clusters_df['cluster'] = clusters_df['cluster'].replace(0, -2) #### NOISE PTS MAPPED FROM 0 TO -2

    combined_df = pd.concat([data_df, clusters_df], axis=1)
    combined_df.to_csv(csv_file, index=False)
    
    # print(f"Successfully converted {hdf5_file} to {csv_file}")