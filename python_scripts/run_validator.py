import glob
from nwbinspector import inspect_nwbfile

nwbfile_paths = glob.glob("/Users/smprince/Documents/code/aqnwb/build/dev/tests/data/*.nwb")

results = []
for nwbfile_path in nwbfile_paths:
    result = list(inspect_nwbfile(nwbfile_path=nwbfile_path))
    # results.append(result)
    print(result)

print('done')


