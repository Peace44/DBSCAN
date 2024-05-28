import h5py

file_path = "../hpdbscan/build/data.h5"

with h5py.File(file_path, 'r') as file:
    data = file["DATA"][:]
    clusters = file["CLUSTERS"][:]

    print("DATA:", data)
    print("CLUSTERS:", clusters)


